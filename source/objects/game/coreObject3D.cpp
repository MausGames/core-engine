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
/* undefine the visual appearance */
void coreObject3D::Undefine()
{
    // reset all resource and memory pointers
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
    m_pModel   = NULL;
}


// ****************************************************************
/* separately enable all resources for rendering */
bool coreObject3D::Enable(const coreProgramPtr& pProgram)
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return false;

    // enable the shader-program
    if(!pProgram.IsUsable()) return false;
    if(!pProgram->Enable())  return false;

    // check for model status
    if(!m_pModel.IsUsable()) return false;

    // calculate model-view matrices
    const coreMatrix4 mModelView     = m_mTransform * Core::Graphics->GetCamera();
    const coreMatrix4 mModelViewProj = mModelView   * Core::Graphics->GetPerspective();

    // update all object uniforms
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_MODELVIEW,     mModelView,     false);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_MODELVIEWPROJ, mModelViewProj, false);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,            m_vColor);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXPARAM,         coreVector4(m_vTexSize, m_vTexOffset));

    // update normal matrix uniform
#if defined(_CORE_GLES_)
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_NORMAL, m_mRotation.m123().Invert().Transpose(), false);
#else
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_NORMAL, m_mRotation.m123().Invert(), true);
#endif

    // enable all active textures
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i)
        if(m_apTexture[i].IsUsable()) m_apTexture[i]->Enable(i);

    // enable the model
    m_pModel->Enable();
    return true;
}


// ****************************************************************
/* render the 3d-object */
void coreObject3D::Render(const coreProgramPtr& pProgram)
{
    // enable all resources
    if(this->Enable(pProgram))
    {
        // draw the model
        m_pModel->Draw();
    }
}


// ****************************************************************
/* move the 3d-object */
void coreObject3D::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // check current update status
    if(m_iUpdate & CORE_OBJECT_UPDATE_TRANSFORM)
    {
        if(m_iUpdate & CORE_OBJECT_UPDATE_ALL)
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
        m_iUpdate = CORE_OBJECT_UPDATE_NOTHING;
    }
}


// ****************************************************************
/* handle collision between two 3d-objects */
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
/* handle collision between 3d-object and line */
float coreObject3D::Collision(const coreObject3D& Object, const coreVector3& vLinePos, const coreVector3& vLineDir)
{
    ASSERT(vLineDir.IsNormalized())

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


// ****************************************************************
/* constructor */
coreBatchList::coreBatchList(const coreUint& iStartCapacity)noexcept
: m_iCurCapacity (iStartCapacity)
, m_iCurEnabled  (0)
, m_iVertexArray (0)
, m_bUpdate      (false)
{
    ASSERT(iStartCapacity)

    // reserve memory for objects
    m_apObjectList.reserve(iStartCapacity);

    // create vertex array object and instance data buffer
    this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
/* destructor */
coreBatchList::~coreBatchList()
{
    // clear memory
    m_apObjectList.clear();

    // delete vertex array object and instance data buffer
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
}


// ****************************************************************
/* undefine the visual appearance */
void coreBatchList::Undefine()
{
    // reset all resource and memory pointers
    m_pProgram = NULL;
}


// ****************************************************************
/* render the batch list */
void coreBatchList::Render(const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramNormal)
{
    if(m_apObjectList.empty()) return;

    if(m_iInstanceBuffer && m_apObjectList.size() >= CORE_OBJECT3D_INSTANCE_THRESHOLD)
    {
        // enable the shader-program
        if(!pProgramInstanced.IsUsable()) return;
        if(!pProgramInstanced->Enable())  return;

        // get first object from list
        const coreObject3D* pFirst = m_apObjectList.front();
        const coreModelPtr& pModel = pFirst->GetModel();

        // check for model status
        if(!pModel.IsUsable()) return;

        // enable all active textures
        for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i)
            if(pFirst->GetTexture(i).IsUsable()) pFirst->GetTexture(i)->Enable(i);

        if(m_bUpdate)
        {
            // reset current number of enabled 3d-objects
            m_iCurEnabled = 0;

            // map required area of the instance data buffer
            const coreUint iLength = m_apObjectList.size() * CORE_OBJECT3D_INSTANCE_SIZE;
            coreByte*      pRange  = m_iInstanceBuffer.Map<coreByte>(0, iLength, CORE_DATABUFFER_MAP_INVALIDATE_ALL);

            FOR_EACH(it, m_apObjectList)
            {
                const coreObject3D* pObject = (*it);

                // render only enabled 3d-objects
                if(!pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) continue;
                ++m_iCurEnabled;

                // calculate and compress data
                const coreMatrix4 mModelView = pObject->GetTransform() * Core::Graphics->GetCamera();
                const coreUint    iColor     = pObject->GetColor4().ColorPack();
                const coreVector4 vTexParams = coreVector4(pObject->GetTexSize(), pObject->GetTexOffset());

                // write data to the buffer
                std::memcpy(pRange,                                          &mModelView, sizeof(coreMatrix4));
                std::memcpy(pRange + sizeof(coreMatrix4),                    &iColor,     sizeof(coreUint));
                std::memcpy(pRange + sizeof(coreMatrix4) + sizeof(coreUint), &vTexParams, sizeof(coreVector4));
                pRange += CORE_OBJECT3D_INSTANCE_SIZE;
            }

            // unmap buffer
            m_iInstanceBuffer.Unmap(pRange - m_iCurEnabled * CORE_OBJECT3D_INSTANCE_SIZE);

            // reset the update status
            m_bUpdate = false;
        }

        // bind vertex array object
        if(m_iVertexArray) glBindVertexArray(m_iVertexArray);
        else
        {
            // create vertex array object
            glGenVertexArrays(1, &m_iVertexArray);
            glBindVertexArray(m_iVertexArray);

            // set vertex data
            pModel->GetVertexBuffer(0)->Activate(0);
            m_iInstanceBuffer.Activate(1);

            // set index data
            if(pModel->GetIndexBuffer())
                pModel->GetIndexBuffer()->Bind();

            // enable vertex attribute array division
            if(GLEW_ARB_vertex_attrib_binding) glVertexBindingDivisor(1, 1);
            else
            {
                glVertexAttribDivisorARB(CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW_NUM+0, 1);
                glVertexAttribDivisorARB(CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW_NUM+1, 1);
                glVertexAttribDivisorARB(CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW_NUM+2, 1);
                glVertexAttribDivisorARB(CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW_NUM+3, 1);
                glVertexAttribDivisorARB(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,       1);
                glVertexAttribDivisorARB(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM,    1);
            }
        }

        // disable current model object (because of direct VAO use)
        coreModel::Disable(false);

        // draw the model instanced
        pModel->DrawInstanced(m_iCurEnabled);
    }
    else
    {
        // draw without instancing
        FOR_EACH(it, m_apObjectList)
            (*it)->Render(pProgramNormal);
    }
}


// ****************************************************************
/* move the batch list */
void coreBatchList::Move()
{
    auto et = m_apObjectList.begin();

    // compare first object with nothing (never true)
    float fOldDistance    = 0.0f;
    bool  bOldTransparent = false;

    FOR_EACH(it, m_apObjectList)
    {
        coreObject3D* pObject = (*it);

        // move only enabled 3d-objects
        if(!pObject->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) continue;

        // move object
        pObject->Move();

        // calculate properties of current object
        const float fCurDistance    = (pObject->GetPosition() - Core::Graphics->GetCamPosition()).LengthSq();
        const bool  bCurTransparent =  pObject->GetAlpha() < 1.0f;
        
        // sort objects (opaque first and from front to back, transparent later and from back to front)
        if(( bCurTransparent && (bOldTransparent && fCurDistance > fOldDistance)) ||
           (!bCurTransparent && (bOldTransparent || fCurDistance < fOldDistance)))
        {
            std::swap(*it, *et);
        }
        else
        {
            // forward properties to next object
            fOldDistance    = fCurDistance;
            bOldTransparent = bCurTransparent;
        }

        // compare current object with next object
        et = it;
    }

    // set the update status
    m_bUpdate = true;
}


// ****************************************************************
/* bind 3d-objects */
void coreBatchList::BindObject(coreObject3D* pObject)
{
#if defined(_CORE_DEBUG_)

    // check for duplicate objects
    FOR_EACH(it, m_apObjectList)
        ASSERT((*it) != pObject)

#endif

    if(m_apObjectList.size() >= m_iCurCapacity)
    {
        // increase current capacity by 50%
        this->Reallocate(m_iCurCapacity + m_iCurCapacity / 2);
    }

    // add object to list
    m_apObjectList.push_back(pObject);
} 
  

// ****************************************************************
/* unbind 3d-objects */
void coreBatchList::UnbindObject(coreObject3D* pObject)
{
    // find object in list
    FOR_EACH(it, m_apObjectList)
    {
        if((*it) == pObject)
        {
            // remove object from list
            m_apObjectList.erase(it);
            return;
        }
    }

    // object not found
    ASSERT(false)
}


// ****************************************************************
/* change current capacity */
void coreBatchList::Reallocate(const coreUint& iNewCapacity)
{
           if(iNewCapacity == m_iCurCapacity)        return;
    ASSERT_IF(iNewCapacity <  m_apObjectList.size()) return;

    // change current capacity
    m_iCurCapacity = iNewCapacity;
    m_apObjectList.reserve(iNewCapacity);

    // reallocate the instance data buffer
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
    this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
/* reset with the resource manager */
void coreBatchList::__Reset(const coreResourceReset& bInit)
{
    // check for OpenGL extensions
    if(!GLEW_ARB_instanced_arrays || !GLEW_ARB_vertex_array_object) return;

    if(bInit)
    {
        // create instance data buffer
        m_iInstanceBuffer.Create(m_iCurCapacity, CORE_OBJECT3D_INSTANCE_SIZE, NULL, CORE_DATABUFFER_STORAGE_PERSISTENT);
        m_iInstanceBuffer.DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW_NUM+0, 4, GL_FLOAT,         0);
        m_iInstanceBuffer.DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW_NUM+1, 4, GL_FLOAT,         4*sizeof(float));
        m_iInstanceBuffer.DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW_NUM+2, 4, GL_FLOAT,         8*sizeof(float));
        m_iInstanceBuffer.DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW_NUM+3, 4, GL_FLOAT,        12*sizeof(float));
        m_iInstanceBuffer.DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,       1, GL_UNSIGNED_INT, 16*sizeof(float));
        m_iInstanceBuffer.DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM,    4, GL_FLOAT,        16*sizeof(float) + 1*sizeof(int));

        // invoke buffer update
        m_bUpdate = true;
    }
    else
    {
        // delete vertex array object
        if(m_iVertexArray) glDeleteVertexArrays(1, &m_iVertexArray);
        m_iVertexArray = 0;

        // delete instance data buffer
        m_iInstanceBuffer.Delete();
    }
}