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


// ****************************************************************
// frame buffer definitions
enum coreFrameBufferType
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
    GLuint m_iFrameBuffer;                //!< frame buffer identifier

    coreTexturePtr m_pTexture;            //!< render target texture
    GLuint m_iDepthBuffer;                //!< depth component buffer

    coreVector2 m_vResolution;            //!< resolution of the frame buffer
    int m_iType;                          //!< type of the frame buffer

    float m_fFOV;                         //!< field-of-view
    float m_fNearClip;                    //!< near clipping plane
    float m_fFarClip;                     //!< far clipping plane

    static coreFrameBuffer* s_pCurrent;   //!< currently active frame buffer object


public:
    coreFrameBuffer(const coreVector2& vResolution, const int& iType, const char* pcLink)noexcept;
    ~coreFrameBuffer();

    //! write to the frame buffer
    //! @{
    void StartWrite();
    void EndWrite();
    //! @}

    //! reset content of the frame buffer
    //! @{
    void Clear();
    void Invalidate();
    //! @}

    //! set object properties
    //! @{
    inline void SetFOV(const float& fFOV)           {m_fFOV      = fFOV;}
    inline void SetNearClip(const float& fNearClip) {m_fNearClip = fNearClip;}
    inline void SetFarClip(const float& fFarClip)   {m_fFarClip  = fFarClip;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint& GetFrameBuffer()const     {return m_iFrameBuffer;}
    inline const coreTexturePtr& GetTexture()const {return m_pTexture;}
    inline const GLuint& GetDepthBuffer()const     {return m_iDepthBuffer;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    inline const int& GetType()const               {return m_iType;}
    inline const float& GetFOV()const              {return m_fFOV;}
    inline const float& GetNearClip()const         {return m_fNearClip;}
    inline const float& GetFarClip()const          {return m_fFarClip;}
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