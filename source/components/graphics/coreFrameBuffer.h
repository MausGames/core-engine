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

// TODO: mipmapping ? (explicit function, how to reserve levels ? -> new create-enum-value, don't forget to move nearest-filter)
// TODO: check for multiple viewports and uber-shaders
// TODO: a frame buffer with a STENCIL_INDEX8 stencil attachment and a DEPTH_COMPONENT24 depth attachment may be treated as unsupported (FRAMEBUFFER_UNSUPPORTED)
// TODO: fully implement multiple color attachment handling (esp. in blit, clear, invalidate)
// TODO: handle multiple color attachments for ES2 with extension and ES3 in general
// TODO: handle depth-blitting to default frame buffer without blit-function (impossible without over-engineering?)
// TODO: save states and reset on fallback blitting
// TODO: check if rendering quad is faster than blitting (only on default frame buffer? what about depth-stencil? still needs blit for MSAA)

// NOTE: superior objects have to handle resource-resets, to handle resolution-depending properties
// NOTE: notated as integer


// ****************************************************************
// frame buffer definitions
#define __CORE_FRAMEBUFFER_ALL_TARGETS(a)                                                    \
    coreRenderTarget* a[CORE_SHADER_OUTPUT_COLORS + 2] = {&m_DepthTarget, &m_StencilTarget}; \
    for(coreByte i = 2; i < ARRAY_SIZE(a); ++i) a[i] = &m_aColorTarget[i - 2];

enum coreFrameBufferCreate : bool
{
    CORE_FRAMEBUFFER_CREATE_NORMAL       = false,   //!< create normal frame buffer
    CORE_FRAMEBUFFER_CREATE_MULTISAMPLED = true     //!< create multisampled frame buffer
};

enum coreFrameBufferTarget : coreUshort
{
    CORE_FRAMEBUFFER_TARGET_COLOR   = GL_COLOR_BUFFER_BIT,    //!< use color buffer
    CORE_FRAMEBUFFER_TARGET_DEPTH   = GL_DEPTH_BUFFER_BIT,    //!< use depth buffer
    CORE_FRAMEBUFFER_TARGET_STENCIL = GL_STENCIL_BUFFER_BIT   //!< use stencil buffer
};
EXTEND_ENUM(coreFrameBufferTarget)


// ****************************************************************
// frame buffer class
class coreFrameBuffer final
{
private:
    //! render target structure
    struct coreRenderTarget
    {
        coreTexturePtr pTexture;    //!< render target texture (readable)
        GLuint iBuffer;             //!< render target buffer (fast, multisampled)
        GLenum iInternal;           //!< internal memory format (e.g. GL_RGBA8)
        GLenum iFormat;             //!< pixel data format (e.g. GL_RGBA)
        GLenum iType;               //!< pixel data type (e.g. GL_UNSIGNED_BYTE)

        constexpr_func coreRenderTarget()noexcept;
        inline bool IsTexture()const {return pTexture ? true : false;}
        inline bool IsBuffer ()const {return !this->IsTexture();}
    };


private:
    GLuint m_iFrameBuffer;                                        //!< frame buffer identifier

    coreRenderTarget m_aColorTarget[CORE_SHADER_OUTPUT_COLORS];   //!< attached color targets
    coreRenderTarget m_DepthTarget;                               //!< attached depth target
    coreRenderTarget m_StencilTarget;                             //!< attached stencil target

    coreVector2 m_vResolution;                                    //!< resolution of the frame buffer
                                                                   
    float m_fFOV;                                                 //!< field-of-view
    float m_fNearClip;                                            //!< near clipping plane
    float m_fFarClip;                                             //!< far clipping plane

    static coreFrameBuffer* s_pCurrent;                           //!< currently active frame buffer object

    static coreObject2D* s_pBlitFallback;                         //!< 2d-object used for fallback-blitting onto the default frame buffer
    static float s_afViewData[5];                                 //!< view properties of the default frame buffer


public:
    coreFrameBuffer()noexcept;
    ~coreFrameBuffer();
    friend class coreObjectManager;

    //! control the frame buffer
    //! @{
    void Create(const coreVector2& vResolution, const coreFrameBufferCreate& bType);
    void Delete();
    //! @}

    //! attach render targets
    //! @{
    coreRenderTarget* AttachTargetTexture(const coreFrameBufferTarget& iTarget, const coreByte& iColorIndex, const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType, const char* pcName = NULL);
    coreRenderTarget* AttachTargetBuffer (const coreFrameBufferTarget& iTarget, const coreByte& iColorIndex, const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType);
    void DetachTargets();
    //! @}

    //! enable rendering to the frame buffer
    //! @{
    void StartDraw();
    static void EndDraw();
    //! @}

    //! copy content to another frame buffer
    //! @{
    void Blit(const coreFrameBufferTarget& iTargets, coreFrameBuffer* pDestination, const coreUint& iSrcX, const coreUint& iSrcY, const coreUint& iDstX, const coreUint& iDstY, const coreUint& iWidth, const coreUint& iHeight)const;
    void Blit(const coreFrameBufferTarget& iTargets, coreFrameBuffer* pDestination)const;
    //! @}

    //! reset content of the frame buffer
    //! @{
    void Clear     (const coreFrameBufferTarget& iTargets);
    void Invalidate(const coreFrameBufferTarget& iTargets);
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
    inline const GLuint&           GetFrameBuffer  ()const                            {return m_iFrameBuffer;}
    inline const coreRenderTarget& GetColorTarget  (const coreByte& iColorIndex)const {ASSERT(iColorIndex < CORE_SHADER_OUTPUT_COLORS) return m_aColorTarget[iColorIndex];}
    inline const coreRenderTarget& GetDepthTarget  ()const                            {return m_DepthTarget;}
    inline const coreRenderTarget& GetStencilTarget()const                            {return m_StencilTarget;}
    inline const coreVector2&      GetResolution   ()const                            {return m_vResolution;}
    inline const float&            GetFOV          ()const                            {return m_fFOV;}
    inline const float&            GetNearClip     ()const                            {return m_fNearClip;}
    inline const float&            GetFarClip      ()const                            {return m_fFarClip;}
    //! @}


private:
    DISABLE_COPY(coreFrameBuffer)

    //! attach render targets
    //! @{
    coreRenderTarget* __AttachTarget(const coreFrameBufferTarget& iTarget, const coreByte& iColorIndex, const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType);
    //! @}
};


// ****************************************************************
// constructor
constexpr_func coreFrameBuffer::coreRenderTarget::coreRenderTarget()noexcept
: iBuffer   (0)
, iInternal (0)
, iFormat   (0)
, iType     (0)
{
}


#endif // _CORE_GUARD_FRAMEBUFFER_H_