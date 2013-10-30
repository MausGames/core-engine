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

    // update all object uniforms
    pProgram->SetUniform(CORE_SHADER_UNIFORM_TRANSFORM,  m_mTransform, false);
    pProgram->SetUniform(CORE_SHADER_UNIFORM_COLOR,      m_vColor);
    pProgram->SetUniform(CORE_SHADER_UNIFORM_TEX_SIZE,   m_vTexSize);
    pProgram->SetUniform(CORE_SHADER_UNIFORM_TEX_OFFSET, m_vTexOffset);

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
            m_mRotation = coreMatrix::RotationZ(m_vDirection);
        }

        // calculate resolution-modified transformation parameters
        const coreVector2& vResolution     = Core::System->GetResolution();
        const coreVector2  vScreenPosition = (m_vPosition + 0.5f*m_vSize*m_vAlignment)*vResolution.x + m_vCenter*vResolution;
        const coreVector2  vScreenSize     = m_vSize*vResolution.x;

        // update transformation matrix
        m_mTransform = coreMatrix::Scaling(coreVector3(vScreenSize, 0.0f)) * m_mRotation *
                       coreMatrix::Translation(coreVector3(vScreenPosition, 0.0f));

        // reset the update status
        m_iUpdate = 0;
    }
}
