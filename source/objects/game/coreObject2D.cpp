//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreModel* coreObject2D::s_pModel = NULL;


// ****************************************************************
// undefine the visual appearance
void coreObject2D::Undefine()
{
    // reset all resource and memory pointers
    for(int i = 0; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
}


// ****************************************************************
// render the 2d-object
void coreObject2D::Render(const coreProgramPtr& pProgram)
{
    if(!(m_iEnable & CORE_OBJECT_ENABLE_RENDER)) return;

    // enable the shader-program
    if(!pProgram.IsUsable()) return;
    if(!pProgram->Enable())  return;

    // calculate screen-view matrix
    const coreMatrix3 mScreenView = m_mTransform.m124() * Core::Graphics->GetOrtho().m124();

    // update all object uniforms
    pProgram->SendUniform(CORE_SHADER_UNIFORM_2D_SCREENVIEW, mScreenView, false);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,         m_vColor);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXPARAM,      coreVector4(m_vTexSize, m_vTexOffset));

    // enable all active textures
    for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
        if(m_apTexture[i].IsUsable()) m_apTexture[i]->Enable(i);

    // draw the model
    s_pModel->Enable();
    s_pModel->DrawArrays();
}


// ****************************************************************
// move the 2d-object
void coreObject2D::Move()
{
    if(!(m_iEnable & CORE_OBJECT_ENABLE_MOVE)) return;

    // check current update status
    if(m_iUpdate & CORE_OBJECT_UPDATE_TRANSFORM)
    {
        if(m_iUpdate & CORE_OBJECT_UPDATE_ALL)
        {
            // update rotation matrix
            m_mRotation = coreMatrix4::RotationZ(m_vDirection);
        }

        // calculate resolution-modified transformation parameters
        const coreVector2& vResolution     = Core::System->GetResolution();
        const coreVector2  vScreenPosition = (m_vPosition + 0.5f*m_vSize*m_vAlignment) * vResolution.y + m_vCenter * vResolution;
        const coreVector2  vScreenSize     = m_vSize*vResolution.y;

        // update transformation matrix
        m_mTransform = m_mRotation;
        m_mTransform._11 *= vScreenSize.x;     m_mTransform._12 *= vScreenSize.x;
        m_mTransform._21 *= vScreenSize.y;     m_mTransform._22 *= vScreenSize.y;
        m_mTransform._41  = vScreenPosition.x; m_mTransform._42  = vScreenPosition.y;

        // reset the update status
        m_iUpdate = CORE_OBJECT_UPDATE_NOTHING;
    }
}


// ****************************************************************
// interact with the 2d-object
// TODO: add interaction for rotated objects (ABS is for 180 degrees)
// TODO: Interact depends on Move, and Move of some menu objects depend on Interact
void coreObject2D::Interact()
{
    // get resolution-modified transformation parameters
    const coreVector2 vScreenPosition = coreVector2(    m_mTransform._41,      m_mTransform._42);
    const coreVector2 vScreenSize     = coreVector2(ABS(m_mTransform._11), ABS(m_mTransform._22)) * 0.5f * m_fFocusRange;

#if defined(_CORE_ANDROID_)

    // reset interaction status
    m_bFocused = false;
    m_iFinger  = 0;

    Core::Input->ForEachFinger(CORE_INPUT_HOLD, [&](const coreUint& i)
    {
        // get relative finger position
        const coreVector2 vInput = Core::Input->GetTouchPosition(i) * Core::System->GetResolution() - vScreenPosition;

        // test for intersection
        if(ABS(vInput.x) < vScreenSize.x &&
           ABS(vInput.y) < vScreenSize.y)
        {
            m_bFocused = true;
            BIT_SET(m_iFinger, i)
        }
    });

#else

    // get relative mouse cursor position
    const coreVector2 vInput = Core::Input->GetMousePosition() * Core::System->GetResolution() - vScreenPosition;

    // test for intersection
    m_bFocused = (ABS(vInput.x) < vScreenSize.x &&
                  ABS(vInput.y) < vScreenSize.y);

#endif
}


// ****************************************************************
// check for direct input
// TODO: make right mouse button on Android a longer push ?
// TODO: consider finger number
bool coreObject2D::IsClicked(const coreByte iButton, const coreInputType iType)const
{
#if defined(_CORE_ANDROID_)

    // check for general intersection status
    if(m_bFocused)
    {
        for(coreUint i = 0; i < CORE_INPUT_FINGERS; ++i)
        {
            // check for every finger on the object
            if((m_iFinger & BIT(i)) && Core::Input->GetTouchButton(i, iType))
                return true;
        }
    }
    return false;

#else

    // check for interaction status and mouse button
    return (m_bFocused && Core::Input->GetMouseButton(iButton, iType)) ? true : false;

#endif
}