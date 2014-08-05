//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreModel* coreModel::s_pCurrent = NULL;


// ****************************************************************
/* move string pointer and skip comments */
template <char cDelimiter> static void SkipComments(const char** ppcInput)
{
    ASSERT(*ppcInput)

    // check for line-comments
    while(**ppcInput == cDelimiter)
    {
        // skip them
        int n = 0; 
        std::sscanf(*ppcInput, "%*[^\n] %n", &n);
        *ppcInput += n;
    }
}
#define MD5_SCAN(s,f,...) {int __n = 0; std::sscanf(s, f " %n", ##__VA_ARGS__, &__n); s += __n; SkipComments<'/'>(&(s));}


// ****************************************************************
// constructor
coreModel::md5Joint::md5Joint(const char** ppcData)noexcept
{
    MD5_SCAN(*ppcData, "%*s %d %*s %f %f %f %*s %*s %f %f %f %*s",
             &iParent,
             &vPosition.x,    &vPosition.y,    &vPosition.z,
             &vOrientation.x, &vOrientation.y, &vOrientation.z)

    // calculate w-component of the normalized quaternion
    const coreVector4 V = vOrientation * vOrientation;
    const float       T = 1.0f - V.x - V.y - V.z;
    vOrientation.w = (T < 0.0f) ? 0.0f : -SQRT(T);
}


// ****************************************************************
// constructor
coreModel::md5Vertex::md5Vertex(const char** ppcData)noexcept
{
    MD5_SCAN(*ppcData, "%*s %*d %*s %f %f %*s %d %d",
             &vTexture.x,   &vTexture.y,
             &iWeightStart, &iWeightCount)
}


// ****************************************************************
// constructor
coreModel::md5Triangle::md5Triangle(const char** ppcData)noexcept
{
    MD5_SCAN(*ppcData, "%*s %*d %hu %hu %hu",
             &aiVertex[0], &aiVertex[2], &aiVertex[1])
}


// ****************************************************************
// constructor
coreModel::md5Weight::md5Weight(const char** ppcData)noexcept
{
    MD5_SCAN(*ppcData, "%*s %*d %d %f %*s %f %f %f %*s",
             &iJoint,      &fBias,
             &vPosition.x, &vPosition.y, &vPosition.z)
}


// ****************************************************************
// constructor
coreModel::md5Mesh::md5Mesh(const char** ppcData)noexcept
{
    int iNum = 0;

    MD5_SCAN(*ppcData, "%*s %*s %*s %d", &iNum)
    aVertex.reserve(iNum);
    for(int i = 0; i < iNum; ++i) aVertex.push_back(md5Vertex(ppcData));

    MD5_SCAN(*ppcData, "%*s %d", &iNum)
    aTriangle.reserve(iNum);
    for(int i = 0; i < iNum; ++i) aTriangle.push_back(md5Triangle(ppcData));

    MD5_SCAN(*ppcData, "%*s %d", &iNum)
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
    MD5_SCAN(*ppcData, "")
    MD5_SCAN(*ppcData, "%15s %d", r_cast<char*>(&acIdentifier), &iVersion)
    if(std::strcmp(acIdentifier, "MD5Version") || iVersion != 10) return;

    // read number of objects
    MD5_SCAN(*ppcData, "%*s %*s")
    MD5_SCAN(*ppcData, "%*s %d", &iNumJoints)
    MD5_SCAN(*ppcData, "%*s %d", &iNumMeshes)
    aJoint.reserve(iNumJoints);
    aMesh.reserve(iNumMeshes);

    // read joint and mesh data
    for(int i = 0; i < iNumMeshes+1; ++i)
    {
        MD5_SCAN(*ppcData, "%*s")
        MD5_SCAN(*ppcData, "%*s")

        if(i) aMesh.push_back(std::move(md5Mesh(ppcData)));
        else for(int i = 0; i < iNumJoints; ++i) aJoint.push_back(md5Joint(ppcData));

        MD5_SCAN(*ppcData, "%*s")
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
    const coreError iCheck = m_Sync.Check(0);
    if(iCheck >= CORE_OK) return iCheck;

    coreFileUnload Unload(pFile);

    ASSERT_IF(!m_apiVertexBuffer.empty()) return CORE_INVALID_CALL;
    if(!pFile)                            return CORE_INVALID_INPUT;
    if(!pFile->GetData())                 return CORE_ERROR_FILE;

    // extract model data
    const char* pcData = r_cast<const char*>(pFile->GetData());
    const md5File oFile(&pcData);

    // check for success
    if(oFile.aMesh.empty())
    {
        Core::Log->Warning("Model (%s) is not a valid MD5-file", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // take data from the first mesh
    const md5Mesh& oMesh = oFile.aMesh[0];

    // save properties
    m_iNumVertices  = oMesh.aVertex.size();
    m_iNumTriangles = oMesh.aTriangle.size();
    m_iNumIndices   = oMesh.aTriangle.size()*3;
    m_sPath         = pFile->GetPath();
    m_iSize         = m_iNumVertices*sizeof(coreVertex) + m_iNumIndices*sizeof(coreUshort);

    // reserve required vertex memory
    coreVertex*  pVertex  = new coreVertex [m_iNumVertices];
    coreVector3* pvOrtho1 = new coreVector3[m_iNumVertices];
    coreVector3* pvOrtho2 = new coreVector3[m_iNumVertices];

    // loop through all vertices
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
    m_fRadius = SQRT(m_fRadius);

    // loop through all triangles
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
        const float R = RCP(B1.s*B2.t - B2.s*B1.t);
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
                                          SIGN(coreVector3::Dot(coreVector3::Cross(pVertex[i].vNormal, pvOrtho1[i]), pvOrtho2[i])));
    }

    // create vertex buffer
    coreVertexBuffer* pBuffer = this->CreateVertexBuffer(m_iNumVertices, sizeof(coreVertex), pVertex, CORE_DATABUFFER_STORAGE_STATIC);
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_POSITION_NUM, 3, GL_FLOAT, 0);
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  2, GL_FLOAT, 3*sizeof(float));
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   3, GL_FLOAT, 5*sizeof(float));
    pBuffer->DefineAttribute(CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  4, GL_FLOAT, 8*sizeof(float));

#if defined(_CORE_GLES_)

    if(m_iNumVertices <= 256)
    {
        // reduce default index data type size
        coreByte* pByteData = new coreByte[m_iNumIndices];
        for(coreUint i = 0; i < m_iNumTriangles; ++i)
        {
            for(coreByte j = 0; j < 3; ++j)
            {
                // convert all indices
                ASSERT(oMesh.aTriangle[i].aiVertex[j] < 256)
                pByteData[i*3 + j] = (coreByte)oMesh.aTriangle[i].aiVertex[j];
            }
        }

        // create small index buffer
        this->CreateIndexBuffer(m_iNumIndices, sizeof(coreByte), pByteData, CORE_DATABUFFER_STORAGE_STATIC);
        SAFE_DELETE_ARRAY(pByteData)
    }
    else 
       
#endif

    // create index buffer
    this->CreateIndexBuffer(m_iNumIndices, sizeof(coreUshort), oMesh.aTriangle.data(), CORE_DATABUFFER_STORAGE_STATIC);

    // free required vertex memory
    SAFE_DELETE_ARRAY(pVertex)
    SAFE_DELETE_ARRAY(pvOrtho1)
    SAFE_DELETE_ARRAY(pvOrtho2)

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
        if(GLEW_ARB_vertex_array_object)
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
        if(GLEW_ARB_vertex_array_object) glBindVertexArray(0);
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

    // allocate vertex buffer
    coreVertexBuffer* pBuffer = new coreVertexBuffer();

    // create vertex buffer
    pBuffer->Create(iNumVertices, iVertexSize, pVertexData, iStorageType);
    m_apiVertexBuffer.push_back(pBuffer);

    // disable current model object (to fully enable the next model) 
    coreModel::Disable(false);

    return pBuffer;
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
    default: ASSERT(false)
    case 4:  m_iIndexType = GL_UNSIGNED_INT;   break;
    case 2:  m_iIndexType = GL_UNSIGNED_SHORT; break;
    case 1:  m_iIndexType = GL_UNSIGNED_BYTE;  break;
    }

    // disable current model object (to unbind current VAO)
    coreModel::Disable(true);

    // create index buffer silently
    m_iIndexBuffer.Create (GL_ELEMENT_ARRAY_BUFFER, iNumIndices*iIndexSize, pIndexData, iStorageType);
    coreDataBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER, false);

    return &m_iIndexBuffer;
}