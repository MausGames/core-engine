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
// separately enable all resources for rendering
bool coreObject3D::Enable(const coreProgramShr& pProgram)
{
    // enable the shader-program
    if(!pProgram) return false;
    if(!pProgram->Enable()) return false;

    // calculate model-view matrices
    const coreMatrix4 mModelView     = m_mTransform * Core::Graphics->GetCamera();
    const coreMatrix4 mModelViewProj = mModelView   * Core::Graphics->GetPerspective();

    // update all object uniforms
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_MODELVIEW,     mModelView,     false);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_MODELVIEWPROJ, mModelViewProj, false);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,            m_vColor);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXSIZE,          m_vTexSize);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXOFFSET,        m_vTexOffset);

    // update normal matrix uniform
#if defined(_CORE_GLES_)
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_NORMAL, m_mRotation.m123().Invert().Transpose(), false);
#else
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_NORMAL, m_mRotation.m123().Invert(), true);
#endif

    // enable all active textures
    for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
        if(m_apTexture[i].IsActive()) m_apTexture[i]->Enable(i);

    // enable the model
    if(!m_pModel.IsLoaded()) return false;
    m_pModel->Enable();

    return true;
}


// ****************************************************************
// render the 3d-object
void coreObject3D::Render(const coreProgramShr& pProgram)
{
    // enable all resources
    if(this->Enable(pProgram))
    {
        // draw the model
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
        m_mTransform = m_mRotation;
        m_mTransform._11 *= m_vSize.x;     m_mTransform._12 *= m_vSize.x;     m_mTransform._13 *= m_vSize.x;
        m_mTransform._21 *= m_vSize.y;     m_mTransform._22 *= m_vSize.y;     m_mTransform._23 *= m_vSize.y;
        m_mTransform._31 *= m_vSize.z;     m_mTransform._32 *= m_vSize.z;     m_mTransform._33 *= m_vSize.z;
        m_mTransform._41  = m_vPosition.x; m_mTransform._42  = m_vPosition.y; m_mTransform._43  = m_vPosition.z;

        // reset the update status
        m_iUpdate = 0;
    }
}


// ****************************************************************
// handle collision between two 3d-objects
bool coreObject3D::Collision(const coreObject3D& Object1, const coreObject3D& Object2)
{
    // get bounding spheres
    const float fRadius1     = Object1.GetModel()->GetRadius() * Object1.GetCollisionRange() * Object1.GetSize().Max();
    const float fRadius2     = Object2.GetModel()->GetRadius() * Object2.GetCollisionRange() * Object2.GetSize().Max();
    const float fMaxDistance = fRadius1 + fRadius2;

    // calculate distance between both objects
    const coreVector3 vDiff = Object2.GetPosition() - Object1.GetPosition();

    // check for intersection
    return (vDiff.LengthSq() <= fMaxDistance * fMaxDistance) ? true : false;
}


// ****************************************************************
// handle collision between 3d-object and line
// TODO: remove SQRT somehow
float coreObject3D::Collision(const coreObject3D& Object, const coreVector3& vLinePos, const coreVector3& vLineDir)
{
    SDL_assert(vLineDir.LengthSq() == 1.0f);

    // get bounding sphere
    const float fRadius = Object.GetModel()->GetRadius() * Object.GetCollisionRange() * Object.GetSize().Max();

    // calculate distance between both objects
    const coreVector3 vDiff = Object.GetPosition() - vLinePos;

    // calculate range parameters
    const float fAdjacent   = coreVector3::Dot(vDiff, vLineDir);
    const float fOppositeSq = vDiff.LengthSq() - fAdjacent * fAdjacent;
    const float fRadiusSq   = fRadius * fRadius;

    // check for intersection (return distance from line position to intersection point on success)
    return (fOppositeSq <= fRadiusSq) ? (fAdjacent - SQRT(fRadiusSq - fOppositeSq)) : 0.0f;
}