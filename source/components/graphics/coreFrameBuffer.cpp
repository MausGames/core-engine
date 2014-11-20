//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreFrameBuffer* coreFrameBuffer::s_pCurrent          = NULL;
coreObject2D*    coreFrameBuffer::s_pBlitFallback     = NULL;
float            coreFrameBuffer::s_afViewData[5]; // = 0.0f;


// ****************************************************************
// constructor
coreFrameBuffer::coreFrameBuffer()noexcept
: m_iFrameBuffer (0)
, m_vResolution  (coreVector2(0.0f,0.0f))
, m_fFOV         (Core::Graphics->GetFOV())
, m_fNearClip    (Core::Graphics->GetNearClip())
, m_fFarClip     (Core::Graphics->GetFarClip())
{
}


// ****************************************************************
// destructor
coreFrameBuffer::~coreFrameBuffer()
{
    // delete frame buffer object
    this->Delete();

    // detach all render targets
    this->DetachTargets();
}


// ****************************************************************
// create frame buffer object
void coreFrameBuffer::Create(const coreVector2& vResolution, const coreFrameBufferCreate& bType)
{
    WARN_IF(m_iFrameBuffer) this->Delete();
    ASSERT(vResolution.x > 0.0f && vResolution.y > 0.0f)

    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(EXT_framebuffer_object)) return;

    // generate frame buffer object
    glGenFramebuffers(1, &m_iFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

    // set resolution
    const int iWidth  = F_TO_SI(vResolution.x);
    const int iHeight = F_TO_SI(vResolution.y);
    m_vResolution     = coreVector2(I_TO_F(iWidth), I_TO_F(iHeight));

    // set number of samples
    const int iSamples = ((bType == CORE_FRAMEBUFFER_CREATE_MULTISAMPLED) && CORE_GL_SUPPORT(EXT_framebuffer_multisample)) ?
                         Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING) : 0;

    // loop through all render targets
    __CORE_FRAMEBUFFER_ALL_TARGETS(apTarget)
    for(coreByte i = 0; i < ARRAY_SIZE(apTarget); ++i)
    {
        coreRenderTarget* pTarget = apTarget[i];
        if(!pTarget->iInternal) continue;

        // set attachment point (depending on target order)
        int iAttachment;
        switch(i)
        {
        default: iAttachment = GL_COLOR_ATTACHMENT0 + (i-2); break;
        case 0:  iAttachment = GL_DEPTH_ATTACHMENT;          break;
        case 1:  iAttachment = GL_STENCIL_ATTACHMENT;        break;
        }

        if(pTarget->pTexture)
        {
            // create render target texture
            pTarget->pTexture->Create(iWidth, iHeight, pTarget->iInternal, pTarget->iFormat, pTarget->iType, GL_CLAMP_TO_EDGE, false);

            // attach render target texture to frame buffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, iAttachment, GL_TEXTURE_2D, pTarget->pTexture->GetTexture(), 0);
        }
        else
        {
            // generate render target buffer
            glGenRenderbuffers(1, &pTarget->iBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, pTarget->iBuffer);

            // allocate buffer memory
            if(iSamples) glRenderbufferStorageMultisample(GL_RENDERBUFFER, iSamples, pTarget->iInternal, iWidth, iHeight);
                    else glRenderbufferStorage           (GL_RENDERBUFFER,           pTarget->iInternal, iWidth, iHeight);

            // attach render target buffer to frame buffer
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, iAttachment, GL_RENDERBUFFER, pTarget->iBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    }

    // ignore color drawings without color attachment
    if(!m_aColorTarget[0].iInternal) glDrawBuffer(GL_NONE);

    // retrieve frame buffer status
    const GLenum iError = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetFrameBuffer() : 0);

    // check for errors
    if(iError != GL_FRAMEBUFFER_COMPLETE)
    {
        Core::Log->Warning("Frame Buffer Object could not be created (GL Error Code: 0x%04X)", iError);
        this->Delete();
    }
}


// ****************************************************************
// delete frame buffer object
void coreFrameBuffer::Delete()
{
    if(!m_iFrameBuffer) return;

    // end rendering to still active frame buffer
    WARN_IF(s_pCurrent == this) coreFrameBuffer::EndDraw();

    // delete frame buffer (with implicit render target detachment)
    glDeleteFramebuffers(1, &m_iFrameBuffer);

    // loop through all render targets
    __CORE_FRAMEBUFFER_ALL_TARGETS(apTarget)
    for(coreByte i = 0; i < ARRAY_SIZE(apTarget); ++i)
    {
        // unload render target texture
        if(apTarget[i]->pTexture) apTarget[i]->pTexture->Unload();

        // delete render target buffer
        if(apTarget[i]->iBuffer)
        {
            glDeleteRenderbuffers(1, &apTarget[i]->iBuffer);
            apTarget[i]->iBuffer = 0;
        }
    }

    // reset properties
    m_iFrameBuffer = 0;
    m_vResolution  = coreVector2(0.0f,0.0f);
}


// ****************************************************************
// attach render target texture
coreFrameBuffer::coreRenderTarget* coreFrameBuffer::AttachTargetTexture(const coreFrameBufferTarget& iTarget, const coreByte& iColorIndex, const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType, const char* pcName)
{
    // get requested render target structure
    coreRenderTarget* pTarget = this->__AttachTarget(iTarget, iColorIndex, iInternal, iFormat, iType);
    if(pTarget)
    {
        // check for OpenGL extensions
        if((!(iTarget & CORE_FRAMEBUFFER_TARGET_DEPTH)   || CORE_GL_SUPPORT(ARB_depth_texture)) &&
           (!(iTarget & CORE_FRAMEBUFFER_TARGET_STENCIL) || CORE_GL_SUPPORT(ARB_texture_stencil8)))
        {
            // allocate render target texture
            if(pcName) pTarget->pTexture = Core::Manager::Resource->Load   <coreTexture>(pcName, CORE_RESOURCE_UPDATE_MANUAL, NULL);
                  else pTarget->pTexture = Core::Manager::Resource->LoadNew<coreTexture>();
        }
    }

    return pTarget;
}


// ****************************************************************
// attach render target buffer
coreFrameBuffer::coreRenderTarget* coreFrameBuffer::AttachTargetBuffer(const coreFrameBufferTarget& iTarget, const coreByte& iColorIndex, const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType)
{
    // get requested render target structure
    if(CORE_GL_SUPPORT(EXT_framebuffer_blit)) return this->__AttachTarget     (iTarget, iColorIndex, iInternal, iFormat, iType);
                                         else return this->AttachTargetTexture(iTarget, iColorIndex, iInternal, iFormat, iType);
}


// ****************************************************************
// detach all render targets
void coreFrameBuffer::DetachTargets()
{
    ASSERT(!m_iFrameBuffer)

    // loop through all render targets
    __CORE_FRAMEBUFFER_ALL_TARGETS(apTarget)
    for(coreByte i = 0; i < ARRAY_SIZE(apTarget); ++i)
    {
        // free render target texture
        if(apTarget[i]->pTexture) Core::Manager::Resource->Free(&apTarget[i]->pTexture);

        // reset properties
        apTarget[i]->iInternal = 0;
        apTarget[i]->iFormat   = 0;
        apTarget[i]->iType     = 0;
    }
}


// ****************************************************************
// start rendering to the frame buffer
void coreFrameBuffer::StartDraw()
{
    ASSERT(m_iFrameBuffer && (s_pCurrent != this))

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
    glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);
    s_pCurrent = this;

    // set view frustum
    Core::Graphics->SetView(m_vResolution, m_fFOV, m_fNearClip, m_fFarClip);
}


// ****************************************************************
// end rendering to the frame buffer
void coreFrameBuffer::EndDraw()
{
    if(!s_pCurrent) return;

    // reset frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    s_pCurrent = NULL;

    // reset view frustum
    Core::Graphics->SetView(coreVector2(s_afViewData[0], s_afViewData[1]), s_afViewData[2], s_afViewData[3], s_afViewData[4]);
    std::memset(s_afViewData, 0, sizeof(s_afViewData));
}


// ****************************************************************
// copy content to another frame buffer
void coreFrameBuffer::Blit(const coreFrameBufferTarget& iTargets, coreFrameBuffer* pDestination, const coreUint& iSrcX, const coreUint& iSrcY, const coreUint& iDstX, const coreUint& iDstY, const coreUint& iWidth, const coreUint& iHeight)const
{
    ASSERT(m_iFrameBuffer)
    ASSERT((!pDestination || ((iDstX + iWidth) <= F_TO_UI(pDestination->GetResolution().x) && (iDstY + iHeight) <= F_TO_UI(pDestination->GetResolution().y))) && 
                             ((iSrcX + iWidth) <= F_TO_UI(m_vResolution.x)                 && (iSrcY + iHeight) <= F_TO_UI(m_vResolution.y)))

    if(CORE_GL_SUPPORT(EXT_framebuffer_blit))
    {
        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // copy content directly
            glBlitNamedFramebuffer(m_iFrameBuffer, pDestination ? pDestination->GetFrameBuffer() : 0,
                                   iSrcX, iSrcY, iSrcX + iWidth, iSrcY + iHeight,
                                   iDstX, iDstY, iDstX + iWidth, iDstY + iHeight,
                                   iTargets, GL_NEAREST);
        }
        else
        {
            // switch to source and destination frame buffer
            if(s_pCurrent != this)         glBindFramebuffer(GL_READ_FRAMEBUFFER, m_iFrameBuffer);
            if(s_pCurrent != pDestination) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pDestination ? pDestination->GetFrameBuffer() : 0);

            // copy content
            glBlitFramebuffer(iSrcX, iSrcY, iSrcX + iWidth, iSrcY + iHeight,
                              iDstX, iDstY, iDstX + iWidth, iDstY + iHeight,
                              iTargets, GL_NEAREST);

            // switch back to old frame buffer
            glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetFrameBuffer() : 0);
        }
    }
    else
    {
        // blit without extension support
        if(pDestination)
        {
            // switch to source frame buffer
            const bool bToggle = (s_pCurrent != this);
            if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

            // handle color target blitting
            if(iTargets & CORE_FRAMEBUFFER_TARGET_COLOR)
            {
                if(pDestination->m_aColorTarget[0].pTexture)
                {
                    // copy screen to destination texture
                    pDestination->m_aColorTarget[0].pTexture->Enable(0);
                    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iDstX, iDstY, iWidth, iHeight);
                }
            }

            // handle depth target blitting
            if(iTargets & CORE_FRAMEBUFFER_TARGET_DEPTH)
            {
                if(pDestination->m_DepthTarget.pTexture)
                {
                    // attach source depth texture as color target
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_DepthTarget.pTexture->GetTexture(), 0);

                    // copy screen to destination texture
                    pDestination->m_DepthTarget.pTexture->Enable(0);
                    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iDstX, iDstY, iWidth, iHeight);

                    // re-attach old color target
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_aColorTarget[0].pTexture ? m_aColorTarget[0].pTexture->GetTexture() : 0, 0);
                }
            }

            // switch back to old frame buffer
            if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetFrameBuffer() : 0);
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
                    const float&      fDstInvWid = Core::Graphics->GetViewResolution().w;
                    const coreVector2 vDstInvRes = Core::Graphics->GetViewResolution().zw();
                    const coreVector2 vSrcInvRes = coreVector2(1.0f,1.0f) / m_vResolution;

                    // forward transformation data and move
                    s_pBlitFallback->SetSize     (coreVector2(I_TO_F(iWidth), -I_TO_F(iHeight)) * fDstInvWid);
                    s_pBlitFallback->SetCenter   (coreVector2(I_TO_F(iDstX),   I_TO_F(iDstY))   * fDstInvWid + coreVector2(I_TO_F(iWidth), I_TO_F(iHeight)) * vDstInvRes * 0.5f - 0.5f);
                    s_pBlitFallback->SetTexSize  (coreVector2(I_TO_F(iWidth),  I_TO_F(iHeight)) * vSrcInvRes);
                    s_pBlitFallback->SetTexOffset(coreVector2(I_TO_F(iSrcX),   I_TO_F(iSrcY))   * vSrcInvRes);
                    s_pBlitFallback->Move();

                    // forward source color texture and render
                    s_pBlitFallback->DefineTexture(0, m_aColorTarget[0].pTexture);
                    s_pBlitFallback->Render();
                    s_pBlitFallback->DefineTexture(0, NULL);
                }
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
            }

            // completely switch back to old frame buffer
            if(pHoldCurrent) pHoldCurrent->StartDraw();
        }
    }
}

void coreFrameBuffer::Blit(const coreFrameBufferTarget& iTargets, coreFrameBuffer* pDestination)const
{
    this->Blit(iTargets, pDestination, 0, 0, 0, 0, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y));
}


// ****************************************************************
// clear content of the frame buffer
void coreFrameBuffer::Clear(const coreFrameBufferTarget& iTargets)
{
    ASSERT(m_iFrameBuffer)

    // switch to destination frame buffer
    const bool bToggle = (s_pCurrent != this);
    if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

    // clear content
    glClear(iTargets);

    // switch back to old frame buffer
    if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetFrameBuffer() : 0);
}


// ****************************************************************
// invalidate content of the frame buffer
void coreFrameBuffer::Invalidate(const coreFrameBufferTarget& iTargets)
{
    ASSERT(m_iFrameBuffer)

    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        GLenum aiAttachment[3];
        coreByte iNum = 0;

        // assemble required attachments
        if(iTargets & CORE_FRAMEBUFFER_TARGET_COLOR)   aiAttachment[iNum++] = GL_COLOR_ATTACHMENT0;
        if(iTargets & CORE_FRAMEBUFFER_TARGET_DEPTH)   aiAttachment[iNum++] = GL_DEPTH_ATTACHMENT;
        if(iTargets & CORE_FRAMEBUFFER_TARGET_STENCIL) aiAttachment[iNum++] = GL_STENCIL_ATTACHMENT;
        WARN_IF(!iNum) return;

        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // invalidate content directly
            glInvalidateNamedFramebufferData(m_iFrameBuffer, iNum, aiAttachment);
        }
        else
        {
            // switch to destination frame buffer
            const bool bToggle = (s_pCurrent != this);
            if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

            // invalidate content
            glInvalidateFramebuffer(GL_FRAMEBUFFER, iNum, aiAttachment);

            // switch back to old frame buffer
            if(bToggle) glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetFrameBuffer() : 0);
        }
    }
}


// ****************************************************************
// attach default render target
coreFrameBuffer::coreRenderTarget* coreFrameBuffer::__AttachTarget(const coreFrameBufferTarget& iTarget, const coreByte& iColorIndex, const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType)
{
    ASSERT(!m_iFrameBuffer && (iColorIndex < CORE_SHADER_OUTPUT_COLORS))

#if defined(_CORE_GLES_)

    // currently only one color attachment supported
    if(iColorIndex > 0) return NULL;

#endif

    // get requested render target structure
    coreRenderTarget* pTarget = NULL;
         if(iTarget & CORE_FRAMEBUFFER_TARGET_COLOR)   pTarget = &m_aColorTarget[iColorIndex];
    else if(iTarget & CORE_FRAMEBUFFER_TARGET_DEPTH)   pTarget = &m_DepthTarget;
    else if(iTarget & CORE_FRAMEBUFFER_TARGET_STENCIL) pTarget = &m_StencilTarget;
    else {ASSERT(false) return NULL;}

    // free possible old texture
    if(pTarget->pTexture) Core::Manager::Resource->Free(&pTarget->pTexture);

    // set properties
    pTarget->iInternal = iInternal;
    pTarget->iFormat   = iFormat;
    pTarget->iType     = iType;

    return pTarget;
}