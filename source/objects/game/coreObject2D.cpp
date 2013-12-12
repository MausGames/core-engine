//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// undefine the visual appearance
void coreObject2D::Undefine()
{
    // reset all resource and memory pointer
    for(int i = 0; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
}


// ****************************************************************
// render the 2d-object
void coreObject2D::Render(const coreProgramShr& pProgram, const bool& bTextured)
{
    // enable the shader-program
    if(!pProgram) return;
    if(!pProgram->Enable()) return;

    // calculate screen-view matrix
    const coreMatrix4 mScreenView = m_mTransform * Core::Graphics->GetOrtho();

    // update all object uniforms
    pProgram->SetUniform(CORE_SHADER_UNIFORM_2D_SCREENVIEW, mScreenView.m124(), false);
    pProgram->SetUniform(CORE_SHADER_UNIFORM_COLOR,         m_vColor);
    pProgram->SetUniform(CORE_SHADER_UNIFORM_TEXSIZE,       m_vTexSize);
    pProgram->SetUniform(CORE_SHADER_UNIFORM_TEXOFFSET,     m_vTexOffset);

    if(bTextured)
    {
        // enable all active textures
        for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
            if(m_apTexture[i].IsActive()) m_apTexture[i]->Enable(i);
    }
    else coreTexture::DisableAll();

    // render the model
    coreModel::StandardPlane()->RenderStrip();
}


// ****************************************************************
// move the 2d-object
void coreObject2D::Move()
{
    if(m_iUpdate & 1)
    {
        if(m_iUpdate & 2)
        {
            // update rotation matrix
            m_mRotation = coreMatrix4::RotationZ(m_vDirection);
        }

        // calculate resolution-modified transformation parameters
        const coreVector2& vResolution     = Core::System->GetResolution();
        const coreVector2  vScreenPosition = (m_vPosition + 0.5f*m_vSize*m_vAlignment)*vResolution.x + m_vCenter*vResolution;
        const coreVector2  vScreenSize     = m_vSize*vResolution.x;

        // update transformation matrix
        m_mTransform = coreMatrix4::Scaling(coreVector3(vScreenSize, 0.0f)) * m_mRotation *
                       coreMatrix4::Translation(coreVector3(vScreenPosition, 0.0f));

        // reset the update status
        m_iUpdate = 0;
    }
}


// ****************************************************************
// interact with the 2d-object
void coreObject2D::Interact()
{
    // get resolution-modified transformation parameters
    const coreVector2 vScreenPosition = coreVector2(m_mTransform._41, m_mTransform._42);
    const coreVector2 vScreenSize     = coreVector2(m_mTransform._11, m_mTransform._22)*0.5f;

    // get relative mouse cursor position
    const coreVector2 vInput = Core::Input->GetMousePosition() * Core::System->GetResolution() - vScreenPosition;

    // test for intersection
    m_bFocused = (ABS(vInput.x) < vScreenSize.x &&
                  ABS(vInput.y) < vScreenSize.y);
}