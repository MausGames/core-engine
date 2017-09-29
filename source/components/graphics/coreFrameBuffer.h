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
// TODO: implement native depth_stencil support and check for GL_EXT_packed_depth_stencil (forced on ARB)
// TODO: change R and RG targets to RGB when not supported ?
// TODO: INTEL_framebuffer_CMAA

// NOTE: superior objects have to handle resource-resets, to handle resolution-depending properties


// ****************************************************************
// frame buffer definitions
#define __CORE_FRAMEBUFFER_ALL_TARGETS(a)                                                     \
    coreRenderTarget* a[CORE_SHADER_OUTPUT_COLORS + 2u] = {&m_DepthTarget, &m_StencilTarget}; \
    for(coreUintW i = 2u; i < ARRAY_SIZE(a); ++i) a[i] = &m_aColorTarget[i - 2u];

enum coreFrameBufferCreate : coreBool
{
    CORE_FRAMEBUFFER_CREATE_NORMAL       = false,   //!< create normal frame buffer
    CORE_FRAMEBUFFER_CREATE_MULTISAMPLED = true     //!< create multisampled frame buffer
};

enum coreFrameBufferTarget : coreUint16
{
    CORE_FRAMEBUFFER_TARGET_COLOR   = GL_COLOR_BUFFER_BIT,    //!< use color buffer
    CORE_FRAMEBUFFER_TARGET_DEPTH   = GL_DEPTH_BUFFER_BIT,    //!< use depth buffer
    CORE_FRAMEBUFFER_TARGET_STENCIL = GL_STENCIL_BUFFER_BIT   //!< use stencil buffer
};
ENABLE_BITWISE(coreFrameBufferTarget)


// ****************************************************************
// frame buffer class
class coreFrameBuffer final
{
private:
    //! render target structure
    struct coreRenderTarget final
    {
        coreTexturePtr  pTexture;   //!< render target texture (readable)
        GLuint          iBuffer;    //!< render target buffer (fast, multisampled)
        coreTextureSpec oSpec;      //!< texture and buffer specification (format)

        constexpr coreRenderTarget()noexcept;
        inline coreBool IsTexture()const {return pTexture ? true : false;}
        inline coreBool IsBuffer ()const {return !this->IsTexture();}
    };


private:
    GLuint m_iIdentifier;                                         //!< frame buffer identifier

    coreRenderTarget m_aColorTarget[CORE_SHADER_OUTPUT_COLORS];   //!< attached color targets
    coreRenderTarget m_DepthTarget;                               //!< attached depth target
    coreRenderTarget m_StencilTarget;                             //!< attached stencil target

    coreVector2 m_vResolution;                                    //!< resolution of the frame buffer

    coreFloat m_fFOV;                                             //!< field-of-view
    coreFloat m_fNearClip;                                        //!< near clipping plane
    coreFloat m_fFarClip;                                         //!< far clipping plane

    static coreFrameBuffer* s_pCurrent;                           //!< currently active frame buffer object
    static coreFloat s_afViewData[5];                             //!< view properties of the default frame buffer


public:
    coreFrameBuffer()noexcept;
    ~coreFrameBuffer();

    DISABLE_COPY(coreFrameBuffer)

    //! control the frame buffer
    //! @{
    void Create(const coreVector2& vResolution, const coreFrameBufferCreate bType);
    void Delete();
    //! @}

    //! attach render targets
    //! @{
    coreRenderTarget* AttachTargetTexture(const coreFrameBufferTarget iTarget, const coreUintW iColorIndex, const coreTextureSpec& oSpec, const coreChar* pcName = NULL);
    coreRenderTarget* AttachTargetBuffer (const coreFrameBufferTarget iTarget, const coreUintW iColorIndex, const coreTextureSpec& oSpec);
    void DetachTargets();
    //! @}

    //! enable rendering to the frame buffer
    //! @{
    void StartDraw();
    static void EndDraw();
    //! @}

    //! copy content to another frame buffer
    //! @{
    void Blit(const coreFrameBufferTarget iTargets, coreFrameBuffer* OUTPUT pDestination, const coreUint32 iSrcX, const coreUint32 iSrcY, const coreUint32 iDstX, const coreUint32 iDstY, const coreUint32 iWidth, const coreUint32 iHeight)const;
    void Blit(const coreFrameBufferTarget iTargets, coreFrameBuffer* OUTPUT pDestination)const;
    //! @}

    //! reset content of the frame buffer
    //! @{
    void Clear     (const coreFrameBufferTarget iTargets);
    void Invalidate(const coreFrameBufferTarget iTargets);
    //! @}

    //! set object properties
    //! @{
    inline void SetFOV     (const coreFloat fFOV)      {m_fFOV      = fFOV;}
    inline void SetNearClip(const coreFloat fNearClip) {m_fNearClip = fNearClip;}
    inline void SetFarClip (const coreFloat fFarClip)  {m_fFarClip  = fFarClip;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&           GetIdentifier   ()const                            {return m_iIdentifier;}
    inline const coreRenderTarget& GetColorTarget  (const coreUintW iColorIndex)const {ASSERT(iColorIndex < CORE_SHADER_OUTPUT_COLORS) return m_aColorTarget[iColorIndex];}
    inline const coreRenderTarget& GetDepthTarget  ()const                            {return m_DepthTarget;}
    inline const coreRenderTarget& GetStencilTarget()const                            {return m_StencilTarget;}
    inline const coreVector2&      GetResolution   ()const                            {return m_vResolution;}
    inline const coreFloat&        GetFOV          ()const                            {return m_fFOV;}
    inline const coreFloat&        GetNearClip     ()const                            {return m_fNearClip;}
    inline const coreFloat&        GetFarClip      ()const                            {return m_fFarClip;}
    //! @}


private:
    //! attach render targets
    //! @{
    coreRenderTarget* __AttachTarget(const coreFrameBufferTarget iTarget, const coreUintW iColorIndex, const coreTextureSpec& oSpec);
    //! @}
};


// ****************************************************************
// constructor
constexpr coreFrameBuffer::coreRenderTarget::coreRenderTarget()noexcept
: pTexture (NULL)
, iBuffer  (0u)
, oSpec    (coreTextureSpec(0u, 0u, 0u))
{
}


#endif // _CORE_GUARD_FRAMEBUFFER_H_