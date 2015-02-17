//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MD5_H_
#define _CORE_GUARD_MD5_H_

// TODO: (collapse vertices (e.g. same pos) and merge nearly identical vertices in this process (with threshold and std::map)) only adapt normals ?


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
/* MD5-joint structure */
struct md5Joint
{
    int iParent;                //!< index of the parent joint (-1 = root joint)
    coreVector3 vPosition;      //!< position of the joint
    coreVector4 vOrientation;   //!< orientation of the joint

    explicit md5Joint(const char** ppcData)noexcept
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
};


// ****************************************************************
/* MD5-vertex structure */
struct md5Vertex
{
    coreVector2 vTexture;   //!< texture coordinates
    int iWeightStart;       //!< index of the first associated weight
    int iWeightCount;       //!< number of associated weights

    explicit md5Vertex(const char** ppcData)noexcept
    {
        MD5_SCAN(*ppcData, "%*s %*d %*s %f %f %*s %d %d",
                 &vTexture.x,   &vTexture.y,
                 &iWeightStart, &iWeightCount)
    }
};


// ****************************************************************
/* MD5-triangle structure */
struct md5Triangle
{
    coreUshort aiVertex[3];   //!< indexes of the defining vertices

    explicit md5Triangle(const char** ppcData)noexcept
    {
        MD5_SCAN(*ppcData, "%*s %*d %hu %hu %hu",
                 &aiVertex[0], &aiVertex[2], &aiVertex[1])
    }
};


// ****************************************************************
/* MD5-weight structure */
struct md5Weight
{
    int   iJoint;            //!< index of the associated joint
    float fBias;             //!< contribution factor
    coreVector3 vPosition;   //!< position of the weight

    explicit md5Weight(const char** ppcData)noexcept
    {
        MD5_SCAN(*ppcData, "%*s %*d %d %f %*s %f %f %f %*s",
                 &iJoint,      &fBias,
                 &vPosition.x, &vPosition.y, &vPosition.z)
    }
};


// ****************************************************************
/* MD5-mesh structure */
struct md5Mesh
{
    std::vector<md5Vertex>   aVertex;     //!< vertex list
    std::vector<md5Triangle> aTriangle;   //!< triangle list
    std::vector<md5Weight>   aWeight;     //!< weight list

    explicit md5Mesh(const char** ppcData)noexcept
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

    md5Mesh(md5Mesh&& m)noexcept
    : aVertex   (std::move(m.aVertex))
    , aTriangle (std::move(m.aTriangle))
    , aWeight   (std::move(m.aWeight))
    {
    }
};


// ****************************************************************
/* MD5-file structure */
struct md5File
{
    std::vector<md5Joint> aJoint;   //!< joint list
    std::vector<md5Mesh>  aMesh;    //!< mesh list

    explicit md5File(const char** ppcData)noexcept
    {
        char acIdentifier[16] = "";
        int  iVersion         = 0;
        int  iNumJoints       = 0;
        int  iNumMeshes       = 0;

        // check for correct file type
        MD5_SCAN(*ppcData, "")
        MD5_SCAN(*ppcData, "%15s %d", r_cast<char*>(&acIdentifier), &iVersion)
        if(std::strncmp(acIdentifier, "MD5Version", 10) || iVersion != 10) return;

        // read number of objects
        MD5_SCAN(*ppcData, "%*s %*s")
        MD5_SCAN(*ppcData, "%*s %d", &iNumJoints)
        MD5_SCAN(*ppcData, "%*s %d", &iNumMeshes)
        aJoint.reserve(iNumJoints);
        aMesh .reserve(iNumMeshes);

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

    md5File(md5File&& m)noexcept
    : aJoint (std::move(m.aJoint))
    , aMesh  (std::move(m.aMesh))
    {
    }
};


// ****************************************************************
/* import MD5 model file (simplified) */
inline coreError coreImportMD5(const coreByte* pData, coreModel::coreImport* OUTPUT pOutput)
{
    WARN_IF(!pData || !pOutput) return CORE_INVALID_INPUT;

    // reset output object
    pOutput->aVertexData.clear();
    pOutput->aiIndexData.clear();

    // extract model data
    const char* pcData = r_cast<const char*>(pData);
    const md5File oFile(&pcData);

    // check for success
    if(oFile.aMesh.empty()) return CORE_INVALID_DATA;

    // use data from the first mesh
    const md5Mesh& oMesh = oFile.aMesh[0];

    // cache size values
    const coreUint iNumVertices  = coreUint(oMesh.aVertex.size());
    const coreUint iNumIndices   = coreUint(oMesh.aTriangle.size())*3;
    const coreUint iNumTriangles = coreUint(oMesh.aTriangle.size());

    // allocate required vertex memory
    pOutput->aVertexData.resize(iNumVertices);
    coreModel::coreVertex* pVertex = pOutput->aVertexData.data();
    coreVector3* pvOrtho1 = new coreVector3[iNumVertices];
    coreVector3* pvOrtho2 = new coreVector3[iNumVertices];

    // loop through all vertices
    for(coreUint i = 0; i < iNumVertices; ++i)
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
        pVertex[i].vTexCoord = oVertex.vTexture;
    }

    // loop through all triangles
    for(coreUint i = 0; i < iNumTriangles; ++i)
    {
        const md5Triangle& oTriangle = oMesh.aTriangle[i];

        // calculate triangle sides
        const coreVector3 A1 = pVertex[oTriangle.aiVertex[1]].vPosition - pVertex[oTriangle.aiVertex[0]].vPosition;
        const coreVector3 A2 = pVertex[oTriangle.aiVertex[2]].vPosition - pVertex[oTriangle.aiVertex[0]].vPosition;
        const coreVector2 B1 = pVertex[oTriangle.aiVertex[1]].vTexCoord - pVertex[oTriangle.aiVertex[0]].vTexCoord;
        const coreVector2 B2 = pVertex[oTriangle.aiVertex[2]].vTexCoord - pVertex[oTriangle.aiVertex[0]].vTexCoord;

        // calculate local normal vector
        const coreVector3 N = coreVector3::Cross(A1.Normalized(), A2.Normalized());

        // calculate local tangent vector parameters
        const float R = RCP(B1.s*B2.t - B2.s*B1.t);
        const coreVector3 D1 = (A1*B2.t - A2*B1.t) * R;
        const coreVector3 D2 = (A2*B1.s - A1*B2.s) * R;

        for(int j = 0; j < 3; ++j)
        {
            // add local values to each point of the triangle
            pVertex [oTriangle.aiVertex[j]].vNormal += N;
            pvOrtho1[oTriangle.aiVertex[j]] += D1;
            pvOrtho2[oTriangle.aiVertex[j]] += D2;
        }
    }
    for(coreUint i = 0; i < iNumVertices; ++i)
    {
        // normalize the normal vector
        pVertex[i].vNormal.Normalize();

        // finish the Gram-Schmidt process to calculate the tangent vector and binormal sign (w)
        pVertex[i].vTangent = coreVector4((pvOrtho1[i] - pVertex[i].vNormal * coreVector3::Dot(pVertex[i].vNormal, pvOrtho1[i])).Normalize(),
                                          SIGN(coreVector3::Dot(coreVector3::Cross(pVertex[i].vNormal, pvOrtho1[i]), pvOrtho2[i])));
    }

    // copy index data
    pOutput->aiIndexData.resize(iNumIndices);
    std::memcpy(pOutput->aiIndexData.data(), oMesh.aTriangle.data(), sizeof(coreUshort)*iNumIndices);

    // free required vertex memory
    SAFE_DELETE_ARRAY(pvOrtho1)
    SAFE_DELETE_ARRAY(pvOrtho2)

    return CORE_OK;
}


#endif /* _CORE_GUARD_MD5_H_ */