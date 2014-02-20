//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreModel*   coreModel::s_pCurrent = NULL;
SDL_SpinLock coreModel::s_iLock    = 0;


// ****************************************************************
// constructor
coreModel::md5Joint::md5Joint(const char** ppcData)noexcept
{
    CORE_DATA_SCAN(*ppcData, "%*s %d %*s %f %f %f %*s %*s %f %f %f %*s",
                   &iParent,
                   &vPosition.x,    &vPosition.y,    &vPosition.z,
                   &vOrientation.x, &vOrientation.y, &vOrientation.z)

    // calculate w-component of the normalized quaternion
    const coreVector4 V = vOrientation * vOrientation;
    const float       T = 1.0f - V.x - V.y - V.z;
    vOrientation.w = (T < 0.0f) ? 0.0f : -coreMath::Sqrt(T);
}


// ****************************************************************
// constructor
coreModel::md5Vertex::md5Vertex(const char** ppcData)noexcept
{
    CORE_DATA_SCAN(*ppcData, "%*s %*d %*s %f %f %*s %d %d",
                   &vTexture.x,   &vTexture.y,
                   &iWeightStart, &iWeightCount)
}


// ****************************************************************
// constructor
coreModel::md5Triangle::md5Triangle(const char** ppcData)noexcept
{
    CORE_DATA_SCAN(*ppcData, "%*s %*d %hu %hu %hu",
                   &aiVertex[0], &aiVertex[2], &aiVertex[1])
}


// ****************************************************************
// constructor
coreModel::md5Weight::md5Weight(const char** ppcData)noexcept
{
    CORE_DATA_SCAN(*ppcData, "%*s %*d %d %f %*s %f %f %f %*s",
                   &iJoint,      &fBias,
                   &vPosition.x, &vPosition.y, &vPosition.z)
}


// ****************************************************************
// constructor
coreModel::md5Mesh::md5Mesh(const char** ppcData)noexcept
{
    int iNum = 0;

    CORE_DATA_SCAN(*ppcData, "%*s %*s %*s %d", &iNum)
    aVertex.reserve(iNum);
    for(int i = 0; i < iNum; ++i) aVertex.push_back(md5Vertex(ppcData));

    CORE_DATA_SCAN(*ppcData, "%*s %d", &iNum)
    aTriangle.reserve(iNum);
    for(int i = 0; i < iNum; ++i) aTriangle.push_back(md5Triangle(ppcData));

    CORE_DATA_SCAN(*ppcData, "%*s %d", &iNum)
    aWeight.reserve(iNum);
    for(int i = 0; i < iNum; ++i) aWeight.push_back(md5Weight(ppcData));
}

coreModel::md5Mesh::md5Mesh(md5Mesh&& m)noexcept
: aVertex   (std::move(m.aVertex))
, aTriangle (std::move(m.aTriangle))
, aWeight   (std::move(m.aWeight))
{
}


// ****************************************************************
// constructor
coreModel::md5File::md5File(const char** ppcData)noexcept
{
    char acIdentifier[16] = "";
    int iVersion          = 0;
    int iNumJoints        = 0;
    int iNumMeshes        = 0;

    // check for correct file type
    CORE_DATA_SCAN(*ppcData, "")
    CORE_DATA_SCAN(*ppcData, "%15s %d", (char*)&acIdentifier, &iVersion)
    if(std::strcmp(acIdentifier, "MD5Version") || iVersion != 10) return;

    // read number of objects
    CORE_DATA_SCAN(*ppcData, "%*s %*s")
    CORE_DATA_SCAN(*ppcData, "%*s %d", &iNumJoints)
    CORE_DATA_SCAN(*ppcData, "%*s %d", &iNumMeshes)
    aJoint.reserve(iNumJoints);
    aMesh.reserve(iNumMeshes);

    // read joint and mesh data
    for(int i = 0; i < iNumMeshes+1; ++i)
    {
        CORE_DATA_SCAN(*ppcData, "%*s")
        CORE_DATA_SCAN(*ppcData, "%*s")

        if(i) aMesh.push_back(std::move(md5Mesh(ppcData)));
        else for(int i = 0; i < iNumJoints; ++i) aJoint.push_back(md5Joint(ppcData));

        CORE_DATA_SCAN(*ppcData, "%*s")
    }
}

coreModel::md5File::md5File(md5File&& m)noexcept
: aJoint (std::move(m.aJoint))
, aMesh  (std::move(m.aMesh))
{
}


// ****************************************************************
// constructor
coreModel::coreModel()noexcept
: m_iVertexArray   (0)
, m_iNumVertices   (0)
, m_iNumTriangles  (0)
, m_iNumIndices    (0)
, m_fRadius        (0.0f)
, m_iPrimitiveType (GL_TRIANGLES)
, m_iIndexType     (0)
{
    // reserve memory for vertex buffers
    m_apiVertexBuffer.reserve(1);
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
    const coreError iStatus = m_Sync.Check(0);
    if(iStatus >= 0) return iStatus;

    coreFileUnload Unload(pFile);

    ASSERT_IF(!m_apiVertexBuffer.empty()) return CORE_INVALID_CALL;
    if(!pFile)                            return CORE_INVALID_INPUT;
    if(!pFile->GetData())                 return CORE_FILE_ERROR;

    // extract model data
    const char* pcData = r_cast<const char*>(pFile->GetData());
    const md5File oFile(&pcData);

    // check for success
    if(oFile.aMesh.empty())
    {
        Core::Log->Error(false, "Model (%s) is not a valid MD5-file", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // take data from the first mesh
    const md5Mesh& oMesh = oFile.aMesh[0];

    // save attributes
    m_iNumVertices  = oMesh.aVertex.size();
    m_iNumTriangles = oMesh.aTriangle.size();
    m_iNumIndices   = oMesh.aTriangle.size()*3;
    m_sPath         = pFile->GetPath();
    m_iSize         = m_iNumVertices*sizeof(coreVertex) + m_iNumIndices*sizeof(coreWord);

    // reserve required vertex memory
    coreVertex*  pVertex  = new coreVertex [m_iNumVertices]; // std::memset(pVertex,  0, m_iNumVertices*sizeof(pVertex[0]));
    coreVector3* pvOrtho1 = new coreVector3[m_iNumVertices]; // std::memset(pvOrtho1, 0, m_iNumVertices*sizeof(pvOrtho1[0]));
    coreVector3* pvOrtho2 = new coreVector3[m_iNumVertices]; // std::memset(pvOrtho2, 0, m_iNumVertices*sizeof(pvOrtho2[0]));

    // traverse all vertices
    for(coreUint i = 0; i < m_iNumVertices; ++i)
    {
        const md5Vertex& oVertex = oMesh.aVertex[i];

        // calculate vertex position
        for(int j = 0; j < oVertex.iWeightCount; ++j)
        {
            const md5Weight& oWeight = oMesh.aWeight[j + oVertex.iWeightStart];
            const md5Joint&  oJoint  = oFile.aJoint[oWeight.iJoint];
            pVertex[i].vPosition += (oJoint.vPosition + oJoint.vOrientation.QuatApply(oWeight.vPosition)) * oWeight.fBias;
        }

        // forward texture coordinate
        pVertex[i].vTexture = oVertex.vTexture;

        // find maximum distance from the model center
        m_fRadius = MAX(pVertex[i].vPosition.LengthSq(), m_fRadius);
    }
    m_fRadius = coreMath::Sqrt(m_fRadius);

    // traverse all triangles
    for(coreUint i = 0; i < m_iNumTriangles; ++i)
    {
        const md5Triangle& oTriangle = oMesh.aTriangle[i];

        // calculate triangle sides
        const coreVector3 A1 = pVertex[oTriangle.aiVertex[1]].vPosition - pVertex[oTriangle.aiVertex[0]].vPosition;
        const coreVector3 A2 = pVertex[oTriangle.aiVertex[2]].vPosition - pVertex[oTriangle.aiVertex[0]].vPosition;
        const coreVector2 B1 = pVertex[oTriangle.aiVertex[1]].vTexture  - pVertex[oTriangle.aiVertex[0]].vTexture;
        const coreVector2 B2 = pVertex[oTriangle.aiVertex[2]].vTexture  - pVertex[oTriangle.aiVertex[0]].vTexture;

        // calculate local normal vector
        const coreVector3 N = coreVector3::Cross(A1.Normalized(), A2.Normalized());

        // calculate local tangent vector parameters
        const float R = 1.0f / (B1.s*B2.t - B2.s*B1.t);
        const coreVector3 D1 = (A1*B2.t - A2*B1.t) * R;
        const coreVector3 D2 = (A2*B1.s - A1*B2.s) * R;

        for(int j = 0; j < 3; ++j)
        {
            // add local values to each point of the triangle
            pVertex[oTriangle.aiVertex[j]].vNormal += N;
            pvOrtho1[oTriangle.aiVertex[j]] += D1;
            pvOrtho2[oTriangle.aiVertex[j]] += D2;
        }
    }
    for(coreUint i = 0; i < m_iNumVertices; ++i)
    {
        // normalize the normal vector
        pVertex[i].vNormal.Normalize();

        // finish the Gram-Schmidt process to calculate the tangent vector and binormal sign (w)
        pVertex[i].vTangent = coreVector4((pvOrtho1[i] - pVertex[i].vNormal * coreVector3::Dot(pVertex[i].vNormal, pvOrtho1[i])).Normalize(),
                                          coreMath::Sign(coreVector3::Dot(coreVector3::Cross(pVertex[i].vNormal, pvOrtho1[i]), pvOrtho2[i])));
    }

    // create vertex buffer
    coreVertexBuffer* pBuffer = this->CreateVertexBuffer(m_iNumVertices, sizeof(coreVertex), pVertex, GL_STATIC_DRAW);
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 3, GL_FLOAT, 0);
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  2, GL_FLOAT, 3*sizeof(float));
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   3, GL_FLOAT, 5*sizeof(float));
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  4, GL_FLOAT, 8*sizeof(float));

    // create index buffer
    this->CreateIndexBuffer(m_iNumIndices, sizeof(coreWord), oMesh.aTriangle.data(), GL_STATIC_DRAW);

    // create sync object
    const bool bSync = m_Sync.Create();

    // free required vertex memory
    SAFE_DELETE_ARRAY(pVertex)
    SAFE_DELETE_ARRAY(pvOrtho1)
    SAFE_DELETE_ARRAY(pvOrtho2)

    Core::Log->Info("Model (%s) loaded", pFile->GetPath());
    return bSync ? CORE_BUSY : CORE_OK;
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

    // reset attributes
    m_sPath          = "";
    m_iSize          = 0;
    m_iVertexArray   = 0;
    m_iNumVertices   = 0;
    m_iNumTriangles  = 0;
    m_iNumIndices    = 0;
    m_fRadius        = 0.0f;
    m_iPrimitiveType = GL_TRIANGLES;
    m_iIndexType     = 0;

    return CORE_OK;
}


// ****************************************************************
// draw the model with indices
void coreModel::DrawElements()const
{
    coreModel::Lock();
    {
        // check and draw the model
        SDL_assert(s_pCurrent == this && m_iIndexBuffer);
        glDrawRangeElements(m_iPrimitiveType, 0, m_iNumVertices, m_iNumIndices, m_iIndexType, 0);
    }
    coreModel::Unlock();
}

void coreModel::DrawElementsInstanced(const coreUint& iCount)const
{
    coreModel::Lock();
    {
        // check and draw the model instanced
        SDL_assert(s_pCurrent == this && m_iIndexBuffer);
        glDrawElementsInstanced(m_iPrimitiveType, m_iNumIndices, m_iIndexType, 0, iCount);
    }
    coreModel::Unlock();
}


// ****************************************************************
// draw the model without indices
void coreModel::DrawArrays()const
{
    // check and draw the model
    SDL_assert(s_pCurrent == this);
    glDrawArrays(m_iPrimitiveType, 0, m_iNumVertices);
}

void coreModel::DrawArraysInstanced(const coreUint& iCount)const
{
    // check and draw the model instanced
    SDL_assert(s_pCurrent == this);
    glDrawArraysInstanced(m_iPrimitiveType, 0, m_iNumVertices, iCount);
}


// ****************************************************************
// enable the model
void coreModel::Enable()
{
    SDL_assert(!m_apiVertexBuffer.empty());

    // check current model object
    if(s_pCurrent == this) return;

    coreModel::Lock();
    {
        // save current model object
        s_pCurrent = this;

        // bind vertex array object
        if(m_iVertexArray) glBindVertexArray(m_iVertexArray);
        else
        {
            if(GLEW_ARB_vertex_array_object)
            {
                // create vertex array object
                glGenVertexArrays(1, &m_iVertexArray);
                glBindVertexArray(m_iVertexArray);
            }
            
            // set vertex data
            for(coreUint i = 0; i < m_apiVertexBuffer.size(); ++i)
                m_apiVertexBuffer[i]->Activate(i);

            // set index data
            if(m_iIndexBuffer) m_iIndexBuffer.Bind();
        }
    }
    coreModel::Unlock();
}


// ****************************************************************
// disable the model
void coreModel::Disable(const bool& bFull)
{
    if(!s_pCurrent) return;

    coreModel::Lock();
    {
        if(bFull)
        {
            // reset vertex array object and data buffers
            if(s_pCurrent->GetVertexArray()) glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER,         0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        // reset current model object
        s_pCurrent = NULL;
    }
    coreModel::Unlock();
}


// ****************************************************************
// create vertex buffer
coreVertexBuffer* coreModel::CreateVertexBuffer(const coreUint& iNumVertices, const coreByte& iVertexSize, const void* pVertexData, const GLenum& iUsage)
{
    SDL_assert(!m_iVertexArray);

    // save attributes
    m_iNumVertices = iNumVertices;

    // allocate vertex buffer
    coreVertexBuffer* pBuffer = new coreVertexBuffer();

    coreModel::Lock();
    {
        // create vertex buffer
        pBuffer->Create(iNumVertices, iVertexSize, pVertexData, iUsage);
        m_apiVertexBuffer.push_back(pBuffer);
    }
    coreModel::Unlock();

    return pBuffer;
}


// ****************************************************************
// create index buffer
coreDataBuffer* coreModel::CreateIndexBuffer(const coreUint& iNumIndices, const coreByte& iIndexSize, const void* pIndexData, const GLenum& iUsage)
{
    SDL_assert(!m_iVertexArray && !m_iIndexBuffer);

    // save attributes
    m_iNumIndices = iNumIndices;

    // detect index type
    switch(iIndexSize)
    {
    case 1:  m_iIndexType = GL_UNSIGNED_BYTE;  break;
    case 2:  m_iIndexType = GL_UNSIGNED_SHORT; break;
    case 4:  m_iIndexType = GL_UNSIGNED_INT;   break;
    default: SDL_assert(false);
    }

    coreModel::Lock();
    {
        // create index buffer
        m_iIndexBuffer.Create(GL_ELEMENT_ARRAY_BUFFER, iNumIndices*iIndexSize, pIndexData, iUsage);

        if(s_pCurrent)
        {
            // reset current index buffer
            if(s_pCurrent->GetIndexBuffer()->GetDataBuffer()) 
                s_pCurrent->GetIndexBuffer()->Bind();
        }
    }
    coreModel::Unlock();

    return &m_iIndexBuffer;
}