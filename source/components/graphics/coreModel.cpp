///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"
#include "model/MD3.h"
#include "model/MD5.h"
#include <forsyth_too.h>

coreModel* coreModel::s_pCurrent = NULL;


// ****************************************************************
/* constructor */
coreModel::coreModel(const coreModelLoad eLoad)noexcept
: coreResource          ()
, m_iVertexArray        (0u)
, m_aVertexBuffer       {}
, m_IndexBuffer         ()
, m_iNumVertices        (0u)
, m_iNumIndices         (0u)
, m_iNumClusters        (0u)
, m_vBoundingRange      (coreVector3(0.0f,0.0f,0.0f))
, m_fBoundingRadius     (0.0f)
, m_vNaturalCenter      (coreVector3(0.0f,0.0f,0.0f))
, m_vNaturalRange       (coreVector3(0.0f,0.0f,0.0f))
, m_vWeightedCenter     (coreVector3(0.0f,0.0f,0.0f))
, m_pvVertexPosition    (NULL)
, m_ppiClusterIndex     (NULL)
, m_piClusterNumIndices (NULL)
, m_pvClusterPosition   (NULL)
, m_pfClusterRadius     (NULL)
, m_iPrimitiveType      (GL_TRIANGLES)
, m_iIndexType          (0u)
, m_eLoad               (eLoad)
, m_Sync                ()
{
}


// ****************************************************************
/* destructor */
coreModel::~coreModel()
{
    this->Unload();
}


// ****************************************************************
/* load model resource data */
coreStatus coreModel::Load(coreFile* pFile)
{
    // check for sync object status
    const coreStatus eCheck = m_Sync.Check(0u);
    if(eCheck >= CORE_OK) return eCheck;

    coreFileScope oUnloader(pFile);

    WARN_IF(!m_aVertexBuffer.empty()) return CORE_INVALID_CALL;
    if(!pFile)                        return CORE_INVALID_INPUT;
    if(!pFile->GetData())             return CORE_ERROR_FILE;

    // extract file extension
    const coreChar* pcExtension = coreData::StrToLower(coreData::StrExtension(pFile->GetPath()));

    // import model file
    coreImport oImport;
         if(!std::memcmp(pcExtension, "md5", 3u)) coreImportMD5(pFile->GetData(), &oImport);
    else if(!std::memcmp(pcExtension, "md3", 3u)) coreImportMD3(pFile->GetData(), &oImport);
    else
    {
        Core::Log->Warning("Model (%s) could not be identified (valid extensions: md5[mesh], md3)", m_sName.c_str());
        return CORE_INVALID_DATA;
    }

    // check for success
    if(oImport.aVertexData.empty())
    {
        Core::Log->Warning("Model (%s) could not be loaded", m_sName.c_str());
        return CORE_INVALID_DATA;
    }

    // save properties
    m_iNumVertices = oImport.aVertexData.size();
    m_iNumIndices  = oImport.aiIndexData.size();
    ASSERT((m_iNumVertices > 0u) && (m_iNumVertices <= 0xFFFFu) && m_iNumIndices)

    // prepare index-map (for deferred remapping)
    alignas(ALIGNMENT_PAGE) BIG_STATIC coreUint16 s_aiMap[0x10000u];
    for(coreUintW i = 0u, ie = m_iNumVertices; i < ie; ++i) s_aiMap[i] = i & 0xFFFFu;

    // apply post-transform vertex cache optimization to index data
    coreUint16* piOptimizedData = new coreUint16[m_iNumIndices];
    Forsyth::OptimizeFaces(oImport.aiIndexData.data(), m_iNumIndices, m_iNumVertices, piOptimizedData, 16u);

    // apply pre-transform vertex cache optimization to vertex data
    coreUint16 iCurIndex = 0u;
    for(coreUintW i = 0u, ie = m_iNumIndices; i < ie; ++i)
    {
        const coreUint16 iFirst = piOptimizedData[i];
        ASSERT(iFirst < m_iNumVertices)

        // check first map entry
        if(s_aiMap[iFirst] >= iCurIndex)
        {
            const coreUint16 iNew = iCurIndex++;
            const coreUint16 iOld = s_aiMap[iFirst];

            // find second map entry
            coreUint16 iSecond = iNew;
            while(s_aiMap[iSecond] != iNew) iSecond = s_aiMap[iSecond];

            // swap indices (in map)
            s_aiMap[iFirst]  = iNew;
            s_aiMap[iSecond] = iOld;

            // swap vertices
            std::swap(oImport.aVertexData[iNew], oImport.aVertexData[iOld]);
        }
    }

    // remap all indices
    for(coreUintW i = 0u, ie = m_iNumIndices; i < ie; ++i)
        piOptimizedData[i] = s_aiMap[piOptimizedData[i]];

    // analyze all vertices
    coreVector3 vRangeMin = coreVector3( FLT_MAX, FLT_MAX, FLT_MAX);
    coreVector3 vRangeMax = coreVector3(-FLT_MAX,-FLT_MAX,-FLT_MAX);
    coreDouble  adSum[3]  = {};
    FOR_EACH(it, oImport.aVertexData)
    {
        // check for valid vertex attributes
        ASSERT((0.0f <= it->vTexCoord.x)    && (it->vTexCoord.x <= 1.0f) &&
               (0.0f <= it->vTexCoord.y)    && (it->vTexCoord.y <= 1.0f) &&
               (it->vNormal.IsNormalized()) && (it->vTangent.xyz().IsNormalized()))

        // find maximum distances from the model center
        vRangeMin.x       = MIN(vRangeMin.x,       it->vPosition.x);
        vRangeMin.y       = MIN(vRangeMin.y,       it->vPosition.y);
        vRangeMin.z       = MIN(vRangeMin.z,       it->vPosition.z);
        vRangeMax.x       = MAX(vRangeMax.x,       it->vPosition.x);
        vRangeMax.y       = MAX(vRangeMax.y,       it->vPosition.y);
        vRangeMax.z       = MAX(vRangeMax.z,       it->vPosition.z);
        m_fBoundingRadius = MAX(m_fBoundingRadius, it->vPosition.LengthSq());
        adSum[0]         += coreDouble(it->vPosition.x);
        adSum[1]         += coreDouble(it->vPosition.y);
        adSum[2]         += coreDouble(it->vPosition.z);
    }
    m_vBoundingRange.x = MAX(ABS(vRangeMin.x), ABS(vRangeMax.x));
    m_vBoundingRange.y = MAX(ABS(vRangeMin.y), ABS(vRangeMax.y));
    m_vBoundingRange.z = MAX(ABS(vRangeMin.z), ABS(vRangeMax.z));
    m_fBoundingRadius  = SQRT(m_fBoundingRadius);
    m_vNaturalCenter   = (vRangeMax + vRangeMin) * 0.5f;
    m_vNaturalRange    = (vRangeMax - vRangeMin) * 0.5f;
    m_vWeightedCenter  = coreVector3(coreFloat(adSum[0] / coreDouble(m_iNumVertices)),
                                     coreFloat(adSum[1] / coreDouble(m_iNumVertices)),
                                     coreFloat(adSum[2] / coreDouble(m_iNumVertices)));

    if(!HAS_FLAG(m_eLoad, CORE_MODEL_LOAD_NO_CLUSTERS))
    {
        // store vertex positions
        m_pvVertexPosition = ALIGNED_NEW(coreVector3, m_iNumVertices, ALIGNMENT_CACHE);
        for(coreUintW i = 0u, ie = m_iNumVertices; i < ie; ++i) m_pvVertexPosition[i] = oImport.aVertexData[i].vPosition;

        // get range factor for target cluster calculations
        const coreVector3 vRangeDiff = coreVector3(1.0f,1.0f,1.0f) / (vRangeMax - vRangeMin).Processed(MAX, CORE_MATH_PRECISION);

        // assign triangles to different clusters based on their vertex positions (uniform grid)
        coreList<coreUint16> aiTempIndex[CORE_MODEL_CLUSTERS_MAX];
        for(coreUintW i = 0u, ie = m_iNumIndices; i < ie; i += 3u)
        {
            // calculate triangle (bounding) center
            const coreVector3 vPos1      = m_pvVertexPosition[piOptimizedData[i]];
            const coreVector3 vPos2      = m_pvVertexPosition[piOptimizedData[i+1u]];
            const coreVector3 vPos3      = m_pvVertexPosition[piOptimizedData[i+2u]];
            const coreVector3 vCenterPos = (coreVector3(MIN(vPos1.x, vPos2.x, vPos3.x), MIN(vPos1.y, vPos2.y, vPos3.y), MIN(vPos1.z, vPos2.z, vPos3.z)) +
                                            coreVector3(MAX(vPos1.x, vPos2.x, vPos3.x), MAX(vPos1.y, vPos2.y, vPos3.y), MAX(vPos1.z, vPos2.z, vPos3.z))) * 0.5f;

            // calculate target cluster
            const coreVector3 vRangePos = ((vCenterPos - vRangeMin) * vRangeDiff).Processed(CLAMP, 0.0f, 1.0f - CORE_MATH_PRECISION) * I_TO_F(CORE_MODEL_CLUSTERS_AXIS);
            const coreUintW   iIndex    = (F_TO_UI(vRangePos.x) * CORE_MODEL_CLUSTERS_AXIS * CORE_MODEL_CLUSTERS_AXIS) +
                                          (F_TO_UI(vRangePos.y) * CORE_MODEL_CLUSTERS_AXIS)                            +
                                          (F_TO_UI(vRangePos.z));

            // reserve memory only for occupied clusters
            ASSERT(iIndex < CORE_MODEL_CLUSTERS_MAX)
            aiTempIndex[iIndex].reserve(m_iNumIndices >> 4u);

            // assign triangle indices to target cluster
            aiTempIndex[iIndex].push_back(piOptimizedData[i]);
            aiTempIndex[iIndex].push_back(piOptimizedData[i+1u]);
            aiTempIndex[iIndex].push_back(piOptimizedData[i+2u]);
        }

        // reorder clusters to compact list
        std::sort(aiTempIndex, aiTempIndex + CORE_MODEL_CLUSTERS_MAX, [](const coreList<coreUint16>& A, const coreList<coreUint16>& B)
        {
            if(A.empty()) return false;
            if(B.empty()) return true;
            return (A[0] != B[0]) ? (A[0] < B[0]) :
                   (A[1] != B[1]) ? (A[1] < B[1]) :
                                    (A[2] < B[2]);
        });

        // save number of clusters
        m_iNumClusters = std::find_if(aiTempIndex, aiTempIndex + CORE_MODEL_CLUSTERS_MAX, [](const coreList<coreUint16>& A) {return A.empty();}) - aiTempIndex;
        ASSERT(m_iNumClusters)

        // allocate cluster memory
        coreByte* pIndexMemory = ALIGNED_NEW(coreByte,    m_iNumClusters * sizeof(coreUint16*) + m_iNumIndices * sizeof(coreUint16), ALIGNMENT_CACHE);
        m_piClusterNumIndices  = ALIGNED_NEW(coreUint16,  m_iNumClusters,                                                            ALIGNMENT_CACHE);
        m_pvClusterPosition    = ALIGNED_NEW(coreVector3, m_iNumClusters,                                                            ALIGNMENT_CACHE);
        m_pfClusterRadius      = ALIGNED_NEW(coreFloat,   m_iNumClusters,                                                            ALIGNMENT_CACHE);

        // prepare index pointers to use only single allocation (also to keep indirections as near as possible)
        m_ppiClusterIndex    = r_cast<coreUint16**>(pIndexMemory);
        m_ppiClusterIndex[0] = r_cast<coreUint16*> (pIndexMemory + m_iNumClusters * sizeof(coreUint16*));
        for(coreUintW i = 1u, ie = m_iNumClusters; i < ie; ++i) m_ppiClusterIndex[i] = m_ppiClusterIndex[0];

        for(coreUintW i = 0u, ie = m_iNumClusters; i < ie; ++i)
        {
            // save number of indices
            m_piClusterNumIndices[i] = aiTempIndex[i].size();

            // adjust index pointers and store indices
            for(coreUintW j = i + 1u, je = m_iNumClusters; j < je; ++j) m_ppiClusterIndex[j] += m_piClusterNumIndices[i];
            std::memcpy(m_ppiClusterIndex[i], aiTempIndex[i].data(), m_piClusterNumIndices[i] * sizeof(coreUint16));

            // find the cluster center
            coreVector3 vClusterMin = coreVector3( FLT_MAX, FLT_MAX, FLT_MAX);
            coreVector3 vClusterMax = coreVector3(-FLT_MAX,-FLT_MAX,-FLT_MAX);
            for(coreUintW j = 0u, je = m_piClusterNumIndices[i]; j < je; ++j)
            {
                const coreVector3 vPosition = m_pvVertexPosition[m_ppiClusterIndex[i][j]];

                vClusterMin.x = MIN(vClusterMin.x, vPosition.x);
                vClusterMin.y = MIN(vClusterMin.y, vPosition.y);
                vClusterMin.z = MIN(vClusterMin.z, vPosition.z);
                vClusterMax.x = MAX(vClusterMax.x, vPosition.x);
                vClusterMax.y = MAX(vClusterMax.y, vPosition.y);
                vClusterMax.z = MAX(vClusterMax.z, vPosition.z);
            }
            m_pvClusterPosition[i] = (vClusterMax + vClusterMin) * 0.5f;

            // find maximum distance from the cluster center
            coreFloat fClusterRadiusSq = 0.0f;
            for(coreUintW j = 0u, je = m_piClusterNumIndices[i]; j < je; ++j)
            {
                fClusterRadiusSq = MAX(fClusterRadiusSq, (m_pvVertexPosition[m_ppiClusterIndex[i][j]] - m_pvClusterPosition[i]).LengthSq());
            }
            m_pfClusterRadius[i] = SQRT(fClusterRadiusSq);
        }
    }

    if(!HAS_FLAG(m_eLoad, CORE_MODEL_LOAD_NO_BUFFERS))
    {
        if(CORE_GL_SUPPORT(ARB_vertex_type_2_10_10_10_rev) && CORE_GL_SUPPORT(ARB_half_float_vertex))
        {
            // reduce total vertex size (high quality compression, de-interleaved)
            coreByte* pPackedData = new coreByte[20u * m_iNumVertices];
            for(coreUintW i = 0u, ie = m_iNumVertices; i < ie; ++i)
            {
                const coreVertex& oVertex = oImport.aVertexData[i];

                // convert vertex attributes
                r_cast<coreUint64*>(pPackedData)                       [i] = coreVector4(oVertex.vPosition, 0.0f).PackFloat4x16();
                r_cast<coreUint32*>(pPackedData +  8u * m_iNumVertices)[i] = oVertex.vTexCoord                   .PackUnorm2x16();
                r_cast<coreUint32*>(pPackedData + 12u * m_iNumVertices)[i] = coreVector4(oVertex.vNormal,   0.0f).PackSnorm210 ();
                r_cast<coreUint32*>(pPackedData + 16u * m_iNumVertices)[i] = oVertex.vTangent                    .PackSnorm210 ();
            }

            // create vertex buffer
            coreVertexBuffer* pBuffer = this->CreateVertexBuffer(m_iNumVertices, 20u, pPackedData, CORE_DATABUFFER_STORAGE_STATIC);
            pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 4u, GL_HALF_FLOAT,         8u, false, 0u,  0u);
            pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TEXCOORD_NUM, 2u, GL_UNSIGNED_SHORT,     4u, false, 8u,  0u);
            pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   4u, GL_INT_2_10_10_10_REV, 4u, false, 12u, 0u);
            pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  4u, GL_INT_2_10_10_10_REV, 4u, false, 16u, 0u);
            SAFE_DELETE_ARRAY(pPackedData)
        }
        else
        {
            // reduce total vertex size (low quality compression, de-interleaved)
            coreByte* pPackedData = new coreByte[24u * m_iNumVertices];
            for(coreUintW i = 0u, ie = m_iNumVertices; i < ie; ++i)
            {
                const coreVertex& oVertex = oImport.aVertexData[i];

                // convert vertex attributes
                r_cast<coreVector3*>(pPackedData)                       [i] = oVertex.vPosition;
                r_cast<coreUint32*> (pPackedData + 12u * m_iNumVertices)[i] = oVertex.vTexCoord                 .PackUnorm2x16();
                r_cast<coreUint32*> (pPackedData + 16u * m_iNumVertices)[i] = coreVector4(oVertex.vNormal, 0.0f).PackSnorm4x8 ();
                r_cast<coreUint32*> (pPackedData + 20u * m_iNumVertices)[i] = oVertex.vTangent                  .PackSnorm4x8 ();
            }

            // create vertex buffer
            coreVertexBuffer* pBuffer = this->CreateVertexBuffer(m_iNumVertices, 24u, pPackedData, CORE_DATABUFFER_STORAGE_STATIC);
            pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 3u, GL_FLOAT,          12u, false, 0u,  0u);
            pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TEXCOORD_NUM, 2u, GL_UNSIGNED_SHORT, 4u,  false, 12u, 0u);
            pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   4u, GL_BYTE,           4u,  false, 16u, 0u);
            pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  4u, GL_BYTE,           4u,  false, 20u, 0u);
            SAFE_DELETE_ARRAY(pPackedData)
        }

        // create index buffer
        this->CreateIndexBuffer(m_iNumIndices, sizeof(coreUint16), piOptimizedData, CORE_DATABUFFER_STORAGE_STATIC);

        // add debug label
        Core::Graphics->LabelOpenGL(GL_BUFFER, m_aVertexBuffer.front().GetIdentifier(), PRINT("%s.vertex", m_sName.c_str()));
        Core::Graphics->LabelOpenGL(GL_BUFFER, m_IndexBuffer          .GetIdentifier(), PRINT("%s.index",  m_sName.c_str()));
    }

    // free index data
    SAFE_DELETE_ARRAY(piOptimizedData)

    Core::Log->Info("Model (%s, %u vertices, %u indices, %u clusters, %.5f x %.5f x %.5f range, %.5f radius) loaded", m_sName.c_str(), m_iNumVertices, m_iNumIndices, m_iNumClusters, m_vBoundingRange.x, m_vBoundingRange.y, m_vBoundingRange.z, m_fBoundingRadius);
    return m_Sync.Create(CORE_SYNC_CREATE_FLUSHED) ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
/* unload model resource data */
coreStatus coreModel::Unload()
{
    if(!m_iNumVertices) return CORE_INVALID_CALL;

    // disable still active model
    if(s_pCurrent == this) coreModel::Disable(true);

    // delete all data buffers
    m_aVertexBuffer.clear();
    m_IndexBuffer.Delete();

    // free cluster memory
    if(m_iNumClusters)
    {
        ALIGNED_DELETE(m_pvVertexPosition)
        ALIGNED_DELETE(m_ppiClusterIndex)
        ALIGNED_DELETE(m_piClusterNumIndices)
        ALIGNED_DELETE(m_pvClusterPosition)
        ALIGNED_DELETE(m_pfClusterRadius)
    }

    // delete vertex array object
    if(m_iVertexArray) coreDelVertexArrays(1u, &m_iVertexArray);
    if(!m_sName.empty()) Core::Log->Info("Model (%s) unloaded", m_sName.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_iVertexArray    = 0u;
    m_iNumVertices    = 0u;
    m_iNumIndices     = 0u;
    m_iNumClusters    = 0u;
    m_vBoundingRange  = coreVector3(0.0f,0.0f,0.0f);
    m_fBoundingRadius = 0.0f;
    m_vNaturalCenter  = coreVector3(0.0f,0.0f,0.0f);
    m_vNaturalRange   = coreVector3(0.0f,0.0f,0.0f);
    m_vWeightedCenter = coreVector3(0.0f,0.0f,0.0f);
    m_iPrimitiveType  = GL_TRIANGLES;
    m_iIndexType      = 0u;

    return CORE_OK;
}


// ****************************************************************
/* draw the model */
void coreModel::DrawArrays()const
{
    // draw the model (without index buffer)
    ASSERT((s_pCurrent == this) || !s_pCurrent)
    glDrawArrays(m_iPrimitiveType, 0, m_iNumVertices);
}

void coreModel::DrawElements()const
{
    // draw the model (with index buffer)
    ASSERT(((s_pCurrent == this) || !s_pCurrent) && m_IndexBuffer.IsValid())
    glDrawRangeElements(m_iPrimitiveType, 0u, m_iNumVertices - 1u, m_iNumIndices, m_iIndexType, NULL);
}


// ****************************************************************
/* draw the model instanced */
void coreModel::DrawArraysInstanced(const coreUint32 iCount)const
{
    // draw the model instanced (without index buffer)
    ASSERT(((s_pCurrent == this) || !s_pCurrent) && iCount)
    glDrawArraysInstanced(m_iPrimitiveType, 0, m_iNumVertices, iCount);
}

void coreModel::DrawElementsInstanced(const coreUint32 iCount)const
{
    // draw the model instanced (with index buffer)
    ASSERT(((s_pCurrent == this) || !s_pCurrent) && m_IndexBuffer.IsValid() && iCount)
    glDrawElementsInstanced(m_iPrimitiveType, m_iNumIndices, m_iIndexType, NULL, iCount);
}


// ****************************************************************
/* enable the model */
void coreModel::Enable()
{
    ASSERT(!m_aVertexBuffer.empty())

    // check and save current model object
    if(s_pCurrent == this) return;
    s_pCurrent = this;

    // bind vertex array object
    if(m_iVertexArray) glBindVertexArray(m_iVertexArray);
    else
    {
        if(CORE_GL_SUPPORT(ARB_vertex_array_object))
        {
            // create vertex array object
            coreGenVertexArrays(1u, &m_iVertexArray);
            glBindVertexArray(m_iVertexArray);

            // force binding of index data
            coreDataBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER, false);

            // add debug label
            Core::Graphics->LabelOpenGL(GL_VERTEX_ARRAY, m_iVertexArray, m_sName.c_str());
        }

        // set vertex data
        for(coreUintW i = 0u, ie = m_aVertexBuffer.size(); i < ie; ++i)
            m_aVertexBuffer[i].Activate(0u);

        // set index data
        if(m_IndexBuffer.IsValid()) m_IndexBuffer.Bind();
    }
}


// ****************************************************************
/* disable the model */
void coreModel::Disable(const coreBool bFull)
{
    // reset current model object
    s_pCurrent = NULL;

    coreBool bFullUnbind = false;
    if(bFull)
    {
        // unbind vertex array object
        if(CORE_GL_SUPPORT(ARB_vertex_array_object)) glBindVertexArray(0u);
        else bFullUnbind = true;
    }

    // unbind data buffers
    coreDataBuffer::Unbind(GL_ARRAY_BUFFER,         bFullUnbind);
    coreDataBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER, bFullUnbind);
}


// ****************************************************************
/* create vertex buffer */
coreVertexBuffer* coreModel::CreateVertexBuffer(const coreUint32 iNumVertices, const coreUint8 iVertexSize, const void* pVertexData, const coreDataBufferStorage eStorageType)
{
    ASSERT(!m_iVertexArray)

    // save properties
    if(m_aVertexBuffer.empty()) m_iNumVertices = iNumVertices;
    ASSERT(m_iNumVertices && (m_iNumVertices == iNumVertices))

    // create vertex buffer
    m_aVertexBuffer.emplace_back();
    m_aVertexBuffer.back().Create(iNumVertices, iVertexSize, pVertexData, eStorageType);

    // disable current model object (to fully enable the next model)
    coreModel::Disable(false);

    return &m_aVertexBuffer.back();
}


// ****************************************************************
/* create index buffer */
coreDataBuffer* coreModel::CreateIndexBuffer(const coreUint32 iNumIndices, const coreUint8 iIndexSize, const void* pIndexData, const coreDataBufferStorage eStorageType)
{
    ASSERT(!m_iVertexArray && !m_IndexBuffer.IsValid())

    // save properties
    m_iNumIndices = iNumIndices;
    ASSERT(m_iNumIndices)

    // detect index type
    switch(iIndexSize)
    {
    default: UNREACHABLE
    case 4u: m_iIndexType = GL_UNSIGNED_INT;   break;
    case 2u: m_iIndexType = GL_UNSIGNED_SHORT; break;
    case 1u: m_iIndexType = GL_UNSIGNED_BYTE;  break;
    }

    // disable current model object (to unbind current VAO)
    coreModel::Disable(true);

    // create index buffer
    m_IndexBuffer.Create(GL_ELEMENT_ARRAY_BUFFER, iNumIndices*iIndexSize, pIndexData, eStorageType);

    return &m_IndexBuffer;
}