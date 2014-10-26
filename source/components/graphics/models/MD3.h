//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MD3_H_
#define _CORE_GUARD_MD3_H_


// ****************************************************************
/* MD3-header structure */
struct md3Header
{ 
    char acIdentity[4];    //!< magic number (IDP3)
    int  iVersion;         //!< version number (15)

    char acName[64];       //!< internal file name
    int  iFlags;           //!< 'dunno

    int iNumFrames;        //!< number of frames
    int iNumTags;          //!< number of tags
    int iNumSurfaces;      //!< number of surfaces
    int iNumSkins;         //!< number of skins (unused)

    int iOffsetFrames;     //!< byte offset to frame data
    int iOffsetTags;       //!< byte offset to tag data
    int iOffsetSurfaces;   //!< byte offset to surface data
    int iOffsetEOF;        //!< end of file (file size)
};


// ****************************************************************
/* MD3-frame structure */
struct md3Frame
{
    coreVector3 vMinBounds;   //!< first corner of the bounding box
    coreVector3 vMaxBounds;   //!< second corner of the bounding box
    coreVector3 vOrigin;      //!< local origin (0.0f, 0.0f, 0.0f)
    float fRadius;            //!< bounding sphere radius

    char acName[16];          //!< frame name
};


// ****************************************************************
/* MD3-tag structure */
struct md3Tag
{
    char acName[64];       //!< tag name

    coreVector3 vOrigin;   //!< origin coordinates
    coreMatrix3 mAxis;     //!< associated rotation matrix
};


// ****************************************************************
/* MD3-mesh structure */
struct md3Mesh
{
    char acIdentity[4];     //!< magic number (IDP3)

    char acName[64];        //!< surface name
    int  iFlags;            //!< 'dunno again

    int iNumFrames;         //!< number of frames
    int iNumShaders;        //!< number of shaders
    int iNumVertices;       //!< number of vertices
    int iNumTriangles;      //!< number of triangles

    int iOffsetTriangles;   //!< byte offset to triangle data
    int iOffsetShaders;     //!< byte offset to shader data
    int iOffsetTexture;     //!< byte offset to texture data
    int iOffsetVertices;    //!< byte offset to vertex data
    int iOffsetEnd;         //!< surface end (next surface)
};


// ****************************************************************
/* MD3-shader structure */
struct md3Shader
{
    char acName[64];     //!< shader name
    int  iShaderIndex;   //!< shader index number
};


// ****************************************************************
/* MD3-triangle structure */
struct md3Triangle
{
    coreUint aiIndex[3];   //!< triangle indices
};


// ****************************************************************
/* MD3-texture structure */
struct md3Texture
{
    coreVector2 vCoord;   //!< texture coordinates
};


// ****************************************************************
/* MD3-vertex structure */
struct md3Vertex
{
    short    asCoord[3];    //!< compressed vertex coordinates (1:64)
    coreByte aiNormal[2];   //!< compressed normal zenith and azimuth
};


// ****************************************************************
/* MD3-surface structure */
struct md3Surface
{
    md3Mesh      oMesh;       //!< mesh object (surface description)
    md3Shader*   pShader;     //!< pointer to shader data
    md3Triangle* pTriangle;   //!< pointer to triangle data
    md3Texture*  pTexture;    //!< pointer to texture data
    md3Vertex*   pVertex;     //!< pointer to vertex data
};


// ****************************************************************
/* MD3-file structure */
struct md3File
{
    md3Header   oHeader;    //!< header object (file description)
    md3Frame*   pFrame;     //!< pointer to frame data
    md3Tag*     pTag;       //!< pointer to tag data
    md3Surface* pSurface;   //!< pointer to surface data
};


// ****************************************************************
/* import MD3 model file (simplified) */
inline coreError coreImportMD3(const coreByte* pData, coreModel::coreImport* pOutput)
{
    WARN_IF(!pData || !pOutput) return CORE_INVALID_INPUT;

    // reset output object
    pOutput->aVertexData.clear();
    pOutput->aiIndexData.clear();

    // read header data
    md3File oFile;
    std::memcpy(&oFile.oHeader, pData, sizeof(md3Header));

    // check for correct file type
    if(std::strncmp(oFile.oHeader.acIdentity, "IDP3", 4) || oFile.oHeader.iVersion != 15)
        return CORE_INVALID_DATA;

    // allocate frame and surface memory
    oFile.pFrame   = new md3Frame  [oFile.oHeader.iNumFrames];
    oFile.pSurface = new md3Surface[oFile.oHeader.iNumSurfaces];

    // read frame data (not used)
    std::memcpy(oFile.pFrame, pData + oFile.oHeader.iOffsetFrames, sizeof(md3Frame) * oFile.oHeader.iNumFrames);

    // read surface data
    const coreByte* pCursor = pData + oFile.oHeader.iOffsetSurfaces;
    for(int i = 0; i < oFile.oHeader.iNumSurfaces; ++i)
    {
        md3Surface& oSurface = oFile.pSurface[i];
        md3Mesh&    oMesh    = oSurface.oMesh;

        // read mesh data
        std::memcpy(&oMesh, pCursor, sizeof(md3Mesh));

        // allocate vertex memory
        oSurface.pTriangle = new md3Triangle[oMesh.iNumTriangles];
        oSurface.pTexture  = new md3Texture [oMesh.iNumVertices];
        oSurface.pVertex   = new md3Vertex  [oMesh.iNumVertices];

        // read vertex data
        std::memcpy(oSurface.pTriangle, pCursor + oMesh.iOffsetTriangles, sizeof(md3Triangle) * oMesh.iNumTriangles);
        std::memcpy(oSurface.pTexture,  pCursor + oMesh.iOffsetTexture,   sizeof(md3Texture)  * oMesh.iNumVertices);
        std::memcpy(oSurface.pVertex,   pCursor + oMesh.iOffsetVertices,  sizeof(md3Vertex)   * oMesh.iNumVertices);

        pCursor += oMesh.iOffsetEnd; 
    }

    // use data from the first surface
    const md3Surface& oSurface = oFile.pSurface[0];

    // cache size values
    const int& iNumVertices  = oSurface.oMesh.iNumVertices;
    const int  iNumIndices   = oSurface.oMesh.iNumTriangles*3;
    const int& iNumTriangles = oSurface.oMesh.iNumTriangles;

    // allocate required vertex memory
    pOutput->aVertexData.resize(iNumVertices);
    coreModel::coreVertex* pVertex = pOutput->aVertexData.data();
    coreVector3* pvOrtho1 = new coreVector3[iNumVertices];
    coreVector3* pvOrtho2 = new coreVector3[iNumVertices];

    // loop through all vertices
    for(int i = 0; i < iNumVertices; ++i)
    {
        // calculate vertex position
        pVertex[i].vPosition = coreVector3(I_TO_F(oSurface.pVertex[i].asCoord[0]),
                                           I_TO_F(oSurface.pVertex[i].asCoord[1]),
                                           I_TO_F(oSurface.pVertex[i].asCoord[2])) * 0.015625f;

        // calculate vertex normal
        const float fLat   = I_TO_F(oSurface.pVertex[i].aiNormal[1]) * (PI / 128.0f);
        const float fLng   = I_TO_F(oSurface.pVertex[i].aiNormal[0]) * (PI / 128.0f);
        const float fSin   = SIN(fLng);
        pVertex[i].vNormal = coreVector3(COS(fLat) * fSin, SIN(fLat) * fSin, COS(fLng)).Normalize();

        // forward texture coordinate
        pVertex[i].vTexCoord = oSurface.pTexture[i].vCoord;
    }

    // loop through all triangles
    for(int i = 0; i < iNumTriangles; ++i)
    {
        const md3Triangle& oTriangle = oSurface.pTriangle[i];

        // calculate triangle sides
        const coreVector3 A1 = pVertex[oTriangle.aiIndex[1]].vPosition - pVertex[oTriangle.aiIndex[0]].vPosition;
        const coreVector3 A2 = pVertex[oTriangle.aiIndex[2]].vPosition - pVertex[oTriangle.aiIndex[0]].vPosition;
        const coreVector2 B1 = pVertex[oTriangle.aiIndex[1]].vTexCoord - pVertex[oTriangle.aiIndex[0]].vTexCoord;
        const coreVector2 B2 = pVertex[oTriangle.aiIndex[2]].vTexCoord - pVertex[oTriangle.aiIndex[0]].vTexCoord;

        // calculate local tangent vector parameters
        const float R = RCP(B1.s*B2.t - B2.s*B1.t);
        const coreVector3 D1 = (A1*B2.t - A2*B1.t) * R;
        const coreVector3 D2 = (A2*B1.s - A1*B2.s) * R;

        for(int j = 0; j < 3; ++j)
        {
            // add local values to each point of the triangle
            pvOrtho1[oTriangle.aiIndex[j]] += D1;
            pvOrtho2[oTriangle.aiIndex[j]] += D2;
        }
    }
    for(int i = 0; i < iNumVertices; ++i)
    {
        // finish the Gram-Schmidt process to calculate the tangent vector and binormal sign (w)
        pVertex[i].vTangent = coreVector4((pvOrtho1[i] - pVertex[i].vNormal * coreVector3::Dot(pVertex[i].vNormal, pvOrtho1[i])).Normalize(),
                                          SIGN(coreVector3::Dot(coreVector3::Cross(pVertex[i].vNormal, pvOrtho1[i]), pvOrtho2[i])));
    }

    // copy index data counter-clockwise
    pOutput->aiIndexData.resize(iNumIndices);
    for(int i = 0; i < iNumTriangles; ++i)
    {
        for(int j = 0; j < 3; ++j)
            pOutput->aiIndexData[i*3+j] = (coreUshort)oFile.pSurface[0].pTriangle[i].aiIndex[2-j];
    }

    // free required vertex memory
    SAFE_DELETE_ARRAY(pvOrtho1)
    SAFE_DELETE_ARRAY(pvOrtho2)

    // free model file memory
    for(int i = 0; i < oFile.oHeader.iNumSurfaces; i++)
    {
        SAFE_DELETE_ARRAY(oFile.pSurface[i].pTriangle)
        SAFE_DELETE_ARRAY(oFile.pSurface[i].pTexture)
        SAFE_DELETE_ARRAY(oFile.pSurface[i].pVertex)
    }
    SAFE_DELETE_ARRAY(oFile.pFrame)
    SAFE_DELETE_ARRAY(oFile.pSurface)

    return CORE_OK;
}


#endif /* _CORE_GUARD_MD3_H_ */