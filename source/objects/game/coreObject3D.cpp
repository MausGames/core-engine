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

    // check for model status
    if(!m_pModel.IsUsable()) return false;

    // enable the shader-program
    if(!pProgram.IsUsable()) return false;
    if(!pProgram->Enable())  return false;

    // update all object uniforms
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_POSITION, m_vPosition);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_SIZE,     m_vSize);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_ROTATION, m_vRotation);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,       m_vColor);
    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXPARAM,    coreVector4(m_vTexSize, m_vTexOffset));

    // enable all active textures
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i)
        if(m_apTexture[i].IsUsable()) m_apTexture[i]->Enable(i);

    // enable the model
    m_pModel->Enable();
    return true;
}

bool coreObject3D::Enable()
{
    // enable default shader-program
    return coreObject3D::Enable(m_pProgram);
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

void coreObject3D::Render()
{
    // render with default shader-program
    coreObject3D::Render(m_pProgram);
}


// ****************************************************************
/* move the 3d-object */
void coreObject3D::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // check current update status
    if(m_iUpdate & CORE_OBJECT_UPDATE_ALL)
    {
        // update rotation quaternion
        m_vRotation = coreMatrix4::Orientation(m_vDirection, m_vOrientation).m123().Quat();

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
: m_iCurCapacity  (iStartCapacity)
, m_iCurEnabled   (0)
, m_aiVertexArray (0)
, m_bUpdate       (false)
{
    ASSERT(iStartCapacity)

    // reserve memory for objects
    m_apObjectList.reserve(iStartCapacity);

    // create vertex array objects and instance data buffers
    this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
/* destructor */
coreBatchList::~coreBatchList()
{
    // clear memory
    m_apObjectList.clear();

    // delete vertex array objects and instance data buffers
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
void coreBatchList::Render(const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramSingle)
{
    if(m_apObjectList.empty()) return;

    if(this->IsInstanced())
    {
        // get first object from list
        const coreObject3D* pFirst = m_apObjectList.front();
        const coreModelPtr& pModel = pFirst->GetModel();

        // check for model status
        if(!pModel.IsUsable()) return;

        // enable the shader-program
        if(!pProgramInstanced.IsUsable()) return;
        if(!pProgramInstanced->Enable())  return;

        // enable all active textures
        for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i)
            if(pFirst->GetTexture(i).IsUsable()) pFirst->GetTexture(i)->Enable(i);

        if(m_bUpdate)
        {
            // switch to next available array and buffer
            m_aiVertexArray.Next();
            m_aiInstanceBuffer.Next();

            // map required area of the instance data buffer
            coreByte* pRange  = m_aiInstanceBuffer.GetCur().Map<coreByte>(0, m_iCurEnabled * CORE_OBJECT3D_INSTANCE_SIZE, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);
            coreByte* pCursor = pRange;

            FOR_EACH(it, m_apObjectList)
            {
                const coreObject3D* pObject = (*it);

                // render only enabled objects
                if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
                {
                    // compress data
                    const coreUint    iColor     = pObject->GetColor4().PackUnorm4x8();
                    const coreVector4 vTexParams = coreVector4(pObject->GetTexSize(), pObject->GetTexOffset());
                    ASSERT(pObject->GetColor4().Min() >= 0.0f && pObject->GetColor4().Max() <= 1.0f)

                    // write data to the buffer
                    std::memcpy(pCursor,                                       &pObject->GetPosition(), sizeof(coreVector3));
                    std::memcpy(pCursor +  3*sizeof(float),                    &pObject->GetSize(),     sizeof(coreVector3));
                    std::memcpy(pCursor +  6*sizeof(float),                    &pObject->GetRotation(), sizeof(coreVector4));
                    std::memcpy(pCursor + 10*sizeof(float),                    &iColor,                 sizeof(coreUint));
                    std::memcpy(pCursor + 10*sizeof(float) + sizeof(coreUint), &vTexParams,             sizeof(coreVector4));
                    pCursor += CORE_OBJECT3D_INSTANCE_SIZE;
                }
            }

            // unmap buffer
            m_aiInstanceBuffer.GetCur().Unmap(pRange);

            // reset the update status
            m_bUpdate = false;
        }

        // bind vertex array object
        if(m_aiVertexArray.GetCur()) glBindVertexArray(m_aiVertexArray.GetCur());
        else
        {
            // create vertex array object
            glGenVertexArrays(1, &m_aiVertexArray.GetCur());
            glBindVertexArray(m_aiVertexArray.GetCur());

            // set vertex data
            pModel->GetVertexBuffer(0)->Activate(0);
            m_aiInstanceBuffer.GetCur().Activate(1);

            // set index data
            if(*pModel->GetIndexBuffer())
            {
                coreDataBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER, false);
                pModel->GetIndexBuffer()->Bind();
            }

            // enable vertex attribute array division
            if(CORE_GL_SUPPORT(ARB_vertex_attrib_binding)) glVertexBindingDivisor(1, 1);
            else
            {
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 1);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,     1);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM, 1);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    1);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM, 1);
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
            (*it)->Render(pProgramSingle);
    }
}

void coreBatchList::Render()
{
    // render with default shader-programs
    if(!m_apObjectList.empty()) coreBatchList::Render(m_pProgram, m_apObjectList.front()->GetProgram());
}


// ****************************************************************
/* move the batch list normally */
void coreBatchList::MoveNormal()
{
    // reset render-count
    m_iCurEnabled = 0;

    FOR_EACH(it, m_apObjectList)
    {
        coreObject3D* pObject = (*it);

        // increase render-count
        if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
            ++m_iCurEnabled;

        // move object
        pObject->Move();
    }

    // set the update status
    m_bUpdate = true;
}


// ****************************************************************
/* move the batch list with automatic temporal sort */
void coreBatchList::MoveSort()
{
    auto et = m_apObjectList.begin();

    // reset render-count
    m_iCurEnabled = 0;

    // compare first object with nothing (never true)
    float fOldDistance    = 0.0f;
    bool  bOldTransparent = false;

    FOR_EACH(it, m_apObjectList)
    {
        coreObject3D* pObject = (*it);

        // increase render-count
        if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
            ++m_iCurEnabled;

        // move only enabled objects
        if(pObject->IsEnabled(CORE_OBJECT_ENABLE_MOVE))
        {
            // move object
            pObject->Move();

            // calculate properties of current object
            const float fCurDistance    = (pObject->GetPosition() - Core::Graphics->GetCamPosition()).LengthSq();
            const bool  bCurTransparent = (pObject->GetAlpha() < 1.0f) ? true : false;

            // sort objects (opaque first and from front to back, transparent later and from back to front)
            if(( bCurTransparent && (bOldTransparent && (fCurDistance > fOldDistance))) ||
               (!bCurTransparent && (bOldTransparent || (fCurDistance < fOldDistance))))
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
    }

    // set the update status
    m_bUpdate = true;
}


// ****************************************************************
/* bind 3d-object */
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
        this->Reallocate(m_iCurCapacity + m_iCurCapacity / 2 + 1);
    }

    // add object to list
    m_apObjectList.push_back(pObject);
}


// ****************************************************************
/* unbind 3d-object */
void coreBatchList::UnbindObject(coreObject3D* pObject)
{
    // find object in list
    FOR_EACH(it, m_apObjectList)
    {
        if((*it) == pObject)
        {
            // remove object from list
            m_apObjectList.erase(it);

            // clear on the last entry
            if(m_apObjectList.empty()) this->Clear();

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
    WARN_IF(iNewCapacity <  m_apObjectList.size()) return;

    // change current capacity
    m_iCurCapacity = iNewCapacity;
    m_apObjectList.reserve(iNewCapacity);

    // reallocate the instance data buffers
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
    this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
/* remove all 3d-objects */
void coreBatchList::Clear()
{
    // clear memory
    m_apObjectList.clear();

    // delete vertex array objects
    if(m_aiVertexArray[0]) glDeleteVertexArrays(CORE_OBJECT3D_INSTANCE_BUFFERS, m_aiVertexArray);
    m_aiVertexArray.List()->fill(0);
}


// ****************************************************************
/* reset with the resource manager */
void coreBatchList::__Reset(const coreResourceReset& bInit)
{
    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_uniform_buffer_object) || !CORE_GL_SUPPORT(ARB_vertex_array_object)) return;

    if(bInit)
    {
        // only allocate with enough capacity
        if(m_iCurCapacity >= CORE_OBJECT3D_INSTANCE_THRESHOLD)
        {
            FOR_EACH(it, *m_aiInstanceBuffer.List())
            {
                // create instance data buffers
                it->Create(m_iCurCapacity, CORE_OBJECT3D_INSTANCE_SIZE, NULL, CORE_DATABUFFER_STORAGE_PERSISTENT | CORE_DATABUFFER_STORAGE_FENCED);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 3, GL_FLOAT,          0);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,     3, GL_FLOAT,          3*sizeof(float));
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM, 4, GL_FLOAT,          6*sizeof(float));
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    4, GL_UNSIGNED_BYTE, 10*sizeof(float));
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM, 4, GL_FLOAT,         10*sizeof(float) + 1*sizeof(coreUint));
            }

            // invoke buffer update
            m_bUpdate = true;
        }
    }
    else
    {
        // delete vertex array objects
        if(m_aiVertexArray[0]) glDeleteVertexArrays(CORE_OBJECT3D_INSTANCE_BUFFERS, m_aiVertexArray);
        m_aiVertexArray.List()->fill(0);

        // delete instance data buffers
        FOR_EACH(it, *m_aiInstanceBuffer.List())
            it->Delete();
    }
}