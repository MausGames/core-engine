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
    SCAN_DATA(*ppcData, "%*s %*d %hu %hu %hu",
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
: m_iVertexArray  (0)
, m_iVertexBuffer (0)
, m_iIndexBuffer  (0)
, m_iNumVertices  (0)
, m_iNumTriangles (0)
, m_iNumIndices   (0)
, m_fRadius       (0.0f)
, m_pSync         (NULL)
{
}

coreModel::coreModel(const char* pcPath)
: m_iVertexArray  (0)
, m_iVertexBuffer (0)
, m_iIndexBuffer  (0)
, m_iNumVertices  (0)
, m_iNumTriangles (0)
, m_iNumIndices   (0)
, m_fRadius       (0.0f)
, m_pSync         (NULL)
{
    // load from path
    this->coreResource::Load(pcPath);
}

coreModel::coreModel(coreFile* pFile)
: m_iVertexArray  (0)
, m_iVertexBuffer (0)
, m_iIndexBuffer  (0)
, m_iNumVertices  (0)
, m_iNumTriangles (0)
, m_iNumIndices   (0)
, m_fRadius       (0.0f)
, m_pSync         (NULL)
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
    // check sync object status
    const coreError iStatus = this->CheckSync();
    if(iStatus != CORE_INVALID_CALL) return iStatus;

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
        return CORE_INVALID_DATA;
    }

    const bool& bVertexArray = Core::Graphics->SupportFeature("GL_ARB_vertex_array_object");
    const bool& bSync        = Core::Graphics->SupportFeature("GL_ARB_sync");
    const md5Mesh& oMesh     = oFile.aMesh[0];

    // save model attributes
    m_iNumVertices  = oMesh.aVertex.size();
    m_iNumTriangles = oMesh.aTriangle.size();
    m_iNumIndices   = oMesh.aTriangle.size()*3;
    m_sPath         = pFile->GetPath();
    m_iSize         = m_iNumVertices*sizeof(coreVertex) + m_iNumIndices*sizeof(coreWord);

    // reserve required vertex memory
    coreVertex*  pVertex  = new coreVertex [m_iNumVertices]; memset(pVertex,  0, m_iNumVertices*sizeof(pVertex[0]));
    coreVector3* pvOrtho1 = new coreVector3[m_iNumVertices]; memset(pvOrtho1, 0, m_iNumVertices*sizeof(pvOrtho1[0]));
    coreVector3* pvOrtho2 = new coreVector3[m_iNumVertices]; memset(pvOrtho2, 0, m_iNumVertices*sizeof(pvOrtho2[0]));

    // traverse all vertices
    for(coreUint i = 0; i < m_iNumVertices; ++i)
    {
        const md5Vertex& oVertex = oMesh.aVertex[i];

        // calculate vertex position
        for(int j = 0; j < oVertex.iWeightCount; ++j)
        {
            const md5Weight& oWeight = oMesh.aWeight[j+oVertex.iWeightStart];
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
        const coreVector3 N = coreVector3::Cross(A1, A2);

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

        // finish the Gram–Schmidt process to calculate the tangent vector and binormal sign (w)
        pVertex[i].vTangent = coreVector4((pvOrtho1[i] - pVertex[i].vNormal * coreVector3::Dot(pVertex[i].vNormal, pvOrtho1[i])).Normalize(),
                                          SIG(coreVector3::Dot(coreVector3::Cross(pVertex[i].vNormal, pvOrtho1[i]), pvOrtho2[i])));
    }

    SDL_AtomicLock(&s_iLock);
    {
        // create vertex array object
        if(bVertexArray)
        {
            glGenVertexArrays(1, &m_iVertexArray);
            glBindVertexArray(m_iVertexArray);
        }

        // create vertex buffer
        glGenBuffers(1, &m_iVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_iNumVertices*sizeof(coreVertex), pVertex, GL_STATIC_DRAW);

        // create index buffer
        glGenBuffers(1, &m_iIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_iNumIndices*sizeof(coreWord), oMesh.aTriangle.data(), GL_STATIC_DRAW);

        // define vertex attribute data
        if(bVertexArray)
        {
            this->__BindVertexAttributes();
            glBindVertexArray(0);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    SDL_AtomicUnlock(&s_iLock);

    // generate sync object or flush all commands
    if(bSync) m_pSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    else glFlush();

    // free required vertex memory
    SAFE_DELETE_ARRAY(pVertex)
    SAFE_DELETE_ARRAY(pvOrtho1)
    SAFE_DELETE_ARRAY(pvOrtho2)

    if(m_pSync) return CORE_BUSY;
    Core::Log->Info(coreUtils::Print("Model (%s) loaded", m_sPath.c_str()));
    return CORE_OK;
}


// ****************************************************************
// unload model resource data
coreError coreModel::Unload()
{
    if(!m_iVertexBuffer) return CORE_INVALID_CALL;

    // delete vertex array object and data buffers
    if(m_iVertexArray) glDeleteVertexArrays(1, &m_iVertexArray);
    glDeleteBuffers(1, &m_iVertexBuffer);
    glDeleteBuffers(1, &m_iIndexBuffer);
    Core::Log->Info(coreUtils::Print("Model (%s) unloaded", m_sPath.c_str()));

    // delete sync object
    if(m_pSync)
    {
        glDeleteSync(m_pSync);
        m_pSync = NULL;
    }

    // reset attributes
    m_sPath         = "";
    m_iSize         = 0;
    m_iVertexArray  = 0;
    m_iVertexBuffer = 0;
    m_iIndexBuffer  = 0;
    m_iNumVertices  = 0;
    m_iNumTriangles = 0;
    m_iNumIndices   = 0;
    m_fRadius       = 0.0f;

    return CORE_OK;
}


// ****************************************************************
// draw the model
void coreModel::Render()
{
    SDL_assert(m_iVertexBuffer);

    SDL_AtomicLock(&s_iLock);
    {
        // set vertex data
        if(m_iVertexArray) glBindVertexArray(m_iVertexArray);
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBuffer);
            this->__BindVertexAttributes();
        }

        // set index buffer and draw the model
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iIndexBuffer);
        glDrawElements(GL_TRIANGLES, m_iNumIndices, GL_UNSIGNED_SHORT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // reset vertex data
        if(m_iVertexArray) glBindVertexArray(0);
        else glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    SDL_AtomicUnlock(&s_iLock);
}


// ****************************************************************
// check sync object status
// TODO: reuse code with coreTexture
coreError coreModel::CheckSync()
{
    if(!m_iVertexBuffer || !m_pSync) return CORE_INVALID_CALL;

    // check for finished model loading
    if(glClientWaitSync(m_pSync, 0, 0) != GL_TIMEOUT_EXPIRED)
    {
        // delete sync object
        glDeleteSync(m_pSync);
        m_pSync = NULL;

        Core::Log->Info(coreUtils::Print("Model (%s) loaded asynchronous", m_sPath.c_str()));
        return CORE_OK;
    }

    return CORE_BUSY;
}


// ****************************************************************
// define vertex attribute data
void coreModel::__BindVertexAttributes()
{
    // enable vertex attributes
    glEnableVertexAttribArray(CORE_SHADER_IN_POSITION_NUM);
    glEnableVertexAttribArray(CORE_SHADER_IN_TEXTURE_NUM);
    glEnableVertexAttribArray(CORE_SHADER_IN_NORMAL_NUM);
    glEnableVertexAttribArray(CORE_SHADER_IN_TANGENT_NUM);

    // set vertex attribute data locations
    glVertexAttribPointer(CORE_SHADER_IN_POSITION_NUM, 3, GL_FLOAT, false, sizeof(coreVertex), 0);
    glVertexAttribPointer(CORE_SHADER_IN_TEXTURE_NUM,  2, GL_FLOAT, false, sizeof(coreVertex), reinterpret_cast<const GLvoid*>(3*sizeof(float)));
    glVertexAttribPointer(CORE_SHADER_IN_NORMAL_NUM,   3, GL_FLOAT, false, sizeof(coreVertex), reinterpret_cast<const GLvoid*>(5*sizeof(float)));
    glVertexAttribPointer(CORE_SHADER_IN_TANGENT_NUM,  4, GL_FLOAT, false, sizeof(coreVertex), reinterpret_cast<const GLvoid*>(8*sizeof(float)));
}