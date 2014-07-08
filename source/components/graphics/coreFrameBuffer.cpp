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
coreFrameBuffer::coreFrameBuffer(const coreVector2& vResolution, const int& iType, const char* pcLink)noexcept
: m_iFrameBuffer (0)
, m_iDepthBuffer (0)
, m_vResolution  (vResolution)
, m_iType        (iType)
, m_fFOV         (Core::Graphics->GetFOV())
, m_fNearClip    (Core::Graphics->GetNearClip())
, m_fFarClip     (Core::Graphics->GetFarClip())
{
    // create own texture as render target
    if(pcLink) m_pTexture = Core::Manager::Resource->LoadLink<coreTexture>(pcLink);
          else m_pTexture = Core::Manager::Resource->LoadNew<coreTexture>();

    // init the frame buffer
    this->__Init();
}


// ****************************************************************
// destructor
coreFrameBuffer::~coreFrameBuffer()
{
    // exit the frame buffer
    this->__Exit();
    
    // free own texture
    Core::Manager::Resource->Free(&m_pTexture);
}


// ****************************************************************
// start writing to the frame buffer
void coreFrameBuffer::StartWrite()
{
    ASSERT(s_pCurrent == NULL && m_pTexture->GetTexture())
    s_pCurrent = this;

    // set current frame buffer
    if(m_iFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

    // set view
    if(m_vResolution != Core::System->GetResolution() ||
       m_fFOV        != Core::Graphics->GetFOV()      ||
       m_fNearClip   != Core::Graphics->GetNearClip() ||
       m_fFarClip    != Core::Graphics->GetFarClip())
    {
        Core::Graphics->ResizeView(m_vResolution, m_fFOV, m_fNearClip, m_fFarClip);
    }
}


// ****************************************************************
// end writing to the frame buffer
void coreFrameBuffer::EndWrite()
{
    ASSERT(s_pCurrent == this && m_pTexture->GetTexture())
    s_pCurrent = NULL;

    // reset current frame buffer
    if(m_iFrameBuffer) glBindFramebuffer(GL_FRAMEBUFFER, 0);
    else
    {
        // copy screen to texture
        m_pTexture->Enable(0);
        coreTexture::Lock();
        {
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, (int)m_vResolution.x, (int)m_vResolution.y);
        }
        coreTexture::Unlock();

        // reset the depth buffer
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    // reset view
    if(m_vResolution != Core::System->GetResolution() ||
       m_fFOV        != Core::Graphics->GetFOV()      ||
       m_fNearClip   != Core::Graphics->GetNearClip() ||
       m_fFarClip    != Core::Graphics->GetFarClip())
    {
        Core::Graphics->ResizeView(Core::System->GetResolution(), Core::Graphics->GetFOV(), 
                                   Core::Graphics->GetNearClip(), Core::Graphics->GetFarClip());
    }
}


// ****************************************************************
// clear content of the frame buffer
void coreFrameBuffer::Clear()
{
    ASSERT(s_pCurrent == this && m_pTexture->GetTexture())

    // clear the whole frame buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


// ****************************************************************
// invalidate content of the frame buffer
void coreFrameBuffer::Invalidate()
{
    ASSERT(s_pCurrent == this && m_pTexture->GetTexture())
                 
    // invalidate the whole frame buffer
    if(GLEW_ARB_invalidate_subdata)
    {
        constexpr_var GLenum aiAttachment[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, aiAttachment);
    }
}


// ****************************************************************
// reset with the resource manager
void coreFrameBuffer::__Reset(const bool& bInit)
{
    if(bInit) this->__Init();
         else this->__Exit();
}


// ****************************************************************
// init the frame buffer
coreError coreFrameBuffer::__Init()
{
    ASSERT_IF(m_pTexture->GetTexture()) return CORE_INVALID_CALL;
    ASSERT(m_vResolution.x > 0.0f && m_vResolution.y > 0.0f)

    // check for OpenGL extensions
    const GLboolean& bStorage     = GLEW_ARB_texture_storage;
    const GLboolean& bFrameBuffer = GLEW_ARB_framebuffer_object;

    // set resolution
    if(!bFrameBuffer) m_vResolution *= MIN((Core::System->GetResolution() / m_vResolution).Min(), 1.0f);
    const coreUint iWidth  = (coreUint)m_vResolution.x;
    const coreUint iHeight = (coreUint)m_vResolution.y;

    // set texture format
    const GLenum iInternal = (m_iType & CORE_FRAMEBUFFER_DEPTH) ? GL_DEPTH_COMPONENT16 : ((m_iType & CORE_FRAMEBUFFER_ALPHA) ? GL_RGBA8 : GL_RGB8);
    const GLenum iFormat   = (m_iType & CORE_FRAMEBUFFER_DEPTH) ? GL_DEPTH_COMPONENT   : ((m_iType & CORE_FRAMEBUFFER_ALPHA) ? GL_RGBA  : GL_RGB);

    // generate empty base texture
    m_pTexture->Generate();

    // specifiy the required texture
    m_pTexture->Enable(0);
    coreTexture::Lock();
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

        // allocate texture memory
        if(bStorage) glTexStorage2D(GL_TEXTURE_2D, 1, iInternal, iWidth, iHeight);
                else glTexImage2D(GL_TEXTURE_2D, 0, iInternal, iWidth, iHeight, 0, iFormat, GL_UNSIGNED_BYTE, 0);
    }
    coreTexture::Unlock();

    if(bFrameBuffer)
    {
        // generate frame buffer object
        glGenFramebuffers(1, &m_iFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

        switch(m_iType & 0x00FF)
        {
        case CORE_FRAMEBUFFER_FULL:

            // generate depth buffer
            glGenRenderbuffers(1, &m_iDepthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, m_iDepthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, iWidth, iHeight);

            // attach depth buffer as depth component
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iDepthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

        case CORE_FRAMEBUFFER_COLOR:

            // attach texture as first color component
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pTexture->GetTexture(), 0);
            break;

        case CORE_FRAMEBUFFER_DEPTH:

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

            return CORE_ERROR_SUPPORT;
        }
    }

    return CORE_OK;
}


// ****************************************************************
// exit the frame buffer
coreError coreFrameBuffer::__Exit()
{
    if(!m_pTexture->GetTexture()) return CORE_INVALID_CALL;

    // delete frame and depth buffer
    this->__DeleteBuffers();

    // unload own texture
    m_pTexture->Unload();

    return CORE_OK;
}


// ****************************************************************
// delete frame and depth buffer
void coreFrameBuffer::__DeleteBuffers()
{
    glDeleteFramebuffers(1, &m_iFrameBuffer);
    glDeleteRenderbuffers(1, &m_iDepthBuffer);
    m_iFrameBuffer = 0;
    m_iDepthBuffer = 0;
}