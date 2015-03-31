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
// TODO: reorder indices/vertices in memory to improve post-transform caching (maybe in model-file, nvTriStrip)
// TODO: Nullify is in main-thread because of VAOs, check for other dependencies and try to fix this
// TODO: compress texture-coords, 32bit is too much, but still need coords <0.0 >1.0 (probably to -1.0 2.0)
// TODO: index buffer ignore as constructor parameter ? (resource manager load)
// TODO: currently radius and range can change, but they should not... implement more strict interface


// ****************************************************************
// model class
class coreModel final : public coreResource
{
public:
    //! vertex structure
    struct coreVertex
    {
        coreVector3 vPosition;   //!< vertex position
        coreVector2 vTexCoord;   //!< texture coordinate
        coreVector3 vNormal;     //!< normal vector
        coreVector4 vTangent;    //!< additional tangent vector

        constexpr_func coreVertex()noexcept;
    };

    //! compressed vertex structure
    struct coreVertexPacked
    {
        coreVector3 vPosition;   //!< vertex position
        coreUint32  iTexCoord;   //!< texture coordinate
        coreUint32  iNormal;     //!< normal vector
        coreUint32  iTangent;    //!< additional tangent vector

        constexpr_func coreVertexPacked()noexcept;
    };

    //! import structure
    struct coreImport
    {
        std::vector<coreVertex> aVertexData;   //!< raw vertex data
        std::vector<coreUint16> aiIndexData;   //!< raw index data

        coreImport()noexcept {}
    };


private:
    GLuint m_iVertexArray;                              //!< vertex array object

    std::vector<coreVertexBuffer*> m_apiVertexBuffer;   //!< vertex buffers
    coreDataBuffer                 m_iIndexBuffer;      //!< index buffer

    coreUint32  m_iNumVertices;                         //!< number of vertices
    coreUint32  m_iNumIndices;                          //!< number of indices
    coreVector3 m_vBoundingRange;                       //!< maximum per-axis distance from the model center
    coreFloat   m_fBoundingRadius;                      //!< maximum direct distance from the model center

    GLenum m_iPrimitiveType;                            //!< primitive type for draw calls (e.g. GL_TRIANGLES)
    GLenum m_iIndexType;                                //!< index type for draw calls (e.g. GL_UNSIGNED_SHORT)

    coreSync m_Sync;                                    //!< sync object for asynchronous model loading

    static coreModel* s_pCurrent;                       //!< currently active model object


public:
    coreModel()noexcept;
    ~coreModel();

    DISABLE_COPY(coreModel)

    //! load and unload model resource data
    //! @{
    coreStatus Load(coreFile* pFile)override;
    coreStatus Unload()override;
    //! @}

    //! draw the model
    //! @{
    inline void Draw        ()const {if(m_iIndexBuffer) this->DrawElements(); else this->DrawArrays();}
    void        DrawArrays  ()const;
    void        DrawElements()const;
    //! @}

    //! draw the model instanced
    //! @{
    inline void DrawInstanced        (const coreUint32& iCount)const {if(m_iIndexBuffer) this->DrawElementsInstanced(iCount); else this->DrawArraysInstanced(iCount);}
    void        DrawArraysInstanced  (const coreUint32& iCount)const;
    void        DrawElementsInstanced(const coreUint32& iCount)const;
    //! @}

    //! enable and disable the model
    //! @{
    void Enable();
    static void Disable(const coreBool& bFull);
    //! @}

    //! generate custom model resource data
    //! @{
    coreVertexBuffer*        CreateVertexBuffer(const coreUint32& iNumVertices, const coreUint8& iVertexSize, const void* pVertexData, const coreDataBufferStorage& iStorageType);
    coreDataBuffer*          CreateIndexBuffer (const coreUint32& iNumIndices,  const coreUint8& iIndexSize,  const void* pIndexData,  const coreDataBufferStorage& iStorageType);
    inline coreVertexBuffer* GetVertexBuffer   (const coreUintW& iIndex) {return m_apiVertexBuffer[iIndex];}
    inline coreDataBuffer*   GetIndexBuffer    ()                        {return &m_iIndexBuffer;}
    //! @}

    //! set object properties
    //! @{
    inline void SetBoundingRange (const coreVector3& vBoundingRange)  {ASSERT( m_vBoundingRange.IsNull()) m_vBoundingRange  = vBoundingRange;}
    inline void SetBoundingRadius(const coreFloat&   fBoundingRadius) {ASSERT(!m_fBoundingRadius)         m_fBoundingRadius = fBoundingRadius;}
    inline void SetPrimitiveType (const GLenum&      iPrimitiveType)  {m_iPrimitiveType = iPrimitiveType;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&      GetVertexArray   ()const {return m_iVertexArray;}
    inline const coreUint32&  GetNumVertices   ()const {return m_iNumVertices;}
    inline const coreUint32&  GetNumIndices    ()const {return m_iNumIndices;}
    inline const coreVector3& GetBoundingRange ()const {return m_vBoundingRange;}
    inline const coreFloat&   GetBoundingRadius()const {return m_fBoundingRadius;}
    inline const GLenum&      GetPrimitiveType ()const {return m_iPrimitiveType;}
    inline const GLenum&      GetIndexType     ()const {return m_iIndexType;}
    //! @}

    //! get currently active model object
    //! @{
    static inline coreModel* GetCurrent() {return s_pCurrent;}
    //! @}
};


// ****************************************************************
// constructor
constexpr_func coreModel::coreVertex::coreVertex()noexcept
: vPosition (coreVector3(0.0f,0.0f,0.0f))
, vTexCoord (coreVector2(0.0f,0.0f))
, vNormal   (coreVector3(0.0f,0.0f,0.0f))
, vTangent  (coreVector4(0.0f,0.0f,0.0f,0.0f))
{
}


// ****************************************************************
// constructor
constexpr_func coreModel::coreVertexPacked::coreVertexPacked()noexcept
: vPosition (coreVector3(0.0f,0.0f,0.0f))
, iTexCoord (0u)
, iNormal   (0u)
, iTangent  (0u)
{
}


// ****************************************************************
// model resource access type
typedef coreResourcePtr<coreModel> coreModelPtr;


#endif // _CORE_GUARD_MODEL_H_