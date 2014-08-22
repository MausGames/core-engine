//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_FRAMEBUFFER_H_
#define _CORE_GUARD_FRAMEBUFFER_H_

// TODO: mipmapping ? (pre-allocation: storage and normal+generation)
// TODO: implement real multisampling (currently supersampling sufficient)
// TODO: frame buffer stack to allow nested buffer wrtings ? (GL 2.0 problems)
// TODO: add stencil support (type-enum already added)
// TODO: multiple color buffers, also check for multiple viewports and mega-shaders


// ****************************************************************
// frame buffer definitions
enum coreFrameBufferType : coreUshort
{
    CORE_FRAMEBUFFER_TYPE_COLOR   = 0x0001,   //!< create color buffer without depth test
    CORE_FRAMEBUFFER_TYPE_DEPTH   = 0x0002,   //!< create depth buffer only
    CORE_FRAMEBUFFER_TYPE_FULL    = 0x0003,   //!< create full frame buffer with color and depth writing
    CORE_FRAMEBUFFER_TYPE_ALPHA   = 0x0100,   //!< add alpha channel (only COLOR and FULL)
    CORE_FRAMEBUFFER_TYPE_STENCIL = 0x0200    //!< add stencil buffer
};
EXTEND_ENUM(coreFrameBufferType)

enum coreFrameBufferUse : coreByte
{
    CORE_FRAMEBUFFER_USE_COLOR   = 0x01,   //!< use the color buffer for this operation
    CORE_FRAMEBUFFER_USE_DEPTH   = 0x02,   //!< use the depth buffer
    CORE_FRAMEBUFFER_USE_STENCIL = 0x04    //!< use the stencil buffer
};
EXTEND_ENUM(coreFrameBufferUse)


// ****************************************************************
// frame buffer class
class coreFrameBuffer final : public coreResourceRelation
{
private:
    GLuint m_iFrameBuffer;                //!< frame buffer identifier

    coreTexturePtr m_pTexture;            //!< render target texture
    GLuint m_iDepthBuffer;                //!< depth component buffer
 
    coreVector2 m_vResolution;            //!< resolution of the frame buffer
    coreFrameBufferType m_iType;          //!< type of the frame buffer

    float m_fFOV;                         //!< field-of-view
    float m_fNearClip;                    //!< near clipping plane
    float m_fFarClip;                     //!< far clipping plane

    static coreFrameBuffer* s_pCurrent;   //!< currently active frame buffer object


public:
    coreFrameBuffer()noexcept;
    ~coreFrameBuffer() {this->Delete();}

    //! control the frame buffer
    //! @{
    void Create(const coreVector2& vResolution, const coreFrameBufferType& iType, const char* pcTextureName);
    void Delete();
    //! @}

    //! enable rendering to the frame buffer
    //! @{
    void StartDraw();
    void EndDraw();
    void EndDrawBound();
    //! @}

    //! copy content to another frame buffer
    //! @{
    void Blit(coreFrameBuffer* pTarget, const coreFrameBufferUse& iBuffer, const coreUint& iSrcX, const coreUint& iSrcY, const coreUint& iDstX, const coreUint& iDstY, const coreUint& iWidth, const coreUint& iHeight);
    void Blit(coreFrameBuffer* pTarget, const coreFrameBufferUse& iBuffer);
    //! @}

    //! reset content of the frame buffer
    //! @{
    void Clear     (const coreFrameBufferUse& iBuffer);
    void Invalidate(const coreFrameBufferUse& iBuffer);
    //! @}

    //! access buffer directly
    //! @{
    inline operator const GLuint& ()const {return m_iFrameBuffer;}
    //! @}

    //! set object properties
    //! @{
    inline void SetFOV     (const float& fFOV)      {m_fFOV      = fFOV;}
    inline void SetNearClip(const float& fNearClip) {m_fNearClip = fNearClip;}
    inline void SetFarClip (const float& fFarClip)  {m_fFarClip  = fFarClip;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&              GetFrameBuffer()const {return m_iFrameBuffer;}
    inline const coreTexturePtr&      GetTexture    ()const {return m_pTexture;}
    inline const GLuint&              GetDepthBuffer()const {return m_iDepthBuffer;}
    inline const coreVector2&         GetResolution ()const {return m_vResolution;}
    inline const coreFrameBufferType& GetType       ()const {return m_iType;}
    inline const float&               GetFOV        ()const {return m_fFOV;}
    inline const float&               GetNearClip   ()const {return m_fNearClip;}
    inline const float&               GetFarClip    ()const {return m_fFarClip;}
    //! @}


private:
    DISABLE_COPY(coreFrameBuffer)

    //! reset with the resource manager
    //! @{
    void __Reset(const coreResourceReset& bInit)override;
    //! @}

    //! delete frame and depth buffer
    //! @{
    void __DeleteBuffers();
    //! @}
};


#endif // _CORE_GUARD_FRAMEBUFFER_H_