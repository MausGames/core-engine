///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MD3_H_
#define _CORE_GUARD_MD3_H_

// TODO 3: add vertex position factor as new header parameter


// ****************************************************************
/* MD3-header structure */
struct md3Header final
{
    coreChar  acIdentity[4];     // magic number (IDP3)
    coreInt32 iVersion;          // version number (15 or 16)

    coreChar  acName[64];        // internal file name
    coreInt32 iFlags;            // 'dunno

    coreInt32 iNumFrames;        // number of frames
    coreInt32 iNumTags;          // number of tags
    coreInt32 iNumSurfaces;      // number of surfaces
    coreInt32 iNumSkins;         // number of skins (unused)

    coreInt32 iOffsetFrames;     // byte offset to frame data
    coreInt32 iOffsetTags;       // byte offset to tag data
    coreInt32 iOffsetSurfaces;   // byte offset to surface data
    coreInt32 iOffsetEOF;        // end of file (file size)
};


// ****************************************************************
/* MD3-frame structure */
struct md3Frame final
{
    coreVector3 vMinBounds;   // first corner of the bounding box
    coreVector3 vMaxBounds;   // second corner of the bounding box
    coreVector3 vOrigin;      // local origin (0.0f, 0.0f, 0.0f)
    coreFloat   fRadius;      // bounding sphere radius

    coreChar acName[16];      // frame name
};


// ****************************************************************
/* MD3-tag structure */
struct md3Tag final
{
    coreChar acName[64];   // tag name

    coreVector3 vOrigin;   // origin coordinates
    coreMatrix3 mAxis;     // associated rotation matrix
};


// ****************************************************************
/* MD3-mesh structure */
struct md3Mesh final
{
    coreChar acIdentity[4];       // magic number (IDP3)

    coreChar  acName[64];         // surface name
    coreInt32 iFlags;             // 'dunno again

    coreInt32 iNumFrames;         // number of frames
    coreInt32 iNumShaders;        // number of shaders
    coreInt32 iNumVertices;       // number of vertices
    coreInt32 iNumTriangles;      // number of triangles

    coreInt32 iOffsetTriangles;   // byte offset to triangle data
    coreInt32 iOffsetShaders;     // byte offset to shader data
    coreInt32 iOffsetTexture;     // byte offset to texture data
    coreInt32 iOffsetVertices;    // byte offset to vertex data
    coreInt32 iOffsetEnd;         // surface end (next surface)
};


// ****************************************************************
/* MD3-shader structure */
struct md3Shader final
{
    coreChar  acName[64];     // shader name
    coreInt32 iShaderIndex;   // shader index number
};


// ****************************************************************
/* MD3-triangle structure */
struct md3Triangle final
{
    coreUint32 aiIndex[3];   // triangle indices
};


// ****************************************************************
/* MD3-texture structure */
struct md3Texture final
{
    coreVector2 vCoord;   // texture coordinates
};


// ****************************************************************
/* MD3-vertex structure */
struct md3Vertex final
{
    coreInt16 asCoord [3];   // compressed vertex coordinates (1:64)
    coreUint8 aiNormal[2];   // compressed normal zenith and azimuth (16 bit)
};

struct md3VertexV16 final
{
    coreInt16  asCoord [3];   // compressed vertex coordinates (1:256)
    coreUint16 aiNormal[2];   // compressed normal zenith and azimuth (32 bit)
};


// ****************************************************************
/* MD3-surface structure */
struct md3Surface final
{
    md3Mesh      oMesh;       // mesh object (surface description)
    md3Shader*   pShader;     // pointer to shader data
    md3Triangle* pTriangle;   // pointer to triangle data
    md3Texture*  pTexture;    // pointer to texture data
    union                     // pointer to vertex data
    {
        md3Vertex*    pVertex;
        md3VertexV16* pVertexV16;
    };
};


// ****************************************************************
/* MD3-file structure */
struct md3File final
{
    md3Header   oHeader;    // header object (file description)
    md3Frame*   pFrame;     // pointer to frame data
    md3Tag*     pTag;       // pointer to tag data
    md3Surface* pSurface;   // pointer to surface data
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
    md3File oFile = {};
    std::memcpy(&oFile.oHeader, pData, sizeof(md3Header));

    // check for correct file type
    if(std::memcmp(oFile.oHeader.acIdentity, "IDP3", 4u) || ((oFile.oHeader.iVersion != 15) && (oFile.oHeader.iVersion != 16)))
        return CORE_INVALID_DATA;

    // determine the file version
    const coreBool bVersion16 = (oFile.oHeader.iVersion == 16);

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
        if(bVersion16) oSurface.pVertexV16 = new md3VertexV16[oMesh.iNumVertices];
                  else oSurface.pVertex    = new md3Vertex   [oMesh.iNumVertices];

        // read vertex data
        std::memcpy(oSurface.pTriangle, pCursor + oMesh.iOffsetTriangles, sizeof(md3Triangle) * oMesh.iNumTriangles);
        std::memcpy(oSurface.pTexture,  pCursor + oMesh.iOffsetTexture,   sizeof(md3Texture)  * oMesh.iNumVertices);
        if(bVersion16) std::memcpy(oSurface.pVertexV16, pCursor + oMesh.iOffsetVertices, sizeof(md3VertexV16) * oMesh.iNumVertices);
                  else std::memcpy(oSurface.pVertex,    pCursor + oMesh.iOffsetVertices, sizeof(md3Vertex)    * oMesh.iNumVertices);

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
    coreVector3* pvOrtho1 = ZERO_NEW(coreVector3, iNumVertices);
    coreVector3* pvOrtho2 = ZERO_NEW(coreVector3, iNumVertices);

    if(bVersion16)
    {
        // loop through all vertices (high quality)
        for(coreUintW i = 0u; i < iNumVertices; ++i)
        {
            // calculate vertex position
            pVertex[i].vPosition = coreVector3(I_TO_F(oSurface.pVertexV16[i].asCoord[0]),
                                               I_TO_F(oSurface.pVertexV16[i].asCoord[1]),
                                               I_TO_F(oSurface.pVertexV16[i].asCoord[2])) * (1.0f/256.0f);

            // calculate vertex normal
            const coreFloat fLat = I_TO_F(oSurface.pVertexV16[i].aiNormal[1]) * (PI / 32768.0f);
            const coreFloat fLng = I_TO_F(oSurface.pVertexV16[i].aiNormal[0]) * (PI / 32768.0f);
            const coreFloat fSin = SIN(fLng);
            pVertex[i].vNormal = coreVector3(COS(fLat) * fSin, SIN(fLat) * fSin, COS(fLng)).NormalizedUnsafe();

            // forward texture coordinate
            pVertex[i].vTexCoord = oSurface.pTexture[i].vCoord;
        }
    }
    else
    {
        // loop through all vertices (low quality)
        for(coreUintW i = 0u; i < iNumVertices; ++i)
        {
            // calculate vertex position
            pVertex[i].vPosition = coreVector3(I_TO_F(oSurface.pVertex[i].asCoord[0]),
                                               I_TO_F(oSurface.pVertex[i].asCoord[1]),
                                               I_TO_F(oSurface.pVertex[i].asCoord[2])) * (1.0f/64.0f);

            // calculate vertex normal
            const coreFloat fLat = I_TO_F(oSurface.pVertex[i].aiNormal[1]) * (PI / 128.0f);
            const coreFloat fLng = I_TO_F(oSurface.pVertex[i].aiNormal[0]) * (PI / 128.0f);
            const coreFloat fSin = SIN(fLng);
            pVertex[i].vNormal = coreVector3(COS(fLat) * fSin, SIN(fLat) * fSin, COS(fLng)).NormalizedUnsafe();

            // forward texture coordinate
            pVertex[i].vTexCoord = oSurface.pTexture[i].vCoord;
        }
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
        const coreFloat   C  = B1.x*B2.y - B2.x*B1.y;
        const coreFloat   R  = RCP(C ? C : 1.0f);
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
        if(pvOrtho1[i].IsNull()) pvOrtho1[i] = coreVector3(0.0f,0.0f,1.0);
        
        // finish the Gram-Schmidt process to calculate the tangent vector and bitangent sign (w)
        pVertex[i].vTangent = coreVector4((pvOrtho1[i] - pVertex[i].vNormal * coreVector3::Dot(pVertex[i].vNormal, pvOrtho1[i])).NormalizedUnsafe(),
                                          SIGN(coreVector3::Dot(coreVector3::Cross(pVertex[i].vNormal, pvOrtho1[i]), pvOrtho2[i])));
    }

    // copy index data counter-clockwise
    pOutput->aiIndexData.resize(iNumIndices);
    for(coreUintW i = 0u; i < iNumTriangles; ++i)
    {
        for(coreUintW j = 0u; j < 3u; ++j)
        {
            pOutput->aiIndexData[i*3u+j] = coreUint16(oSurface.pTriangle[i].aiIndex[2u-j]);
        }
    }

    // free required vertex memory
    ZERO_DELETE(pvOrtho1)
    ZERO_DELETE(pvOrtho2)

    // free model file memory
    for(coreUintW i = 0u, ie = oFile.oHeader.iNumSurfaces; i < ie; ++i)
    {
        SAFE_DELETE_ARRAY(oFile.pSurface[i].pTriangle)
        SAFE_DELETE_ARRAY(oFile.pSurface[i].pTexture)
        if(bVersion16) SAFE_DELETE_ARRAY(oFile.pSurface[i].pVertexV16)
                  else SAFE_DELETE_ARRAY(oFile.pSurface[i].pVertex)
    }
    SAFE_DELETE_ARRAY(oFile.pFrame)
    SAFE_DELETE_ARRAY(oFile.pSurface)

    return CORE_OK;
}


#endif /* _CORE_GUARD_MD3_H_ */