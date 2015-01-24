//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"
#include "models/MD3.h"
#include "models/MD5.h"

coreModel* coreModel::s_pCurrent = NULL;


// ****************************************************************
// constructor
coreModel::coreModel()noexcept
: m_iVertexArray    (0)
, m_iNumVertices    (0)
, m_iNumIndices     (0)
, m_vBoundingRange  (coreVector3(0.0f,0.0f,0.0f))
, m_fBoundingRadius (0.0f)
, m_iPrimitiveType  (GL_TRIANGLES)
, m_iIndexType      (0)
{
}


// ****************************************************************
// destructor
coreModel::~coreModel()
{
    this->Unload();
}


// ****************************************************************
// load model resource data
coreError coreModel::Load(coreFile* pFile)
{
    // check for sync object status
    const coreError iCheck = m_Sync.Check(0, CORE_SYNC_CHECK_FLUSHED);
    if(iCheck >= CORE_OK) return iCheck;

    coreFileUnload Unload(pFile);

    WARN_IF(!m_apiVertexBuffer.empty()) return CORE_INVALID_CALL;
    if(!pFile)                          return CORE_INVALID_INPUT;
    if(!pFile->GetData())               return CORE_ERROR_FILE;

    // extract file extension
    const char* pcExtension = coreData::StrLower(coreData::StrExtension(pFile->GetPath()));

    // import model file
    coreImport oImport;
         if(!std::strncmp(pcExtension, "md5", 3)) coreImportMD5(pFile->GetData(), &oImport);
    else if(!std::strncmp(pcExtension, "md3", 3)) coreImportMD3(pFile->GetData(), &oImport);
    else
    {
        Core::Log->Warning("Model (%s) could not be identified (valid extensions: md5[mesh], md3)", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // check for success
    if(oImport.aVertexData.empty())
    {
        Core::Log->Warning("Model (%s) could not be loaded", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // save properties
    m_iNumVertices = coreUint(oImport.aVertexData.size());
    m_iNumIndices  = coreUint(oImport.aiIndexData.size());
    m_sPath        = pFile->GetPath();
    m_iSize        = m_iNumVertices*sizeof(coreVertex) + m_iNumIndices*sizeof(coreUshort);

    // find maximum distance from the model center
    FOR_EACH(it, oImport.aVertexData)
    {
        m_vBoundingRange.x = MAX(m_vBoundingRange.x, it->vPosition.x);
        m_vBoundingRange.y = MAX(m_vBoundingRange.y, it->vPosition.y);
        m_vBoundingRange.z = MAX(m_vBoundingRange.z, it->vPosition.z);
        m_fBoundingRadius  = MAX(it->vPosition.LengthSq(), m_fBoundingRadius);
    }
    m_fBoundingRadius = SQRT(m_fBoundingRadius);

    GLuint iNormFormat;
    std::function<coreUint(const coreVector4&)> nPackFunc;
    if(CORE_GL_SUPPORT(ARB_vertex_type_2_10_10_10_rev))
    {
        // use high-precision packed format
        iNormFormat = GL_INT_2_10_10_10_REV;
        nPackFunc   = [](const coreVector4& vVector) {return vVector.PackSnorm210();};
    }
    else
    {
        // use low-precision byte format
        iNormFormat = GL_BYTE;
        nPackFunc   = [](const coreVector4& vVector) {return vVector.PackSnorm4x8();};
    }

    // reduce total vertex size
    coreVertexPacked* pPackedData = new coreVertexPacked[m_iNumVertices];
    for(coreUint i = 0; i < m_iNumVertices; ++i)
    {
        const coreVertex& oVertex = oImport.aVertexData[i];

        // check for valid attribute values
        ASSERT((0.0f <= oVertex.vTexCoord.x)    && (oVertex.vTexCoord.x <= 1.0f) &&
               (0.0f <= oVertex.vTexCoord.y)    && (oVertex.vTexCoord.y <= 1.0f) &&
               (oVertex.vNormal.IsNormalized()) && (oVertex.vTangent.xyz().IsNormalized()))

        // convert vertex attributes
        pPackedData[i].vPosition = oVertex.vPosition;
        pPackedData[i].iTexCoord = oVertex.vTexCoord.PackUnorm2x16();
        pPackedData[i].iNormal   = nPackFunc(coreVector4(oVertex.vNormal, 0.0f));
        pPackedData[i].iTangent  = nPackFunc(oVertex.vTangent);
    }

    // create vertex buffer
    coreVertexBuffer* pBuffer = this->CreateVertexBuffer(m_iNumVertices, sizeof(coreVertexPacked), pPackedData, CORE_DATABUFFER_STORAGE_STATIC);
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 3, GL_FLOAT,          0);
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TEXCOORD_NUM, 2, GL_UNSIGNED_SHORT, 3*sizeof(float));
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   4, iNormFormat,       3*sizeof(float) + 1*sizeof(coreUint));
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  4, iNormFormat,       3*sizeof(float) + 2*sizeof(coreUint));
    SAFE_DELETE_ARRAY(pPackedData)

#if defined(_CORE_GLES_)

    if(m_iNumVertices <= 256)
    {
        // reduce default index size
        coreByte* pByteData = new coreByte[m_iNumIndices];
        for(coreUint i = 0; i < m_iNumIndices; ++i)
        {
            // convert all indices
            ASSERT(oImport.aiIndexData[i] < 256)
            pByteData[i] = (coreByte)oImport.aiIndexData[i];
        }

        // create small index buffer
        this->CreateIndexBuffer(m_iNumIndices, sizeof(coreByte), pByteData, CORE_DATABUFFER_STORAGE_STATIC);
        SAFE_DELETE_ARRAY(pByteData)
    }
    else

#endif
    // create index buffer
    this->CreateIndexBuffer(m_iNumIndices, sizeof(coreUshort), oImport.aiIndexData.data(), CORE_DATABUFFER_STORAGE_STATIC);

    Core::Log->Info("Model (%s) loaded", pFile->GetPath());
    return m_Sync.Create() ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
// unload model resource data
coreError coreModel::Unload()
{
    if(m_apiVertexBuffer.empty()) return CORE_INVALID_CALL;

    // disable still active model
    if(s_pCurrent == this) coreModel::Disable(true);

    // delete all data buffers
    FOR_EACH(it, m_apiVertexBuffer) SAFE_DELETE(*it)
    m_apiVertexBuffer.clear();
    m_iIndexBuffer.Delete();

    // delete vertex array object
    if(m_iVertexArray) glDeleteVertexArrays(1, &m_iVertexArray);
    if(!m_sPath.empty()) Core::Log->Info("Model (%s) unloaded", m_sPath.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_sPath           = "";
    m_iSize           = 0;
    m_iVertexArray    = 0;
    m_iNumVertices    = 0;
    m_iNumIndices     = 0;
    m_vBoundingRange  = coreVector3(0.0f,0.0f,0.0f);
    m_fBoundingRadius = 0.0f;
    m_iPrimitiveType  = GL_TRIANGLES;
    m_iIndexType      = 0;

    return CORE_OK;
}


// ****************************************************************
// draw the model
void coreModel::DrawArrays()const
{
    // check and draw the model
    ASSERT(s_pCurrent == this || !s_pCurrent)
    glDrawArrays(m_iPrimitiveType, 0, m_iNumVertices);
}

void coreModel::DrawElements()const
{
    // check and draw the model
    ASSERT((s_pCurrent == this || !s_pCurrent) && m_iIndexBuffer)
    glDrawRangeElements(m_iPrimitiveType, 0, m_iNumVertices, m_iNumIndices, m_iIndexType, 0);
}


// ****************************************************************
// draw the model instanced
void coreModel::DrawArraysInstanced(const coreUint& iCount)const
{
    // check and draw the model instanced
    ASSERT(s_pCurrent == this || !s_pCurrent)
    glDrawArraysInstanced(m_iPrimitiveType, 0, m_iNumVertices, iCount);
}

void coreModel::DrawElementsInstanced(const coreUint& iCount)const
{
    // check and draw the model instanced
    ASSERT((s_pCurrent == this || !s_pCurrent) && m_iIndexBuffer)
    glDrawElementsInstanced(m_iPrimitiveType, m_iNumIndices, m_iIndexType, 0, iCount);
}


// ****************************************************************
// enable the model
void coreModel::Enable()
{
    ASSERT(!m_apiVertexBuffer.empty())

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
            glGenVertexArrays(1, &m_iVertexArray);
            glBindVertexArray(m_iVertexArray);
        }

        // set vertex data
        for(coreByte i = 0; i < m_apiVertexBuffer.size(); ++i)
            m_apiVertexBuffer[i]->Activate(i);

        // set index data
        if(m_iIndexBuffer) m_iIndexBuffer.Bind();
    }
}


// ****************************************************************
// disable the model
void coreModel::Disable(const bool& bFull)
{
    // reset current model object
    s_pCurrent = NULL;

    bool bFullUnbind = false;
    if(bFull)
    {
        // unbind vertex array object
        if(CORE_GL_SUPPORT(ARB_vertex_array_object)) glBindVertexArray(0);
        else bFullUnbind = true;
    }

    // unbind data buffers
    coreDataBuffer::Unbind(GL_ARRAY_BUFFER,         bFullUnbind);
    coreDataBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER, bFullUnbind);
}


// ****************************************************************
// create vertex buffer
coreVertexBuffer* coreModel::CreateVertexBuffer(const coreUint& iNumVertices, const coreByte& iVertexSize, const void* pVertexData, const coreDataBufferStorage& iStorageType)
{
    ASSERT(!m_iVertexArray)

    // save properties
    if(m_apiVertexBuffer.empty()) m_iNumVertices = iNumVertices;
    ASSERT(m_iNumVertices == iNumVertices)

    // create vertex buffer
    m_apiVertexBuffer.push_back(new coreVertexBuffer());
    m_apiVertexBuffer.back()->Create(iNumVertices, iVertexSize, pVertexData, iStorageType);

    // disable current model object (to fully enable the next model)
    coreModel::Disable(false);

    return m_apiVertexBuffer.back();
}


// ****************************************************************
// create index buffer
coreDataBuffer* coreModel::CreateIndexBuffer(const coreUint& iNumIndices, const coreByte& iIndexSize, const void* pIndexData, const coreDataBufferStorage& iStorageType)
{
    ASSERT(!m_iVertexArray && !m_iIndexBuffer)

    // save properties
    m_iNumIndices = iNumIndices;

    // detect index type
    switch(iIndexSize)
    {
    default: WARN_IF(true) {}
    case 4:  m_iIndexType = GL_UNSIGNED_INT;   break;
    case 2:  m_iIndexType = GL_UNSIGNED_SHORT; break;
    case 1:  m_iIndexType = GL_UNSIGNED_BYTE;  break;
    }

    // disable current model object (to unbind current VAO)
    coreModel::Disable(true);

    // create index buffer
    m_iIndexBuffer.Create (GL_ELEMENT_ARRAY_BUFFER, iNumIndices*iIndexSize, pIndexData, iStorageType);
    coreDataBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER, false);

    return &m_iIndexBuffer;
}