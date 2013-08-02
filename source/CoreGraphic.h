#pragma once


// ****************************************************************
// main graphic interface
class CoreGraphic
{
private:
    SDL_GLContext m_Context;                   // OpenGL context
                                               
    float m_fFOV;                              // field-of-view
    float m_fNearClip;                         // near clipping plane
    float m_fFarClip;                          // far clipping plane
                                               
    coreVector3 m_vCamPosition;                // position of the camera
    coreVector3 m_vCamDirection;               // direction of the camera
    coreVector3 m_vCamOrientation;             // orientation of the camera
    coreMatrix m_mCamera;                      // camera matrix
                                               
    coreMatrix m_mPerspective;                 // perspective projection matrix
    coreMatrix m_mOrtho;                       // orthogonal projection matrix
    coreMatrix m_mCurProjection;               // current loaded projection matrix
    coreVector2 m_vCurResolution;              // current viewport resolution

    bool m_bOpenGL3;                           // support for OpenGL 3.0+
    std::map<std::string, bool> m_abFeature;   // support for hardware features


private:
    CoreGraphic();
    ~CoreGraphic();
    friend class Core;

    // update the graphic scene
    void __UpdateScene();


public:
    // control camera
    void SetCamera(const coreVector3* pvPosition, const coreVector3* pvDirection, const coreVector3* pvOrientation);
    inline void LoadCamera() {glLoadMatrixf(m_mCamera);}

    // control view and projection
    void ResizeView(coreVector2 vResolution);
    void EnablePerspective();
    void EnableOrtho();

    // create a screenshot
    void Screenshot(const char* pcPath);
    void Screenshot();

    // get attributes
    inline const SDL_GLContext& GetContext()const      {return m_Context;}
    inline const float& GetFOV()const                  {return m_fFOV;}
    inline const float& GetNearClip()const             {return m_fNearClip;}
    inline const float& GetFarClip()const              {return m_fFarClip;}
    inline const coreVector3& GetCamPosition()const    {return m_vCamPosition;}
    inline const coreVector3& GetCamDirection()const   {return m_vCamDirection;}
    inline const coreVector3& GetCamOrientation()const {return m_vCamOrientation;}
    inline const coreMatrix& GetCamera()const          {return m_mCamera;}
    inline const coreMatrix& GetPerspective()const     {return m_mPerspective;}
    inline const coreMatrix& GetOrtho()const           {return m_mOrtho;}

    // check hardware support
    inline const bool& SupportOpenGL3()const           {return m_bOpenGL3;}
    inline const bool& SupportFeature(char* acFeature) {if(!m_abFeature.count(acFeature)) m_abFeature[acFeature] = (glewIsSupported(acFeature) ? true : false); return m_abFeature.at(acFeature);}
};