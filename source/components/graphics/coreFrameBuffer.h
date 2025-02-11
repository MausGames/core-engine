///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_FRAMEBUFFER_H_
#define _CORE_GUARD_FRAMEBUFFER_H_

// TODO 3: mipmapping ? (explicit function, how to reserve levels ? -> new create-enum-value, don't forget to move nearest-filter)
// TODO 5: check for multipl-viewport-rendering
// TODO 2: certain formats, format-combinations, as texture or rendertarget or with multisampling, may not be supported (GL_FRAMEBUFFER_UNSUPPORTED), implement transparent fallback
// TODO 3: fully implement multiple color attachment handling (esp. in blit (GL can only blit first attachment), clear (currently glClear), invalidate (already invalidates all attachments))
// TODO 3: handle depth/stencil-blitting to default frame buffer on fallback-blitting
// TODO 3: save and restore OpenGL states (GL_DEPTH_TEST/GL_BLEND) on fallback-blitting
// TODO 3: change R and RG targets to RGB if not supported ? also change some other targets if not supported
// TODO 5: <old comment style>
// TODO 4: do I still need the name parameter ?

// NOTE: superior objects have to handle resource-resets, to handle resolution-depending properties


// ****************************************************************
/* frame buffer definitions */
#define __CORE_FRAMEBUFFER_ALL_TARGETS(a)                                                     \
    coreRenderTarget* a[CORE_SHADER_OUTPUT_COLORS + 2u] = {&m_DepthTarget, &m_StencilTarget}; \
    for(coreUintW i = 2u; i < ARRAY_SIZE(a); ++i) a[i] = &m_aColorTarget[i - 2u];

enum coreFrameBufferCreate : coreBool
{
    CORE_FRAMEBUFFER_CREATE_NORMAL       = false,   // create normal frame buffer
    CORE_FRAMEBUFFER_CREATE_MULTISAMPLED = true     // create multisampled frame buffer
};

enum coreFrameBufferTarget : coreUint16
{
    CORE_FRAMEBUFFER_TARGET_COLOR   = GL_COLOR_BUFFER_BIT,    // use color buffer
    CORE_FRAMEBUFFER_TARGET_DEPTH   = GL_DEPTH_BUFFER_BIT,    // use depth buffer
    CORE_FRAMEBUFFER_TARGET_STENCIL = GL_STENCIL_BUFFER_BIT   // use stencil buffer
};
ENABLE_BITWISE(coreFrameBufferTarget)


// ****************************************************************
/* frame buffer class */
class coreFrameBuffer final
{
private:
    /* render target structure */
    struct coreRenderTarget final
    {
        coreTexturePtr  pTexture;   // render target texture (readable)
        GLuint          iBuffer;    // render target buffer (fast, multisampled)
        coreTextureSpec oSpec;      // texture and buffer specification (format)
        coreTextureMode eMode;      // texture mode (sampling)

        constexpr coreRenderTarget()noexcept;
        inline coreBool IsTexture()const {return pTexture ? true : false;}
        inline coreBool IsBuffer ()const {return iBuffer         != 0u;}
        inline coreBool IsValid  ()const {return oSpec.iInternal != 0u;}
    };


private:
    GLuint m_iIdentifier;                                         // frame buffer identifier

    coreRenderTarget m_aColorTarget[CORE_SHADER_OUTPUT_COLORS];   // attached color targets
    coreRenderTarget m_DepthTarget;                               // attached depth target
    coreRenderTarget m_StencilTarget;                             // attached stencil target

    coreVector2 m_vResolution;                                    // resolution of the frame buffer

    coreFloat m_fFOV;                                             // field-of-view
    coreFloat m_fNearClip;                                        // near clipping plane
    coreFloat m_fFarClip;                                         // far clipping plane
    coreFloat m_fAspectRatio;                                     // aspect ratio

    coreString m_sName;                                           // debug label

    coreBool m_bIntelMorph;                                       // apply conservative morphological anti aliasing

    static coreFrameBuffer* s_pCurrent;                           // currently active frame buffer object (NULL = default frame buffer)
    static coreFloat s_afViewData[6];                             // view properties of the default frame buffer


public:
    coreFrameBuffer()noexcept;
    ~coreFrameBuffer();

    DISABLE_COPY(coreFrameBuffer)

    /* control the frame buffer */
    coreStatus Create(const coreVector2 vResolution, const coreFrameBufferCreate eType);
    void       Delete();

    /* attach render targets */
    coreRenderTarget* AttachTargetTexture(const coreFrameBufferTarget eTarget, const coreUintW iColorIndex, const coreTextureSpec& oSpec, const coreTextureMode eMode, const coreChar* pcName = NULL);
    coreRenderTarget* AttachTargetBuffer (const coreFrameBufferTarget eTarget, const coreUintW iColorIndex, const coreTextureSpec& oSpec);
    void DetachTargets();

    /* enable rendering to the frame buffer */
    void StartDraw();
    static void EndDraw();

    /* copy content to another frame buffer */
    void Blit(const coreFrameBufferTarget eTargets, coreFrameBuffer* OUTPUT pDestination, const coreUint32 iSrcX, const coreUint32 iSrcY, const coreUint32 iDstX, const coreUint32 iDstY, const coreUint32 iWidth, const coreUint32 iHeight)const;
    void Blit(const coreFrameBufferTarget eTargets, coreFrameBuffer* OUTPUT pDestination)const;

    /* reset content of the frame buffer */
    void       Clear     (const coreFrameBufferTarget eTargets);
    coreStatus Invalidate(const coreFrameBufferTarget eTargets);

    /* set object properties */
    inline void SetFOV        (const coreFloat fFOV)         {m_fFOV         = fFOV;}
    inline void SetNearClip   (const coreFloat fNearClip)    {m_fNearClip    = fNearClip;}
    inline void SetFarClip    (const coreFloat fFarClip)     {m_fFarClip     = fFarClip;}
    inline void SetAspectRatio(const coreFloat fAspectRatio) {m_fAspectRatio = fAspectRatio;}
    inline void SetName       (const coreChar* pcName)       {m_sName        = pcName;}

    /* get object properties */
    inline const GLuint&           GetIdentifier   ()const                            {return m_iIdentifier;}
    inline const coreRenderTarget& GetColorTarget  (const coreUintW iColorIndex)const {ASSERT(iColorIndex < CORE_SHADER_OUTPUT_COLORS) return m_aColorTarget[iColorIndex];}
    inline const coreRenderTarget& GetDepthTarget  ()const                            {return m_DepthTarget;}
    inline const coreRenderTarget& GetStencilTarget()const                            {return m_StencilTarget;}
    inline const coreVector2&      GetResolution   ()const                            {return m_vResolution;}
    inline const coreFloat&        GetFOV          ()const                            {return m_fFOV;}
    inline const coreFloat&        GetNearClip     ()const                            {return m_fNearClip;}
    inline const coreFloat&        GetFarClip      ()const                            {return m_fFarClip;}


private:
    /* attach render targets */
    coreRenderTarget* __AttachTarget(const coreFrameBufferTarget eTarget, const coreUintW iColorIndex, const coreTextureSpec& oSpec, const coreTextureMode eMode);
};


// ****************************************************************
/* constructor */
constexpr coreFrameBuffer::coreRenderTarget::coreRenderTarget()noexcept
: pTexture (NULL)
, iBuffer  (0u)
, oSpec    (coreTextureSpec(0u, 0u, 0u, 0u, 0u))
, eMode    (CORE_TEXTURE_MODE_DEFAULT)
{
}


#endif /* _CORE_GUARD_FRAMEBUFFER_H_ */