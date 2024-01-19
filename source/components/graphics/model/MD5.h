///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MD5_H_
#define _CORE_GUARD_MD5_H_

// TODO 5: adjust normals where multiple vertices are at the same position (with threshold and lookup-table)


// ****************************************************************
/* move string pointer and skip comments */
static void coreSkipComments(const coreChar** ppcInput)
{
    ASSERT(*ppcInput)

    // check for line-comments
    while((**ppcInput) == '/')
    {
        // skip them
        coreInt32 n = 0;
        std::sscanf((*ppcInput), "%*[^\n] %n", &n);
        (*ppcInput) += n;
    }
}
#define MD5_SCAN(s,f,...) {coreInt32 __n = 0; std::sscanf((*s), f " %n", ##__VA_ARGS__, &__n); (*s) += __n; coreSkipComments(s);}


// ****************************************************************
/* MD5-joint structure */
struct md5Joint final
{
    coreInt32   iParent;        // index of the parent joint (-1 = root joint)
    coreVector3 vPosition;      // position of the joint
    coreVector4 vOrientation;   // orientation of the joint

    explicit md5Joint(const coreChar** ppcData)noexcept
    {
        MD5_SCAN(ppcData, "%*s %d %*s %f %f %f %*s %*s %f %f %f %*s",
                 &iParent,
                 &vPosition.x,    &vPosition.y,    &vPosition.z,
                 &vOrientation.x, &vOrientation.y, &vOrientation.z)

        // calculate w-component of the normalized quaternion
        const coreVector3 V = vOrientation.xyz() * vOrientation.xyz();
        const coreFloat   T = 1.0f - V.x - V.y - V.z;
        vOrientation.w = (T < 0.0f) ? 0.0f : -SQRT(T);
    }
};


// ****************************************************************
/* MD5-vertex structure */
struct md5Vertex final
{
    coreVector2 vTexture;       // texture coordinates
    coreUint16  iWeightStart;   // index of the first associated weight
    coreUint16  iWeightCount;   // number of associated weights

    explicit md5Vertex(const coreChar** ppcData)noexcept
    {
        MD5_SCAN(ppcData, "%*s %*d %*s %f %f %*s %hu %hu",
                 &vTexture.x,   &vTexture.y,
                 &iWeightStart, &iWeightCount)
    }
};


// ****************************************************************
/* MD5-triangle structure */
struct md5Triangle final
{
    coreUint16 aiVertex[3];   // indexes of the defining vertices

    explicit md5Triangle(const coreChar** ppcData)noexcept
    {
        MD5_SCAN(ppcData, "%*s %*d %hu %hu %hu",
                 &aiVertex[0], &aiVertex[2], &aiVertex[1])
    }
};


// ****************************************************************
/* MD5-weight structure */
struct md5Weight final
{
    coreInt32   iJoint;      // index of the associated joint
    coreFloat   fBias;       // contribution factor
    coreVector3 vPosition;   // position of the weight

    explicit md5Weight(const coreChar** ppcData)noexcept
    {
        MD5_SCAN(ppcData, "%*s %*d %d %f %*s %f %f %f %*s",
                 &iJoint,      &fBias,
                 &vPosition.x, &vPosition.y, &vPosition.z)
    }
};


// ****************************************************************
/* MD5-mesh structure */
struct md5Mesh final
{
    coreList<md5Vertex>   aVertex;     // vertex list
    coreList<md5Triangle> aTriangle;   // triangle list
    coreList<md5Weight>   aWeight;     // weight list

    explicit md5Mesh(const coreChar** ppcData)noexcept
    {
        coreUintW iNum = 0u;

        MD5_SCAN(ppcData, "%*s %*s %*s %zu", &iNum)
        aVertex.reserve(iNum);
        for(coreUintW i = iNum; i--; ) aVertex.emplace_back(ppcData);

        MD5_SCAN(ppcData, "%*s %zu", &iNum)
        aTriangle.reserve(iNum);
        for(coreUintW i = iNum; i--; ) aTriangle.emplace_back(ppcData);

        MD5_SCAN(ppcData, "%*s %zu", &iNum)
        aWeight.reserve(iNum);
        for(coreUintW i = iNum; i--; ) aWeight.emplace_back(ppcData);
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
struct md5File final
{
    coreList<md5Joint> aJoint;   // joint list
    coreList<md5Mesh>  aMesh;    // mesh list

    explicit md5File(const coreChar** ppcData)noexcept
    {
        coreChar  acIdentifier[16] = {};
        coreInt32 iVersion         = 0;
        coreUintW iNumJoints       = 0u;
        coreUintW iNumMeshes       = 0u;

        // check for correct file type
        MD5_SCAN(ppcData, "")
        MD5_SCAN(ppcData, "%15s %d", r_cast<coreChar*>(&acIdentifier), &iVersion)
        if(std::memcmp(acIdentifier, "MD5Version", 10u) || (iVersion != 10)) return;

        // read number of objects
        MD5_SCAN(ppcData, "%*s %*s")
        MD5_SCAN(ppcData, "%*s %zu", &iNumJoints)
        MD5_SCAN(ppcData, "%*s %zu", &iNumMeshes)
        aJoint.reserve(iNumJoints);
        aMesh .reserve(iNumMeshes);

        // read joint and mesh data
        for(coreUintW i = 0u, ie = (iNumMeshes + 1u); i < ie; ++i)
        {
            MD5_SCAN(ppcData, "%*s")
            MD5_SCAN(ppcData, "%*s")

            if(i) aMesh.emplace_back(ppcData);
            else for(coreUintW j = iNumJoints; j--; ) aJoint.emplace_back(ppcData);

            if(i != iNumMeshes) MD5_SCAN(ppcData, "%*s")   // # ignore last symbol, because there is no 0-delimiter
        }
    }

    md5File(md5File&& m)noexcept
    : aJoint (std::move(m.aJoint))
    , aMesh  (std::move(m.aMesh))
    {
    }
};


// ****************************************************************
/* import MD5 model file (simplified) (# only use precise calculations) */
inline coreStatus coreImportMD5(const coreByte* pData, coreModel::coreImport* OUTPUT pOutput)
{
    WARN_IF(!pData || !pOutput) return CORE_INVALID_INPUT;

    // reset output object
    pOutput->aVertexData.clear();
    pOutput->aiIndexData.clear();

    // extract model data
    const coreChar* pcData = r_cast<const coreChar*>(pData);
    const md5File oFile(&pcData);

    // check for success
    if(oFile.aMesh.empty()) return CORE_INVALID_DATA;

    // use data from the first mesh
    const md5Mesh& oMesh = oFile.aMesh[0];

    // cache size values
    const coreUintW iNumVertices  = oMesh.aVertex  .size();
    const coreUintW iNumIndices   = oMesh.aTriangle.size() * 3u;
    const coreUintW iNumTriangles = oMesh.aTriangle.size();
    ASSERT((iNumVertices > 0u) && (iNumVertices <= 0xFFFFu) && iNumIndices && iNumTriangles)

    // allocate required vertex memory
    pOutput->aVertexData.resize(iNumVertices);
    coreModel::coreVertex* pVertex = pOutput->aVertexData.data();
    coreVector3* pvOrtho1 = ZERO_NEW(coreVector3, iNumVertices);
    coreVector3* pvOrtho2 = ZERO_NEW(coreVector3, iNumVertices);

    // loop through all vertices
    for(coreUintW i = 0u; i < iNumVertices; ++i)
    {
        const md5Vertex& oVertex = oMesh.aVertex[i];

        // calculate vertex position
        for(coreUintW j = 0u, je = oVertex.iWeightCount; j < je; ++j)
        {
            const md5Weight& oWeight = oMesh.aWeight[j + oVertex.iWeightStart];
            const md5Joint&  oJoint  = oFile.aJoint[oWeight.iJoint];
            pVertex[i].vPosition += (oJoint.vPosition + oJoint.vOrientation.QuatApply(oWeight.vPosition)) * oWeight.fBias;
        }

        // forward texture coordinate
        pVertex[i].vTexCoord = oVertex.vTexture;
    }

    // loop through all triangles
    for(coreUintW i = 0u; i < iNumTriangles; ++i)
    {
        const md5Triangle& oTriangle = oMesh.aTriangle[i];

        // calculate triangle sides
        const coreVector3 A1 = pVertex[oTriangle.aiVertex[1]].vPosition - pVertex[oTriangle.aiVertex[0]].vPosition;
        const coreVector3 A2 = pVertex[oTriangle.aiVertex[2]].vPosition - pVertex[oTriangle.aiVertex[0]].vPosition;
        const coreVector2 B1 = pVertex[oTriangle.aiVertex[1]].vTexCoord - pVertex[oTriangle.aiVertex[0]].vTexCoord;
        const coreVector2 B2 = pVertex[oTriangle.aiVertex[2]].vTexCoord - pVertex[oTriangle.aiVertex[0]].vTexCoord;

        // calculate local normal vector (Nelson Max algorithm)
        const coreVector3 N = coreVector3::Cross(A1, A2) / (A1.LengthSq() + A2.LengthSq());

        // calculate local tangent vector parameters
        const coreFloat   C  = B1.x*B2.y - B2.x*B1.y;
        const coreFloat   R  = 1.0f / (C ? C : 1.0f);
        const coreVector3 D1 = (A1*B2.y - A2*B1.y) * R;
        const coreVector3 D2 = (A2*B1.x - A1*B2.x) * R;

        for(coreUintW j = 0u; j < 3u; ++j)
        {
            // add local values to each point of the triangle
            pVertex [oTriangle.aiVertex[j]].vNormal += N;
            pvOrtho1[oTriangle.aiVertex[j]] += D1;
            pvOrtho2[oTriangle.aiVertex[j]] += D2;
        }
    }
    for(coreUintW i = 0u; i < iNumVertices; ++i)
    {
        // normalize the normal vector
        pVertex[i].vNormal = pVertex[i].vNormal.NormalizedUnsafePrecise();

        // handle degenerate triangles
        if(pvOrtho1[i].IsNull()) pvOrtho1[i] = coreVector3(0.0f,0.0f,1.0f);

        // finish the Gram-Schmidt process to calculate the tangent vector and bitangent sign (w)
        pVertex[i].vTangent = coreVector4((pvOrtho1[i] - pVertex[i].vNormal * coreVector3::Dot(pVertex[i].vNormal, pvOrtho1[i])).NormalizedUnsafePrecise(),
                                          SIGN(coreVector3::Dot(coreVector3::Cross(pVertex[i].vNormal, pvOrtho1[i]), pvOrtho2[i])));
    }

    // copy index data
    pOutput->aiIndexData.resize(iNumIndices);
    std::memcpy(pOutput->aiIndexData.data(), oMesh.aTriangle.data(), sizeof(coreUint16) * iNumIndices);

    // free required vertex memory
    ZERO_DELETE(pvOrtho1)
    ZERO_DELETE(pvOrtho2)

    return CORE_OK;
}


#endif /* _CORE_GUARD_MD5_H_ */