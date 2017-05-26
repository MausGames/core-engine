//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MODEL_H_
#define _CORE_GUARD_MODEL_H_

// TODO: support more than one mesh per file
// TODO: add animation
// TODO: enable shared vertex buffers mixed with unique
// TODO: check out (Multi)Draw(Array|Elements)Indirect
// TODO: Nullify is in main-thread because of VAOs, check for other dependencies and try to fix this
// TODO: compress texture-coords, 32bit is too much, but still need coords <0.0 >1.0 (probably to -1.0 2.0)
// TODO: index buffer ignore as constructor parameter ? (resource manager load)
// TODO: currently radius and range can change, but they should not... implement more strict interface
// TODO: de-interleaved vertex data


// ****************************************************************
// model definitions
#define CORE_MODEL_CLUSTERS_AXIS (5u)                               //!< number of clusters per axis
#define CORE_MODEL_CLUSTERS_MAX  (POW3(CORE_MODEL_CLUSTERS_AXIS))   //!< maximum number of clusters per model


// ****************************************************************
// model class
class coreModel final : public coreResource
{
public:
    //! vertex structure
    struct coreVertex final
    {
        coreVector3 vPosition;   //!< vertex position
        coreVector2 vTexCoord;   //!< texture coordinate
        coreVector3 vNormal;     //!< normal vector
        coreVector4 vTangent;    //!< additional tangent vector
    };

    //! compressed vertex structure
    #pragma pack(push, 4)
    struct coreVertexPackedHigh final
    {
        coreUint64 iPosition;    //!< vertex position           (Float4x16)
        coreUint32 iTexCoord;    //!< texture coordinate        (Unorm2x16)
        coreUint32 iNormal;      //!< normal vector             (Snorm210)
        coreUint32 iTangent;     //!< additional tangent vector (Snorm210)
    };
    struct coreVertexPackedLow final
    {
        coreVector3 vPosition;   //!< vertex position
        coreUint32  iTexCoord;   //!< texture coordinate        (Unorm2x16)
        coreUint32  iNormal;     //!< normal vector             (Snorm4x8)
        coreUint32  iTangent;    //!< additional tangent vector (Snorm4x8)
    };
    #pragma pack(pop)

    //! import structure
    struct coreImport final
    {
        std::vector<coreVertex> aVertexData;   //!< raw vertex data
        std::vector<coreUint16> aiIndexData;   //!< raw index data
    };


private:
    GLuint m_iVertexArray;                           //!< vertex array object

    std::vector<coreVertexBuffer> m_aVertexBuffer;   //!< vertex buffers
    coreDataBuffer                m_IndexBuffer;     //!< index buffer

    coreUint32 m_iNumVertices;                       //!< number of vertices
    coreUint32 m_iNumIndices;                        //!< number of indices
    coreUint8  m_iNumClusters;                       //!< number of clusters

    coreVector3 m_vBoundingRange;                    //!< maximum per-axis distance from the model center
    coreFloat   m_fBoundingRadius;                   //!< maximum direct distance from the model center

    coreVector3* m_pvVertexPosition;                 //!< vertex positions for clusters and precise collision detection

    coreUint16** m_ppiClusterIndex;                  //!< pointer to indices per cluster (single allocation)
    coreUint16*  m_piClusterNumIndices;              //!< number of indices per cluster
    coreVector3* m_pvClusterPosition;                //!< relative cluster center
    coreFloat*   m_pfClusterRadius;                  //!< maximum direct distance from the cluster center

    GLenum m_iPrimitiveType;                         //!< primitive type for draw calls (e.g. GL_TRIANGLES)
    GLenum m_iIndexType;                             //!< index type for draw calls (e.g. GL_UNSIGNED_SHORT)

    coreSync m_Sync;                                 //!< sync object for asynchronous model loading

    static coreModel* s_pCurrent;                    //!< currently active model object


public:
    explicit coreModel(const coreBool bCreateClusters = true)noexcept;
    ~coreModel()final;

    DISABLE_COPY(coreModel)

    //! load and unload model resource data
    //! @{
    coreStatus Load(coreFile* pFile)final;
    coreStatus Unload()final;
    //! @}

    //! draw the model
    //! @{
    inline void Draw        ()const {if(m_IndexBuffer) this->DrawElements(); else this->DrawArrays();}
    void        DrawArrays  ()const;
    void        DrawElements()const;
    //! @}

    //! draw the model instanced
    //! @{
    inline void DrawInstanced        (const coreUint32 iCount)const {if(m_IndexBuffer) this->DrawElementsInstanced(iCount); else this->DrawArraysInstanced(iCount);}
    void        DrawArraysInstanced  (const coreUint32 iCount)const;
    void        DrawElementsInstanced(const coreUint32 iCount)const;
    //! @}

    //! enable and disable the model
    //! @{
    void Enable();
    static void Disable(const coreBool bFull);
    //! @}

    //! generate custom model resource data
    //! @{
    coreVertexBuffer*        CreateVertexBuffer(const coreUint32 iNumVertices, const coreUint8 iVertexSize, const void* pVertexData, const coreDataBufferStorage iStorageType);
    coreDataBuffer*          CreateIndexBuffer (const coreUint32 iNumIndices,  const coreUint8 iIndexSize,  const void* pIndexData,  const coreDataBufferStorage iStorageType);
    inline coreVertexBuffer* GetVertexBuffer   (const coreUintW iIndex) {return &m_aVertexBuffer[iIndex];}
    inline coreDataBuffer*   GetIndexBuffer    ()                       {return &m_IndexBuffer;}
    //! @}

    //! set object properties
    //! @{
    inline void SetBoundingRange (const coreVector3& vBoundingRange)  {ASSERT( m_vBoundingRange.IsNull()) m_vBoundingRange  = vBoundingRange;}
    inline void SetBoundingRadius(const coreFloat    fBoundingRadius) {ASSERT(!m_fBoundingRadius)         m_fBoundingRadius = fBoundingRadius;}
    inline void SetPrimitiveType (const GLenum       iPrimitiveType)  {m_iPrimitiveType = iPrimitiveType;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&      GetVertexArray      ()const                       {return m_iVertexArray;}
    inline const coreUint32&  GetNumVertices      ()const                       {return m_iNumVertices;}
    inline const coreUint32&  GetNumIndices       ()const                       {return m_iNumIndices;}
    inline const coreUint8&   GetNumClusters      ()const                       {return m_iNumClusters;}
    inline const coreVector3& GetBoundingRange    ()const                       {return m_vBoundingRange;}
    inline const coreFloat&   GetBoundingRadius   ()const                       {return m_fBoundingRadius;}
    inline const coreVector3* GetVertexPosition   ()const                       {return m_pvVertexPosition;}
    inline const coreUint16*  GetClusterIndex     (const coreUintW iIndex)const {ASSERT(iIndex < m_iNumClusters) return m_ppiClusterIndex    [iIndex];}
    inline const coreUint16&  GetClusterNumIndices(const coreUintW iIndex)const {ASSERT(iIndex < m_iNumClusters) return m_piClusterNumIndices[iIndex];}
    inline const coreVector3& GetClusterPosition  (const coreUintW iIndex)const {ASSERT(iIndex < m_iNumClusters) return m_pvClusterPosition  [iIndex];}
    inline const coreFloat&   GetClusterRadius    (const coreUintW iIndex)const {ASSERT(iIndex < m_iNumClusters) return m_pfClusterRadius    [iIndex];}
    inline const GLenum&      GetPrimitiveType    ()const                       {return m_iPrimitiveType;}
    inline const GLenum&      GetIndexType        ()const                       {return m_iIndexType;}
    //! @}

    //! get currently active model object
    //! @{
    static inline coreModel* GetCurrent() {return s_pCurrent;}
    //! @}
};


// ****************************************************************
// model resource access type
using coreModelPtr = coreResourcePtr<coreModel>;


#endif // _CORE_GUARD_MODEL_H_