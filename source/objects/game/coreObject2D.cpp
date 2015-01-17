//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// undefine the visual appearance
void coreObject2D::Undefine()
{
    // reset all resource and memory pointers
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
}


// ****************************************************************
// render the 2d-object
void coreObject2D::Render(const coreProgramPtr& pProgram)
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return;

    // enable the shader-program
    if(!pProgram.IsUsable()) return;
    if(!pProgram->Enable())  return;

    // update all object uniforms
    pProgram->SendUniform(CORE_SHADER_UNIFORM_2D_SCREENVIEW, m_mTransform * Core::Graphics->GetOrtho().m124(), false);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,         m_vColor);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXPARAM,      coreVector4(m_vTexSize, m_vTexOffset));

    // enable all active textures
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i)
        if(m_apTexture[i].IsUsable()) m_apTexture[i]->Enable(i);

    // draw the model
    Core::Manager::Object->GetLowModel()->Enable();
    Core::Manager::Object->GetLowModel()->DrawArrays();
}

void coreObject2D::Render()
{
    // render with default shader-program
    coreObject2D::Render(m_pProgram);
}


// ****************************************************************
// move the 2d-object
void coreObject2D::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // check current update status
    if(m_iUpdate & CORE_OBJECT_UPDATE_TRANSFORM)
    {
        if(m_iUpdate & CORE_OBJECT_UPDATE_ALL)
        {
            // update rotation matrix
            m_mRotation = coreMatrix3::Rotation(m_vDirection);
        }

        // calculate resolution-modified transformation parameters
        const coreVector2& vResolution     = Core::System->GetResolution();
        const coreVector2  vScreenPosition = (m_vPosition + 0.5f*m_vSize*m_vAlignment) * vResolution.Min() + m_vCenter * vResolution;
        const coreVector2  vScreenSize     = m_vSize * vResolution.Min();

        // update transformation matrix
        m_mTransform = m_mRotation;
        m_mTransform._11 *= vScreenSize.x;     m_mTransform._12 *= vScreenSize.x;
        m_mTransform._21 *= vScreenSize.y;     m_mTransform._22 *= vScreenSize.y;
        m_mTransform._31  = vScreenPosition.x; m_mTransform._32  = vScreenPosition.y;

        // reset the update status
        m_iUpdate = CORE_OBJECT_UPDATE_NOTHING;
    }
}


// ****************************************************************
// interact with the 2d-object
void coreObject2D::Interact()
{
    // get resolution-modified transformation parameters
    const coreVector2 vScreenPosition = coreVector2(    m_mTransform._31,      m_mTransform._32);
    const coreVector2 vScreenSize     = coreVector2(ABS(m_mTransform._11), ABS(m_mTransform._22)) * 0.5f * m_vFocusModifier;

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