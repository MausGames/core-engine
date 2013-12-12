//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_FRAMEBUFFER_H_
#define _CORE_GUARD_FRAMEBUFFER_H_


// ****************************************************************
// frame buffer definitions
enum coreFramebufferType
{
    CORE_FRAMEBUFFER_COLOR = 0x0001,   //!< color writing without depth test
    CORE_FRAMEBUFFER_DEPTH = 0x0002,   //!< depth writing
    CORE_FRAMEBUFFER_FULL  = 0x0004,   //!< full frame buffer with color and depth writing
    CORE_FRAMEBUFFER_ALPHA = 0x0100,   //!< add alpha channel (only COLOR and FULL)
};


// ****************************************************************
// frame buffer class
// TODO: mipmapping ?
// TODO: implement real multisampling (currently supersampling sufficient)
// TODO: frame buffer stack to allow nested buffer wrtings ? (GL 2.0 problems)
// TODO: add stencil support
class coreFrameBuffer final : public coreReset
{
private:
    GLuint m_iFrameBuffer;                //!< frame buffer identifier/OpenGL name

    coreTexturePtr m_pTexture;            //!< render target texture
    GLuint m_iDepthBuffer;                //!< depth component buffer

    coreVector2 m_vResolution;            //!< resolution of the frame buffer
    int m_iType;                          //!< type of the frame buffer

    static coreFrameBuffer* s_pCurrent;   //!< currently active frame buffer object


public:
    coreFrameBuffer(const coreVector2& vResolution, const int& iType, const char* pcLink);
    ~coreFrameBuffer();

    //! write to the frame buffer
    //! @{
    void StartWrite();
    void EndWrite();
    //! @}

    //! clear content of the frame buffer
    //! @{
    void Clear();
    //! @}

    //! get object attributes
    //! @{
    inline const GLuint& GetFrameBuffer()const     {return m_iFrameBuffer;}
    inline const coreTexturePtr& GetTexture()const {return m_pTexture;}
    inline const GLuint& GetDepthBuffer()const     {return m_iDepthBuffer;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    inline const int& GetType()const               {return m_iType;}
    //! @}

    //! get currently active frame buffer object
    //! @{
    static inline coreFrameBuffer* GetCurrent() {return s_pCurrent;}
    //! @}


private:
    DISABLE_COPY(coreFrameBuffer)

    //! reset with the resource manager
    //! @{
    void __Reset(const bool& bInit)override;
    //! @}

    //! init and exit the frame buffer
    //! @{
    coreError __Init();
    coreError __Exit();
    //! @}

    //! delete frame and depth buffer
    //! @{
    void __DeleteBuffers();
    //! @}
};


#endif // _CORE_GUARD_FRAMEBUFFER_H_