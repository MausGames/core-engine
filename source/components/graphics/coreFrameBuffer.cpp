//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreFrameBuffer* coreFrameBuffer::s_pCurrent = NULL;


// ****************************************************************
// constructor
coreFrameBuffer::coreFrameBuffer()noexcept
: m_iFrameBuffer (0)
, m_iDepthBuffer (0)
, m_vResolution  (coreVector2(0.0f,0.0f))
, m_iType        (CORE_FRAMEBUFFER_TYPE_FULL)
, m_fFOV         (Core::Graphics->GetFOV())
, m_fNearClip    (Core::Graphics->GetNearClip())
, m_fFarClip     (Core::Graphics->GetFarClip())
{
}


// ****************************************************************
// create frame buffer object
void coreFrameBuffer::Create(const coreVector2& vResolution, const coreFrameBufferType& iType, const char* pcTextureName)
{
    ASSERT_IF(m_pTexture) this->Delete();
    ASSERT(vResolution.x > 0.0f && vResolution.y > 0.0f)

    // check for OpenGL extensions
    const GLboolean& bFrameBuffer = GLEW_ARB_framebuffer_object;

    // set resolution
    m_vResolution = bFrameBuffer ? vResolution : (vResolution * MIN((Core::System->GetResolution() / vResolution).Min(), 1.0f));
    m_vResolution = coreVector2(FLOOR(m_vResolution.x), FLOOR(m_vResolution.y));
    const int iWidth  = (int)m_vResolution.x;
    const int iHeight = (int)m_vResolution.y;

    // set type and texture format
    m_iType = iType;
    const GLenum iInternal = (m_iType & CORE_FRAMEBUFFER_TYPE_COLOR) ? ((m_iType & CORE_FRAMEBUFFER_TYPE_ALPHA) ? GL_RGBA8 : GL_RGB8) : GL_DEPTH_COMPONENT16;
    const GLenum iFormat   = (m_iType & CORE_FRAMEBUFFER_TYPE_COLOR) ? ((m_iType & CORE_FRAMEBUFFER_TYPE_ALPHA) ? GL_RGBA  : GL_RGB)  : GL_DEPTH_COMPONENT;

    // allocate own texture as render target
    if(pcTextureName) m_pTexture = Core::Manager::Resource->Load   <coreTexture>(pcTextureName, CORE_RESOURCE_UPDATE_MANUAL, NULL);
                 else m_pTexture = Core::Manager::Resource->LoadNew<coreTexture>();

    // create base texture
    m_pTexture->Create(iWidth, iHeight, iInternal, iFormat, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, false);

    if(bFrameBuffer)
    {
        // generate frame buffer object
        glGenFramebuffers(1, &m_iFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

        switch(m_iType & 0x00FF)
        {
        case CORE_FRAMEBUFFER_TYPE_FULL:

            // generate depth buffer
            glGenRenderbuffers(1, &m_iDepthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, m_iDepthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, iWidth, iHeight);

            // attach depth buffer as depth component
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iDepthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

        case CORE_FRAMEBUFFER_TYPE_COLOR:

            // attach texture as first color component
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pTexture->GetTexture(), 0);
            break;

        case CORE_FRAMEBUFFER_TYPE_DEPTH:

            // attach texture as depth component
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_pTexture->GetTexture(), 0);
            break;

        default: ASSERT(false)
        }

        // get frame buffer status
        const GLenum iError = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // check for errors
        if(iError != GL_FRAMEBUFFER_COMPLETE)
        {
            Core::Log->Warning("Frame Buffer Object could not be created (GL Error Code: %d)", iError);
            this->__DeleteBuffers();
        }
    }
}


// ****************************************************************
// delete frame buffer object
void coreFrameBuffer::Delete()
{
    if(!m_pTexture) return;

    // delete frame and depth buffer
    this->__DeleteBuffers();

    // free own texture
    Core::Manager::Resource->Free(&m_pTexture);

    // reset properties
    m_vResolution = coreVector2(0.0f,0.0f);
    m_iType       = CORE_FRAMEBUFFER_TYPE_FULL;
}


// ****************************************************************
// clear content of the frame buffer
void coreFrameBuffer::Clear(const coreFrameBufferUse& iBuffer)
{
    ASSERT(m_pTexture)
    
    // switch to target frame buffer
    const bool bToggle = (this != s_pCurrent) && m_iFrameBuffer;
    if(bToggle) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_iFrameBuffer);

    // clear the whole frame buffer
    glClear(((iBuffer & CORE_FRAMEBUFFER_USE_COLOR)   ? GL_COLOR_BUFFER_BIT   : 0) | 
            ((iBuffer & CORE_FRAMEBUFFER_USE_DEPTH)   ? GL_DEPTH_BUFFER_BIT   : 0) |
            ((iBuffer & CORE_FRAMEBUFFER_USE_STENCIL) ? GL_STENCIL_BUFFER_BIT : 0));

    // switch back to old frame buffer
    if(bToggle) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetFrameBuffer() : 0);
}


// ****************************************************************
// invalidate content of the frame buffer
void coreFrameBuffer::Invalidate(const coreFrameBufferUse& iBuffer)
{
    ASSERT(m_pTexture)

    // invalidate the whole frame buffer
    if(GLEW_ARB_invalidate_subdata)
    {
        GLenum aiAttachment[3];
        coreByte iNum = 0;

        // assemble required attachments
        if(iBuffer & CORE_FRAMEBUFFER_USE_COLOR)   aiAttachment[iNum++] = GL_COLOR_ATTACHMENT0;
        if(iBuffer & CORE_FRAMEBUFFER_USE_DEPTH)   aiAttachment[iNum++] = GL_DEPTH_ATTACHMENT;
        if(iBuffer & CORE_FRAMEBUFFER_USE_STENCIL) aiAttachment[iNum++] = GL_STENCIL_ATTACHMENT;
        ASSERT_IF(!iNum) return;

        if(GLEW_ARB_direct_state_access)
        {
            // invalidate content directly
            glInvalidateNamedFramebufferData(m_iFrameBuffer, iNum, aiAttachment);
        }
        else
        {
            // switch to target frame buffer
            const bool bToggle = (this != s_pCurrent) && m_iFrameBuffer;
            if(bToggle) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_iFrameBuffer);

            // invalidate content
            glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, iNum, aiAttachment);

            // switch back to old frame buffer
            if(bToggle) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetFrameBuffer() : 0);
        }
    }
}


// ****************************************************************
// copy content to another frame buffer
void coreFrameBuffer::Blit(coreFrameBuffer* pTarget, const coreFrameBufferUse& iBuffer, const coreUint& iSrcX, const coreUint& iSrcY, const coreUint& iDstX, const coreUint& iDstY, const coreUint& iWidth, const coreUint& iHeight)
{
    ASSERT(m_pTexture)
    ASSERT((iSrcX + iWidth) <= coreUint(m_vResolution.x)            && (iSrcY + iHeight) <= coreUint(m_vResolution.y) &&
           (iDstX + iWidth) <= coreUint(pTarget->GetResolution().x) && (iDstY + iHeight) <= coreUint(pTarget->GetResolution().y))

    if(m_iFrameBuffer)
    {
        // assemble required buffer-bits
        const GLenum iBits = ((iBuffer & CORE_FRAMEBUFFER_USE_COLOR)   ? GL_COLOR_BUFFER_BIT   : 0) | 
                             ((iBuffer & CORE_FRAMEBUFFER_USE_DEPTH)   ? GL_DEPTH_BUFFER_BIT   : 0) |
                             ((iBuffer & CORE_FRAMEBUFFER_USE_STENCIL) ? GL_STENCIL_BUFFER_BIT : 0);

        if(GLEW_ARB_direct_state_access)
        {
            // copy content directly
            glBlitNamedFramebuffer(m_iFrameBuffer, pTarget ? pTarget->GetFrameBuffer() : 0,
                                   iSrcX, iSrcY, iSrcX + iWidth, iSrcY + iHeight, 
                                   iDstX, iDstY, iDstX + iWidth, iDstY + iHeight,
                                   iBits, GL_NEAREST);
        }
        else
        {
            // switch to source and target frame buffer
            if(this    != s_pCurrent) glBindFramebuffer(GL_READ_FRAMEBUFFER, m_iFrameBuffer);
            if(pTarget != s_pCurrent) glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pTarget ? pTarget->GetFrameBuffer() : 0);
                 
            // copy content
            glBlitFramebuffer(iSrcX, iSrcY, iSrcX + iWidth, iSrcY + iHeight, 
                              iDstX, iDstY, iDstX + iWidth, iDstY + iHeight,
                              iBits, GL_NEAREST);

            // switch back to old frame buffer
            glBindFramebuffer(GL_FRAMEBUFFER, s_pCurrent ? s_pCurrent->GetFrameBuffer() : 0);
        }
    }
    else if(m_pTexture)
    {
        // copy screen to texture
        pTarget->GetTexture()->Enable(0);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iDstX, iDstY, iWidth, iHeight);
    }
}

void coreFrameBuffer::Blit(coreFrameBuffer* pTarget, const coreFrameBufferUse& iBuffer)
{
    this->Blit(pTarget, iBuffer, 0, 0, 0, 0, (coreUint)m_vResolution.x, (coreUint)m_vResolution.y);
}


// ****************************************************************
// start rendering to the frame buffer
void coreFrameBuffer::StartDraw()
{
    ASSERT(s_pCurrent == NULL && m_pTexture)
    s_pCurrent = this;

    // set current frame buffer
    if(m_iFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

    // set view
    Core::Graphics->ResizeView(m_vResolution, m_fFOV, m_fNearClip, m_fFarClip);
}


// ****************************************************************
// end rendering to the frame buffer
void coreFrameBuffer::EndDraw()
{
    ASSERT(s_pCurrent == this && m_pTexture)
    s_pCurrent = NULL;

    // reset current frame buffer
    if(m_iFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, 0);
    else this->EndDrawBound();

    // reset view
    Core::Graphics->ResizeView(Core::System->GetResolution(), Core::Graphics->GetFOV(), 
                               Core::Graphics->GetNearClip(), Core::Graphics->GetFarClip());
}


// ****************************************************************
// end rendering without resetting frame buffer and view
void coreFrameBuffer::EndDrawBound()
{
    ASSERT(m_pTexture)
    s_pCurrent = NULL;

    if(!m_iFrameBuffer && m_pTexture)
    {
        // copy screen to texture
        m_pTexture->Enable(0);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, (coreUint)m_vResolution.x, (coreUint)m_vResolution.y);

        // clear depth buffer
        glClear(GL_DEPTH_BUFFER_BIT);
    }
}


// ****************************************************************
// reset with the resource manager
void coreFrameBuffer::__Reset(const coreResourceReset& bInit)
{
    if(!m_pTexture) return;

    if(bInit) 
    {
        // save texture name and free own texture
        const char* pcTextureName = PRINT("%s", m_pTexture.GetHandle()->GetName());
        Core::Manager::Resource->Free(&m_pTexture);

        // re-create the frame buffer object
        this->Create(m_vResolution, m_iType, *pcTextureName ? pcTextureName : NULL);
    }
    else
    {
        // delete frame and depth buffer
        this->__DeleteBuffers();

        // unload own texture
        m_pTexture->Unload();
    }
}


// ****************************************************************
// delete frame and depth buffer
void coreFrameBuffer::__DeleteBuffers()
{
    // end rendering to still active frame buffer
    ASSERT_IF(this == s_pCurrent)
        this->EndDraw();

    // delete frame and depth buffer
    if(m_iFrameBuffer) glDeleteFramebuffers (1, &m_iFrameBuffer);
    if(m_iDepthBuffer) glDeleteRenderbuffers(1, &m_iDepthBuffer);
    m_iFrameBuffer = 0;
    m_iDepthBuffer = 0;
}