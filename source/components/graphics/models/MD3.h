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
    coreChar  acIdentity[4];     //!< magic number (IDP3)
    coreInt32 iVersion;          //!< version number (15)

    coreChar  acName[64];        //!< internal file name
    coreInt32 iFlags;            //!< 'dunno

    coreInt32 iNumFrames;        //!< number of frames
    coreInt32 iNumTags;          //!< number of tags
    coreInt32 iNumSurfaces;      //!< number of surfaces
    coreInt32 iNumSkins;         //!< number of skins (unused)

    coreInt32 iOffsetFrames;     //!< byte offset to frame data
    coreInt32 iOffsetTags;       //!< byte offset to tag data
    coreInt32 iOffsetSurfaces;   //!< byte offset to surface data
    coreInt32 iOffsetEOF;        //!< end of file (file size)
};


// ****************************************************************
/* MD3-frame structure */
struct md3Frame
{
    coreVector3 vMinBounds;   //!< first corner of the bounding box
    coreVector3 vMaxBounds;   //!< second corner of the bounding box
    coreVector3 vOrigin;      //!< local origin (0.0f, 0.0f, 0.0f)
    coreFloat   fRadius;      //!< bounding sphere radius

    coreChar acName[16];      //!< frame name
};


// ****************************************************************
/* MD3-tag structure */
struct md3Tag
{
    coreChar acName[64];   //!< tag name

    coreVector3 vOrigin;   //!< origin coordinates
    coreMatrix3 mAxis;     //!< associated rotation matrix
};


// ****************************************************************
/* MD3-mesh structure */
struct md3Mesh
{
    coreChar acIdentity[4];        //!< magic number (IDP3)

    coreChar  acName[64];         //!< surface name
    coreInt32 iFlags;             //!< 'dunno again

    coreInt32 iNumFrames;         //!< number of frames
    coreInt32 iNumShaders;        //!< number of shaders
    coreInt32 iNumVertices;       //!< number of vertices
    coreInt32 iNumTriangles;      //!< number of triangles

    coreInt32 iOffsetTriangles;   //!< byte offset to triangle data
    coreInt32 iOffsetShaders;     //!< byte offset to shader data
    coreInt32 iOffsetTexture;     //!< byte offset to texture data
    coreInt32 iOffsetVertices;    //!< byte offset to vertex data
    coreInt32 iOffsetEnd;         //!< surface end (next surface)
};


// ****************************************************************
/* MD3-shader structure */
struct md3Shader
{
    coreChar  acName[64];     //!< shader name
    coreInt32 iShaderIndex;   //!< shader index number
};


// ****************************************************************
/* MD3-triangle structure */
struct md3Triangle
{
    coreUint32 aiIndex[3];   //!< triangle indices
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
    coreInt16 asCoord [3];   //!< compressed vertex coordinates (1:64)
    coreUint8 aiNormal[2];   //!< compressed normal zenith and azimuth
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
inline coreStatus coreImportMD3(const coreByte* pData, coreModel::coreImport* OUTPUT pOutput)
{
    WARN_IF(!pData || !pOutput) return CORE_INVALID_INPUT;

    // reset output object
    pOutput->aVertexData.clear();
    pOutput->aiIndexData.clear();

    // read header data
    md3File oFile;
    std::memcpy(&oFile.oHeader, pData, sizeof(md3Header));

    // check for correct file type
    if(std::strncmp(oFile.oHeader.acIdentity, "IDP3", 4u) || oFile.oHeader.iVersion != 15)
        return CORE_INVALID_DATA;

    // allocate frame and surface memory
    oFile.pFrame   = new md3Frame  [oFile.oHeader.iNumFrames];
    oFile.pSurface = new md3Surface[oFile.oHeader.iNumSurfaces];

    // read frame data (not used)
    std::memcpy(oFile.pFrame, pData + oFile.oHeader.iOffsetFrames, sizeof(md3Frame) * oFile.oHeader.iNumFrames);

    // read surface data
    const coreByte* pCursor = pData + oFile.oHeader.iOffsetSurfaces;
    for(coreUintW i = 0u, ie = oFile.oHeader.iNumSurfaces; i < ie; ++i)
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
    const coreUintW iNumVertices  = oSurface.oMesh.iNumVertices;
    const coreUintW iNumIndices   = oSurface.oMesh.iNumTriangles * 3u;
    const coreUintW iNumTriangles = oSurface.oMesh.iNumTriangles;
    ASSERT(iNumVertices <= 0xFFFFu)

    // allocate required vertex memory
    pOutput->aVertexData.resize(iNumVertices);
    coreModel::coreVertex* pVertex = pOutput->aVertexData.data();
    coreVector3* pvOrtho1 = new coreVector3[iNumVertices];
    coreVector3* pvOrtho2 = new coreVector3[iNumVertices];

    // loop through all vertices
    for(coreUintW i = 0u; i < iNumVertices; ++i)
    {
        // calculate vertex position
        pVertex[i].vPosition = coreVector3(I_TO_F(oSurface.pVertex[i].asCoord[0]),
                                           I_TO_F(oSurface.pVertex[i].asCoord[1]),
                                           I_TO_F(oSurface.pVertex[i].asCoord[2])) * 0.015625f;

        // calculate vertex normal
        const coreFloat fLat = I_TO_F(oSurface.pVertex[i].aiNormal[1]) * (PI / 128.0f);
        const coreFloat fLng = I_TO_F(oSurface.pVertex[i].aiNormal[0]) * (PI / 128.0f);
        const coreFloat fSin = SIN(fLng);
        pVertex[i].vNormal = coreVector3(COS(fLat) * fSin, SIN(fLat) * fSin, COS(fLng)).Normalize();

        // forward texture coordinate
        pVertex[i].vTexCoord = oSurface.pTexture[i].vCoord;
    }

    // loop through all triangles
    for(coreUintW i = 0u; i < iNumTriangles; ++i)
    {
        const md3Triangle& oTriangle = oSurface.pTriangle[i];

        // calculate triangle sides
        const coreVector3 A1 = pVertex[oTriangle.aiIndex[1]].vPosition - pVertex[oTriangle.aiIndex[0]].vPosition;
        const coreVector3 A2 = pVertex[oTriangle.aiIndex[2]].vPosition - pVertex[oTriangle.aiIndex[0]].vPosition;
        const coreVector2 B1 = pVertex[oTriangle.aiIndex[1]].vTexCoord - pVertex[oTriangle.aiIndex[0]].vTexCoord;
        const coreVector2 B2 = pVertex[oTriangle.aiIndex[2]].vTexCoord - pVertex[oTriangle.aiIndex[0]].vTexCoord;

        // calculate local tangent vector parameters
        const coreFloat   R  = RCP(B1.x*B2.y - B2.x*B1.y);
        const coreVector3 D1 = (A1*B2.y - A2*B1.y) * R;
        const coreVector3 D2 = (A2*B1.x - A1*B2.x) * R;

        for(coreUintW j = 0u; j < 3u; ++j)
        {
            // add local values to each point of the triangle
            pvOrtho1[oTriangle.aiIndex[j]] += D1;
            pvOrtho2[oTriangle.aiIndex[j]] += D2;
        }
    }
    for(coreUintW i = 0u; i < iNumVertices; ++i)
    {
        // finish the Gram-Schmidt process to calculate the tangent vector and bitangent sign (w)
        pVertex[i].vTangent = coreVector4((pvOrtho1[i] - pVertex[i].vNormal * coreVector3::Dot(pVertex[i].vNormal, pvOrtho1[i])).Normalize(),
                                          SIGN(coreVector3::Dot(coreVector3::Cross(pVertex[i].vNormal, pvOrtho1[i]), pvOrtho2[i])));
    }

    // copy index data counter-clockwise
    pOutput->aiIndexData.resize(iNumIndices);
    for(coreUintW i = 0u; i < iNumTriangles; ++i)
    {
        for(coreUintW j = 0u; j < 3u; ++j)
        {
            pOutput->aiIndexData[i*3u+j] = coreUint16(oFile.pSurface[0].pTriangle[i].aiIndex[2u-j]);
        }
    }

    // free required vertex memory
    SAFE_DELETE_ARRAY(pvOrtho1)
    SAFE_DELETE_ARRAY(pvOrtho2)

    // free model file memory
    for(coreUintW i = 0u, ie = oFile.oHeader.iNumSurfaces; i < ie; ++i)
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