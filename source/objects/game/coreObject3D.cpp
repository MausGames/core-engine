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
// define model through resource file
const coreModelPtr& coreObject3D::DefineModelFile(const char* pcPath)
{
    // set and return model object
    m_pModel = Core::Manager::Resource->LoadFile<coreModel>(pcPath);
    return m_pModel;
}


// ****************************************************************
// define model through linked resource
const coreModelPtr& coreObject3D::DefineModelLink(const char* pcName)
{
    // set and return model object
    m_pModel = Core::Manager::Resource->LoadLink<coreModel>(pcName);
    return m_pModel;
}


// ****************************************************************
// undefine the visual appearance
void coreObject3D::Undefine()
{
    // reset all resource and memory pointer
    for(int i = 0; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
    m_pModel   = NULL;
}


// ****************************************************************
// render the 3d-object
// TODO: integrated matrix transpose isn't possible on OpenGL ES 2.0 (?)
void coreObject3D::Render(const coreProgramShr& pProgram, const bool& bTextured)
{
    // enable the shader-program
    if(!pProgram) return;
    if(!pProgram->Enable()) return;

    // calculate model-view matrices
    const coreMatrix4 mModelView     = m_mTransform * Core::Graphics->GetCamera();
    const coreMatrix4 mModelViewProj = mModelView   * Core::Graphics->GetPerspective();

    // update all object uniforms
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_MODELVIEW,     mModelView,                    false);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_MODELVIEWPROJ, mModelViewProj,                false);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_NORMAL,        m_mRotation.Inverted().m123(), true);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,            m_vColor);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXSIZE,          m_vTexSize);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXOFFSET,        m_vTexOffset);

    if(bTextured)
    {
        // enable all active textures
        for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
            if(m_apTexture[i].IsActive()) m_apTexture[i]->Enable(i);
    }
    else coreTexture::DisableAll();

    if(m_pModel.IsLoaded())
    {
        // draw the model
        m_pModel->Enable();
        m_pModel->DrawElements();
    }
}


// ****************************************************************
// move the 3d-object
void coreObject3D::Move()
{
    if(m_iUpdate & 1)
    {
        if(m_iUpdate & 2)
        {
            // update rotation matrix
            m_mRotation = coreMatrix4::Orientation(m_vDirection, m_vOrientation);
        }

        // update transformation matrix
        m_mTransform = coreMatrix4::Scaling(m_vSize) * m_mRotation *
                       coreMatrix4::Translation(m_vPosition);

        // reset the update status
        m_iUpdate = 0;
    }
}