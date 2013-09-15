//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

SDL_SpinLock coreModel::s_iLock = 0;


// ****************************************************************
// constructor
coreModel::md5Joint::md5Joint(const char** ppcData)
{
    SCAN_DATA(*ppcData, "%*s %d %*s %f %f %f %*s %*s %f %f %f %*s",
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
coreModel::md5Vertex::md5Vertex(const char** ppcData)
{
    SCAN_DATA(*ppcData, "%*s %*d %*s %f %f %*s %d %d",
              &vTexture.x,   &vTexture.y,
              &iWeightStart, &iWeightCount)
}


// ****************************************************************
// constructor
coreModel::md5Triangle::md5Triangle(const char** ppcData)
{
    SCAN_DATA(*ppcData, "%*s %*d %d %d %d",
              &aiVertex[0], &aiVertex[2], &aiVertex[1])
}


// ****************************************************************
// constructor
coreModel::md5Weight::md5Weight(const char** ppcData)
{
    SCAN_DATA(*ppcData, "%*s %*d %d %f %*s %f %f %f %*s",
              &iJoint,      &fBias,
              &vPosition.x, &vPosition.y, &vPosition.z)
}


// ****************************************************************
// constructor
coreModel::md5Mesh::md5Mesh(const char** ppcData)
{
    int iNum = 0;

    SCAN_DATA(*ppcData, "%*s %*s %*s %d", &iNum)
    aVertex.reserve(iNum);
    for(int i = 0; i < iNum; ++i) aVertex.push_back(md5Vertex(ppcData));

    SCAN_DATA(*ppcData, "%*s %d", &iNum)
    aTriangle.reserve(iNum);
    for(int i = 0; i < iNum; ++i) aTriangle.push_back(md5Triangle(ppcData));

    SCAN_DATA(*ppcData, "%*s %d", &iNum)
    aWeight.reserve(iNum);
    for(int i = 0; i < iNum; ++i) aWeight.push_back(md5Weight(ppcData));
}

coreModel::md5Mesh::md5Mesh(md5Mesh&& m)
: aVertex   (std::move(m.aVertex))
, aTriangle (std::move(m.aTriangle))
, aWeight   (std::move(m.aWeight))
{
}


// ****************************************************************
// constructor
coreModel::md5File::md5File(const char** ppcData)
{
    char acIdentifier[16] = "";
    int iVersion          = 0;
    int iNumJoints        = 0;
    int iNumMeshes        = 0;

    // check for correct file type
    SCAN_DATA(*ppcData, "")
    SCAN_DATA(*ppcData, "%15s %d", &acIdentifier, &iVersion)
    if(strcmp(acIdentifier, "MD5Version") || iVersion != 10) return;

    // read number of objects
    SCAN_DATA(*ppcData, "%*s %*s")
    SCAN_DATA(*ppcData, "%*s %d", &iNumJoints)
    SCAN_DATA(*ppcData, "%*s %d", &iNumMeshes)
    aJoint.reserve(iNumJoints);
    aMesh.reserve(iNumMeshes);

    // read joint and mesh data
    for(int i = 0; i < iNumMeshes+1; ++i)
    {
        SCAN_DATA(*ppcData, "%*s")
        SCAN_DATA(*ppcData, "%*s")

        if(i) aMesh.push_back(std::move(md5Mesh(ppcData)));
        else for(int i = 0; i < iNumJoints; ++i) aJoint.push_back(md5Joint(ppcData));

        SCAN_DATA(*ppcData, "%*s")
    }
}

coreModel::md5File::md5File(md5File&& m)
: aJoint (std::move(m.aJoint))
, aMesh  (std::move(m.aMesh))
{
}


// ****************************************************************
// constructor
coreModel::coreModel()
: m_iVertexBuffer  (0)
, m_iNormalBuffer  (0)
, m_iTextureBuffer (0)
, m_iTangentBuffer (0)
, m_iIndexBuffer   (0)
, m_iNumVertices   (0)
, m_iNumTriangles  (0)
, m_iNumIndices    (0)
, m_fRadius        (0.0f)
{
}

coreModel::coreModel(const char* pcPath)
: m_iVertexBuffer  (0)
, m_iNormalBuffer  (0)
, m_iTextureBuffer (0)
, m_iTangentBuffer (0)
, m_iIndexBuffer   (0)
, m_iNumVertices   (0)
, m_iNumTriangles  (0)
, m_iNumIndices    (0)
, m_fRadius        (0.0f)
{
    // load from path
    this->coreResource::Load(pcPath);
}

coreModel::coreModel(coreFile* pFile)
: m_iVertexBuffer  (0)
, m_iNormalBuffer  (0)
, m_iTextureBuffer (0)
, m_iTangentBuffer (0)
, m_iIndexBuffer   (0)
, m_iNumVertices   (0)
, m_iNumTriangles  (0)
, m_iNumIndices    (0)
, m_fRadius        (0.0f)
{
    // load from file
    this->Load(pFile);
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
    SDL_assert(!m_iVertexBuffer);

    if(m_iVertexBuffer)   return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_FILE_ERROR;

    // extract model data
    const char* pcData = reinterpret_cast<const char*>(pFile->GetData());
    const md5File oFile(&pcData);

    // check for success
    if(oFile.aMesh.empty())
    {
        Core::Log->Error(0, coreUtils::Print("Model (%s) is not a valid MD5-file", pFile->GetPath()));
        return CORE_FILE_ERROR;
    }

    const md5Mesh& oMesh = oFile.aMesh[0];

    // save model attributes
    m_iNumVertices  = oMesh.aVertex.size();
    m_iNumTriangles = oMesh.aTriangle.size();
    m_iNumIndices   = oMesh.aTriangle.size()*3;
    m_sPath         = pFile->GetPath();
    m_iSize         = m_iNumVertices*12*sizeof(float) + m_iNumIndices*sizeof(coreUint);

    // reserve required memory
    coreVector3* pvVertex  = new coreVector3[m_iNumVertices];   memset(pvVertex,  0, m_iNumVertices*sizeof(pvVertex[0]));
    coreVector3* pvNormal  = new coreVector3[m_iNumVertices];   memset(pvNormal,  0, m_iNumVertices*sizeof(pvNormal[0]));
    coreVector2* pvTexture = new coreVector2[m_iNumVertices];   memset(pvTexture, 0, m_iNumVertices*sizeof(pvTexture[0]));
    coreVector4* pvTangent = new coreVector4[m_iNumVertices];   memset(pvTangent, 0, m_iNumVertices*sizeof(pvTangent[0]));
    coreVector3* pvOrtho1  = new coreVector3[m_iNumVertices*2]; memset(pvOrtho1,  0, m_iNumVertices*sizeof(pvOrtho1[0])*2);
    coreVector3* pvOrtho2  = pvOrtho1 + m_iNumVertices;

    // traverse all vertices
    for(coreUint i = 0; i < m_iNumVertices; ++i)
    {
        const md5Vertex& oVertex = oMesh.aVertex[i];

        // calculate vertex position
        for(coreUint j = 0; j < oVertex.iWeightCount; ++j)
        {
            const md5Weight& oWeight = oMesh.aWeight[j+oVertex.iWeightStart];
            const md5Joint&  oJoint  = oFile.aJoint[oWeight.iJoint];
            pvVertex[i] += (oJoint.vPosition + oJoint.vOrientation.QuatApply(oWeight.vPosition)) * oWeight.fBias;
        }

        // forward texture coordinate
        pvTexture[i] = oVertex.vTexture;

        // find maximum distance from the model center
        m_fRadius = MAX(pvVertex[i].LengthSq(), m_fRadius);
    }
    m_fRadius = coreMath::Sqrt(m_fRadius);

    SDL_AtomicLock(&s_iLock); // split into two blocks to improve asynchronous performance
    {
        // create vertex position buffer
        glGenBuffers(1, &m_iVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_iNumVertices*sizeof(pvVertex[0]), pvVertex, GL_STATIC_DRAW);

        // create texture coordinate buffer
        glGenBuffers(1, &m_iTextureBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_iTextureBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_iNumVertices*sizeof(pvTexture[0]), pvTexture, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    SDL_AtomicUnlock(&s_iLock);

    // traverse all triangles
    for(coreUint i = 0; i < m_iNumTriangles; ++i)
    {
        const md5Triangle& oTriangle = oMesh.aTriangle[i];

        // calculate triangle sides
        const coreVector3 A1 = pvVertex [oTriangle.aiVertex[1]] - pvVertex [oTriangle.aiVertex[0]];
        const coreVector3 A2 = pvVertex [oTriangle.aiVertex[2]] - pvVertex [oTriangle.aiVertex[0]];
        const coreVector2 B1 = pvTexture[oTriangle.aiVertex[1]] - pvTexture[oTriangle.aiVertex[0]];
        const coreVector2 B2 = pvTexture[oTriangle.aiVertex[2]] - pvTexture[oTriangle.aiVertex[0]];

        // calculate local normal vector
        const coreVector3 N = coreVector3::Cross(A1, A2);

        // calculate local tangent vector parameters
        const float R = 1.0f / (B1.s*B2.t - B2.s*B1.t);
        const coreVector3 D1 = (A1*B2.t - A2*B1.t) * R;
        const coreVector3 D2 = (A2*B1.s - A1*B2.s) * R;

        for(coreUint j = 0; j < 3; ++j)
        {
            // add local values to each point of the triangle
            pvNormal[oTriangle.aiVertex[j]] += N;
            pvOrtho1[oTriangle.aiVertex[j]] += D1;
            pvOrtho2[oTriangle.aiVertex[j]] += D2;
        }
    }
    for(coreUint i = 0; i < m_iNumVertices; ++i)
    {
        // normalize the normal vector
        pvNormal[i].Normalize();

        // finish the Gram–Schmidt process to calculate the tangent vector and binormal sign (w)
        pvTangent[i] = coreVector4((pvOrtho1[i] - pvNormal[i] * coreVector3::Dot(pvNormal[i], pvOrtho1[i])).Normalize(),
                                   SIG(coreVector3::Dot(coreVector3::Cross(pvNormal[i], pvOrtho1[i]), pvOrtho2[i])));
    }

    SDL_AtomicLock(&s_iLock);
    {
        // create normal vector buffer
        glGenBuffers(1, &m_iNormalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_iNormalBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_iNumVertices*sizeof(pvNormal[0]), pvNormal, GL_STATIC_DRAW);

        // create tangent vector buffer
        glGenBuffers(1, &m_iTangentBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_iTangentBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_iNumVertices*sizeof(pvTangent[0]), pvTangent, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // create index buffer
        glGenBuffers(1, &m_iIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_iNumIndices*sizeof(float), oMesh.aTriangle.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    SDL_AtomicUnlock(&s_iLock);

    // free required memory
    SAFE_DELETE_ARRAY(pvVertex)
    SAFE_DELETE_ARRAY(pvNormal)
    SAFE_DELETE_ARRAY(pvTexture)
    SAFE_DELETE_ARRAY(pvTangent)
    SAFE_DELETE_ARRAY(pvOrtho1)

    Core::Log->Info(coreUtils::Print("Model (%s) loaded", m_sPath.c_str()));
    return CORE_OK;
}


// ****************************************************************
// unload model resource data
coreError coreModel::Unload()
{
    if(!m_iVertexBuffer) return CORE_INVALID_CALL;

    // delete all data buffers
    glDeleteBuffers(1, &m_iVertexBuffer);
    glDeleteBuffers(1, &m_iNormalBuffer);
    glDeleteBuffers(1, &m_iTextureBuffer);
    glDeleteBuffers(1, &m_iTangentBuffer);
    glDeleteBuffers(1, &m_iIndexBuffer);
    Core::Log->Info(coreUtils::Print("Model (%s) unloaded", m_sPath.c_str()));

    // reset attributes
    m_sPath          = "";
    m_iSize          = 0;
    m_iVertexBuffer  = 0;
    m_iNormalBuffer  = 0;
    m_iTextureBuffer = 0;
    m_iTangentBuffer = 0;
    m_iIndexBuffer   = 0;
    m_iNumVertices   = 0;
    m_iNumTriangles  = 0;
    m_iNumIndices    = 0;
    m_fRadius        = 0.0f;

    return CORE_OK;
}


// ****************************************************************
// draw the model
void coreModel::Render()
{
    SDL_assert(m_iVertexBuffer);

    SDL_AtomicLock(&s_iLock);
    {
        // set all data buffers
        // \todo define proper tangent attribute binding
        glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);  glVertexPointer(3, GL_FLOAT, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_iNormalBuffer);  glNormalPointer(GL_FLOAT, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_iTextureBuffer); glTexCoordPointer(2, GL_FLOAT, 0, 0);
        //glBindBuffer(GL_ARRAY_BUFFER, m_iTangentBuffer); glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

        // set index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iIndexBuffer);

        // draw the model and reset all buffers
        glDrawElements(GL_TRIANGLES, m_iNumIndices, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    SDL_AtomicUnlock(&s_iLock);
}


// ****************************************************************
// draw only vertex buffer of the model
void coreModel::RenderSimple()
{
    SDL_assert(m_iVertexBuffer);

    SDL_AtomicLock(&s_iLock);
    {
        // set only vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
        glVertexPointer(3, GL_FLOAT, 0, 0);

        // set index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iIndexBuffer);

        // draw the model and reset all buffers
        glDrawElements(GL_TRIANGLES, m_iNumIndices, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    SDL_AtomicUnlock(&s_iLock);
}