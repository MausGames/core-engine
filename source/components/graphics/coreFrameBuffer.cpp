///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

coreFrameBuffer* coreFrameBuffer::s_pCurrent      = NULL;
coreFloat        coreFrameBuffer::s_afViewData[5] = {};


// ****************************************************************
/* constructor */
coreFrameBuffer::coreFrameBuffer()noexcept
: m_iIdentifier   (0u)
, m_aColorTarget  {}
, m_DepthTarget   ()
, m_StencilTarget ()
, m_vResolution   (coreVector2(0.0f,0.0f))
, m_fFOV          (0.0f)
, m_fNearClip     (0.0f)
, m_fFarClip      (0.0f)
{
}


// ****************************************************************
/* destructor */
coreFrameBuffer::~coreFrameBuffer()
{
    // delete frame buffer object
    this->Delete();

    // detach all render targets
    this->DetachTargets();
}


// ****************************************************************
/* create frame buffer object */
coreStatus coreFrameBuffer::Create(const coreVector2 vResolution, const coreFrameBufferCreate eType)
{
    WARN_IF(m_iIdentifier) this->Delete();
    ASSERT(vResolution.x > 0.0f && vResolution.y > 0.0f)

    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(EXT_framebuffer_object)) return CORE_ERROR_SUPPORT;

    // generate frame buffer object
    glGenFramebuffers(1, &m_iIdentifier);
    glBindFramebuffer(GL_FRAMEBUFFER, m_iIdentifier);

    // set resolution
    const coreUint32 iWidth  = F_TO_UI(vResolution.x);
    const coreUint32 iHeight = F_TO_UI(vResolution.y);
    m_vResolution = coreVector2(I_TO_F(iWidth), I_TO_F(iHeight));

    // set view properties
    if(!m_fFOV)      m_fFOV      = Core::Graphics->GetFOV();
    if(!m_fNearClip) m_fNearClip = Core::Graphics->GetNearClip();
    if(!m_fFarClip)  m_fFarClip  = Core::Graphics->GetFarClip();

    // check for multisampling
    const coreUint8 iSamples     = CLAMP(Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING), 0, Core::Graphics->GetMaxSamples());
    const coreBool  bAmdAdvanced = CORE_GL_SUPPORT(AMD_framebuffer_multisample_advanced) && (eType == CORE_FRAMEBUFFER_CREATE_MULTISAMPLED) && (iSamples == 8u || iSamples == 4u || iSamples == 2u);
    const coreBool  bNvCoverage  = CORE_GL_SUPPORT(NV_framebuffer_multisample_coverage)  && (eType == CORE_FRAMEBUFFER_CREATE_MULTISAMPLED) && (iSamples == 8u || iSamples == 4u);
    const coreBool  bMultisample = CORE_GL_SUPPORT(EXT_framebuffer_multisample)          && (eType == CORE_FRAMEBUFFER_CREATE_MULTISAMPLED) && (iSamples >= 2u);

    // loop through all render targets
    __CORE_FRAMEBUFFER_ALL_TARGETS(apTarget)
    for(coreUintW i = 0u; i < ARRAY_SIZE(apTarget); ++i)
    {
        coreRenderTarget* pTarget = apTarget[i];
        if(!pTarget->IsValid()) continue;

        // set attachment point (depending on target order)
        GLenum iAttachment;
        switch(i)
        {
        default: iAttachment = GL_COLOR_ATTACHMENT0 + (i-2u); break;
        case 0u: iAttachment = GL_DEPTH_ATTACHMENT;           break;
        case 1u: iAttachment = GL_STENCIL_ATTACHMENT;         break;
        }

        if(pTarget->pTexture)
        {
            // create render target texture
            pTarget->pTexture->Create(iWidth, iHeight, pTarget->oSpec, CORE_TEXTURE_MODE_DEFAULT);

            // attach render target texture to frame buffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, iAttachment, GL_TEXTURE_2D, pTarget->pTexture->GetIdentifier(), 0);
        }
        else
        {
            // generate render target buffer
            glGenRenderbuffers(1, &pTarget->iBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, pTarget->iBuffer);

            // allocate buffer memory
                 if(bAmdAdvanced) glRenderbufferStorageMultisampleAdvancedAMD(GL_RENDERBUFFER, iSamples * ((i >= 2u) ? 2u : 1u), iSamples, pTarget->oSpec.iInternal, iWidth, iHeight);
            else if(bNvCoverage)  glRenderbufferStorageMultisampleCoverageNV (GL_RENDERBUFFER, iSamples * 2u,                    iSamples, pTarget->oSpec.iInternal, iWidth, iHeight);
            else if(bMultisample) glRenderbufferStorageMultisample           (GL_RENDERBUFFER,                                   iSamples, pTarget->oSpec.iInternal, iWidth, iHeight);
            else                  glRenderbufferStorage                      (GL_RENDERBUFFER,                                             pTarget->oSpec.iInternal, iWidth, iHeight);

            // attach render target buffer to frame buffer
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, iAttachment, GL_RENDERBUFFER, pTarget->iBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, 0u);
        }
    }

    // ignore color drawings without color attachment
    if(!m_aColorTarget[0].IsValid()) glDrawBuffer(GL_NONE);
    else
    {
        GLenum aiAttachment[CORE_SHADER_OUTPUT_COLORS];
        coreInt32 iNum = 0;

        // enable color drawings with all target buffers
        for(coreUintW i = 0u; i < CORE_SHADER_OUTPUT_COLORS; ++i)
        {
            // check for available color attachments
            if(m_aColorTarget[i].IsValid())
                aiAttachment[iNum++] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(iNum, aiAttachment);
    }

    // retrieve frame buffer status
    const GLenum iError = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetIdentifier() : 0u);

    // check for errors
    WARN_IF(iError != GL_FRAMEBUFFER_COMPLETE)
    {
        Core::Log->Warning("Frame Buffer Object could not be created (GL Error Code: 0x%08X)", iError);
        this->Delete();

        return CORE_ERROR_SYSTEM;
    }

    return CORE_OK;
}


// ****************************************************************
/* delete frame buffer object */
void coreFrameBuffer::Delete()
{
    if(!m_iIdentifier) return;

    // end rendering to still active frame buffer
    WARN_IF(s_pCurrent == this) coreFrameBuffer::EndDraw();

    // delete frame buffer (with implicit render target detachment)
    glDeleteFramebuffers(1, &m_iIdentifier);

    // loop through all render targets
    __CORE_FRAMEBUFFER_ALL_TARGETS(apTarget)
    for(coreUintW i = 0u; i < ARRAY_SIZE(apTarget); ++i)
    {
        // unload render target texture
        if(apTarget[i]->pTexture)
        {
            apTarget[i]->pTexture->Unload();
        }

        // delete render target buffer
        if(apTarget[i]->iBuffer)
        {
            glDeleteRenderbuffers(1, &apTarget[i]->iBuffer);
            apTarget[i]->iBuffer = 0u;
        }
    }

    // reset properties
    m_iIdentifier = 0u;
    m_vResolution = coreVector2(0.0f,0.0f);
}


// ****************************************************************
/* attach render target texture */
coreFrameBuffer::coreRenderTarget* coreFrameBuffer::AttachTargetTexture(const coreFrameBufferTarget eTarget, const coreUintW iColorIndex, const coreTextureSpec& oSpec, const coreChar* pcName)
{
    // get requested render target structure
    coreRenderTarget* pTarget = this->__AttachTarget(eTarget, iColorIndex, oSpec);
    if(pTarget)
    {
        // check for OpenGL extensions
        if((!HAS_FLAG(eTarget, CORE_FRAMEBUFFER_TARGET_DEPTH)   || CORE_GL_SUPPORT(ARB_depth_texture)) &&
           (!HAS_FLAG(eTarget, CORE_FRAMEBUFFER_TARGET_STENCIL) || CORE_GL_SUPPORT(ARB_texture_stencil8)))
        {
            // allocate render target texture
            if(pcName) pTarget->pTexture = Core::Manager::Resource->Load   <coreTexture>(pcName, CORE_RESOURCE_UPDATE_MANUAL, NULL);
                  else pTarget->pTexture = Core::Manager::Resource->LoadNew<coreTexture>();
        }
    }

    return pTarget;
}


// ****************************************************************
/* attach render target buffer */
coreFrameBuffer::coreRenderTarget* coreFrameBuffer::AttachTargetBuffer(const coreFrameBufferTarget eTarget, const coreUintW iColorIndex, const coreTextureSpec& oSpec)
{
    // get requested render target structure
    if(CORE_GL_SUPPORT(EXT_framebuffer_blit)) return this->__AttachTarget     (eTarget, iColorIndex, oSpec);
                                         else return this->AttachTargetTexture(eTarget, iColorIndex, oSpec);
}


// ****************************************************************
/* detach all render targets */
void coreFrameBuffer::DetachTargets()
{
    ASSERT(!m_iIdentifier)

    // loop through all render targets
    __CORE_FRAMEBUFFER_ALL_TARGETS(apTarget)
    for(coreUintW i = 0u; i < ARRAY_SIZE(apTarget); ++i)
    {
        // free render target texture
        if(apTarget[i]->pTexture) Core::Manager::Resource->Free(&apTarget[i]->pTexture);

        // reset properties
        apTarget[i]->oSpec = coreTextureSpec(0u, 0u, 0u);
    }
}


// ****************************************************************
/* start rendering to the frame buffer */
void coreFrameBuffer::StartDraw()
{
    ASSERT(m_iIdentifier && (s_pCurrent != this))

    // save view properties of the default frame buffer
    if(!s_afViewData[0])
    {
        s_afViewData[0] = Core::Graphics->GetViewResolution().x;
        s_afViewData[1] = Core::Graphics->GetViewResolution().y;
        s_afViewData[2] = Core::Graphics->GetFOV();
        s_afViewData[3] = Core::Graphics->GetNearClip();
        s_afViewData[4] = Core::Graphics->GetFarClip();
    }

    // set frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_iIdentifier);
    s_pCurrent = this;

    // set view frustum
    Core::Graphics->SetView(m_vResolution, m_fFOV, m_fNearClip, m_fFarClip);
}


// ****************************************************************
/* end rendering to the frame buffer */
void coreFrameBuffer::EndDraw()
{
    if(!s_pCurrent) return;

    // reset frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0u);
    s_pCurrent = NULL;

    // reset view frustum
    Core::Graphics->SetView(coreVector2(s_afViewData[0], s_afViewData[1]), s_afViewData[2], s_afViewData[3], s_afViewData[4]);
    std::memset(s_afViewData, 0, sizeof(s_afViewData));
}


// ****************************************************************
/* copy content to another frame buffer */
void coreFrameBuffer::Blit(const coreFrameBufferTarget eTargets, coreFrameBuffer* OUTPUT pDestination, const coreUint32 iSrcX, const coreUint32 iSrcY, const coreUint32 iDstX, const coreUint32 iDstY, const coreUint32 iWidth, const coreUint32 iHeight)const
{
    ASSERT(m_iIdentifier)
    ASSERT((!pDestination || ((iDstX + iWidth) <= F_TO_UI(pDestination->GetResolution().x) && (iDstY + iHeight) <= F_TO_UI(pDestination->GetResolution().y))) &&
                             ((iSrcX + iWidth) <= F_TO_UI(m_vResolution.x)                 && (iSrcY + iHeight) <= F_TO_UI(m_vResolution.y)))

    if(CORE_GL_SUPPORT(EXT_framebuffer_blit))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // copy content directly
            glBlitNamedFramebuffer(m_iIdentifier, pDestination ? pDestination->GetIdentifier() : 0u,
                                   iSrcX, iSrcY, iSrcX + iWidth, iSrcY + iHeight,
                                   iDstX, iDstY, iDstX + iWidth, iDstY + iHeight,
                                   eTargets, GL_NEAREST);
        }
        else
        {
            // switch to source and destination frame buffer
            if(s_pCurrent != this)         glBindFramebuffer(GL_READ_FRAMEBUFFER, m_iIdentifier);
            if(s_pCurrent != pDestination) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pDestination ? pDestination->GetIdentifier() : 0u);

            // copy content
            glBlitFramebuffer(iSrcX, iSrcY, iSrcX + iWidth, iSrcY + iHeight,
                              iDstX, iDstY, iDstX + iWidth, iDstY + iHeight,
                              eTargets, GL_NEAREST);

            // switch back to old frame buffer
            glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetIdentifier() : 0u);
        }
    }
    else
    {
        // blit without extension support
        if(pDestination)
        {
            // switch to source frame buffer
            const coreBool bToggle = (s_pCurrent != this);
            if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, m_iIdentifier);

            // handle color target blitting
            if(HAS_FLAG(eTargets, CORE_FRAMEBUFFER_TARGET_COLOR))
            {
                if(pDestination->m_aColorTarget[0].pTexture)
                {
                    // copy screen to destination texture
                    pDestination->m_aColorTarget[0].pTexture->Enable(0);
                    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iDstX, iDstY, iWidth, iHeight);
                }
            }

            // handle depth target blitting
            if(HAS_FLAG(eTargets, CORE_FRAMEBUFFER_TARGET_DEPTH))
            {
                if(pDestination->m_DepthTarget.pTexture)
                {
                    // attach source depth texture as color target
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_DepthTarget.pTexture->GetIdentifier(), 0u);

                    // copy screen to destination texture
                    pDestination->m_DepthTarget.pTexture->Enable(0);
                    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iDstX, iDstY, iWidth, iHeight);

                    // re-attach old color target
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_aColorTarget[0].pTexture ? m_aColorTarget[0].pTexture->GetIdentifier() : 0u, 0);
                }
            }

            // switch back to old frame buffer
            if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetIdentifier() : 0u);
        }
        else
        {
            // completely switch to default frame buffer
            coreFrameBuffer* pHoldCurrent = s_pCurrent;
            if(pHoldCurrent) coreFrameBuffer::EndDraw();

            // handle color target blitting onto the default frame buffer
            if(m_aColorTarget[0].pTexture)
            {
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);
                {
                    const coreFloat   fDstInvWid = Core::Graphics->GetViewResolution().w;
                    const coreVector2 vDstInvRes = Core::Graphics->GetViewResolution().zw();
                    const coreVector2 vSrcInvRes = coreVector2(1.0f,1.0f) / m_vResolution;

                    // forward transformation data and move
                    coreObject2D* pBlitFallback = Core::Manager::Object->GetBlitFallback();
                    pBlitFallback->SetSize     (coreVector2(I_TO_F(iWidth), -I_TO_F(iHeight)) * fDstInvWid);
                    pBlitFallback->SetCenter   (coreVector2(I_TO_F(iDstX),   I_TO_F(iDstY))   * fDstInvWid + coreVector2(I_TO_F(iWidth), I_TO_F(iHeight)) * vDstInvRes * 0.5f - 0.5f);
                    pBlitFallback->SetTexSize  (coreVector2(I_TO_F(iWidth),  I_TO_F(iHeight)) * vSrcInvRes);
                    pBlitFallback->SetTexOffset(coreVector2(I_TO_F(iSrcX),   I_TO_F(iSrcY))   * vSrcInvRes);
                    pBlitFallback->Move();

                    // forward source color texture and render
                    pBlitFallback->DefineTexture(0u, m_aColorTarget[0].pTexture);
                    pBlitFallback->Render();
                    pBlitFallback->DefineTexture(0u, NULL);
                }
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
            }

            // completely switch back to old frame buffer
            if(pHoldCurrent) pHoldCurrent->StartDraw();
        }
    }
}

void coreFrameBuffer::Blit(const coreFrameBufferTarget eTargets, coreFrameBuffer* OUTPUT pDestination)const
{
    this->Blit(eTargets, pDestination, 0u, 0u, 0u, 0u, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y));
}


// ****************************************************************
/* clear content of the frame buffer */
void coreFrameBuffer::Clear(const coreFrameBufferTarget eTargets)
{
    ASSERT(m_iIdentifier)

    // switch to destination frame buffer
    const coreBool bToggle = (s_pCurrent != this);
    if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, m_iIdentifier);

    // clear content
    glClear(eTargets);

    // switch back to old frame buffer
    if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetIdentifier() : 0u);
}


// ****************************************************************
/* invalidate content of the frame buffer */
coreStatus coreFrameBuffer::Invalidate(const coreFrameBufferTarget eTargets)
{
    ASSERT(m_iIdentifier)

    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        GLenum aiAttachment[CORE_SHADER_OUTPUT_COLORS + 2u];
        coreInt32 iNum = 0;

        // assemble required attachments
        if(HAS_FLAG(eTargets, CORE_FRAMEBUFFER_TARGET_COLOR))
        {
            for(coreUintW i = 0u; i < CORE_SHADER_OUTPUT_COLORS; ++i)
            {
                // check for available color attachments
                if(m_aColorTarget[i].IsValid())
                    aiAttachment[iNum++] = GL_COLOR_ATTACHMENT0 + i;
            }
        }
        if(HAS_FLAG(eTargets, CORE_FRAMEBUFFER_TARGET_DEPTH)   && m_DepthTarget  .IsValid()) aiAttachment[iNum++] = GL_DEPTH_ATTACHMENT;
        if(HAS_FLAG(eTargets, CORE_FRAMEBUFFER_TARGET_STENCIL) && m_StencilTarget.IsValid()) aiAttachment[iNum++] = GL_STENCIL_ATTACHMENT;
        WARN_IF(!iNum) return CORE_INVALID_INPUT;

        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // invalidate content directly
            glInvalidateNamedFramebufferData(m_iIdentifier, iNum, aiAttachment);
        }
        else
        {
            // switch to destination frame buffer
            const coreBool bToggle = (s_pCurrent != this);
            if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, m_iIdentifier);

            // invalidate content
            glInvalidateFramebuffer(GL_FRAMEBUFFER, iNum, aiAttachment);

            // switch back to old frame buffer
            if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetIdentifier() : 0u);
        }

        return CORE_OK;
    }

    return CORE_ERROR_SUPPORT;
}


// ****************************************************************
/* attach default render target */
coreFrameBuffer::coreRenderTarget* coreFrameBuffer::__AttachTarget(const coreFrameBufferTarget eTarget, const coreUintW iColorIndex, const coreTextureSpec& oSpec)
{
    ASSERT(!m_iIdentifier && (iColorIndex < CORE_SHADER_OUTPUT_COLORS))

#if defined(_CORE_GLES_)

    // currently only one color attachment supported
    if(iColorIndex > 0u) return NULL;

#endif

    // get requested render target structure
    coreRenderTarget* pTarget = NULL;
         if(HAS_FLAG(eTarget, CORE_FRAMEBUFFER_TARGET_COLOR))   pTarget = &m_aColorTarget[iColorIndex];
    else if(HAS_FLAG(eTarget, CORE_FRAMEBUFFER_TARGET_DEPTH))   pTarget = &m_DepthTarget;
    else if(HAS_FLAG(eTarget, CORE_FRAMEBUFFER_TARGET_STENCIL)) pTarget = &m_StencilTarget;
    else {WARN_IF(true) {} return NULL;}

    // free possible old texture
    if(pTarget->pTexture) Core::Manager::Resource->Free(&pTarget->pTexture);

    // set properties
    pTarget->oSpec = oSpec;

    return pTarget;
}