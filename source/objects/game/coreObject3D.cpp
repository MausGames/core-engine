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
/* destructor */
coreObject3D::~coreObject3D()
{
    // unbind from object manager
    this->ChangeType(0);
}


// ****************************************************************
/* assignment operations */
coreObject3D& coreObject3D::operator = (const coreObject3D& c)noexcept
{
    // bind to object manager
    this->ChangeType(c.m_iType);

    // copy remaining properties
    coreObject::operator = (c);
    m_vPosition          = c.m_vPosition;
    m_vSize              = c.m_vSize;
    m_vDirection         = c.m_vDirection;
    m_vOrientation       = c.m_vOrientation;
    m_pModel             = c.m_pModel;
    m_vRotation          = c.m_vRotation;
    m_vCollisionModifier = c.m_vCollisionModifier;
    m_vCollisionRange    = c.m_vCollisionRange;
    m_fCollisionRadius   = c.m_fCollisionRadius;

    return *this;
}

coreObject3D& coreObject3D::operator = (coreObject3D&& m)noexcept
{
    // bind to object manager
    this->ChangeType(m.m_iType);

    // move remaining properties
    coreObject::operator = (std::move(m));
    m_vPosition          = m.m_vPosition;
    m_vSize              = m.m_vSize;
    m_vDirection         = m.m_vDirection;
    m_vOrientation       = m.m_vOrientation;
    m_pModel             = std::move(m.m_pModel);
    m_vRotation          = m.m_vRotation;
    m_vCollisionModifier = m.m_vCollisionModifier;
    m_vCollisionRange    = m.m_vCollisionRange;
    m_fCollisionRadius   = m.m_fCollisionRadius;

    return *this;
}


// ****************************************************************
/* undefine the visual appearance */
void coreObject3D::Undefine()
{
    // reset all resource and memory pointers
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
    m_pModel   = NULL;
}


// ****************************************************************
/* separately enable all resources for rendering */
coreBool coreObject3D::Prepare(const coreProgramPtr& pProgram)
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
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i)
        if(m_apTexture[i].IsUsable()) m_apTexture[i]->Enable(i);

    // enable the model
    m_pModel->Enable();
    return true;
}

coreBool coreObject3D::Prepare()
{
    // enable default shader-program
    return coreObject3D::Prepare(m_pProgram);
}


// ****************************************************************
/* render the 3d-object */
void coreObject3D::Render(const coreProgramPtr& pProgram)
{
    // enable all resources
    if(this->Prepare(pProgram))
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
    if(m_iUpdate)
    {
        if(CONTAINS_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
        {
            // update rotation quaternion
            m_vRotation = coreMatrix4::Orientation(m_vDirection, m_vOrientation).m123().Quat();
        }
        if(CONTAINS_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION))
        {
            // cancel update without valid model
            if(!m_pModel.IsUsable()) return;

            // update collision range and radius
            m_vCollisionRange  = m_pModel->GetBoundingRange () * this->GetSize()       * m_vCollisionModifier;
            m_fCollisionRadius = m_pModel->GetBoundingRadius() * this->GetSize().Max() * m_vCollisionModifier.Max();
        }

        // reset the update status
        m_iUpdate = CORE_OBJECT_UPDATE_NOTHING;
    }
}


// ****************************************************************
/* change object type and manager registration */
void coreObject3D::ChangeType(const coreInt32& iType)
{
    if(m_iType == iType) return;

    // unbind from old type and bind to new type
    if(m_iType) Core::Manager::Object->__UnbindObject(this, m_iType);
    m_iType = iType;
    if(m_iType) Core::Manager::Object->__BindObject  (this, m_iType);
}


// ****************************************************************
/* constructor */
coreBatchList::coreBatchList(const coreUint32& iStartCapacity)noexcept
: m_iCurCapacity (iStartCapacity)
, m_iCurEnabled  (0u)
, m_bUpdate      (false)
{
    ASSERT(iStartCapacity)

    // reserve memory for objects
    m_apObjectList.reserve(iStartCapacity);

    // create vertex array objects and instance data buffers
    m_aiVertexArray.Fill(0u);
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
        for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i)
            if(pFirst->GetTexture(i).IsUsable()) pFirst->GetTexture(i)->Enable(i);

        if(m_bUpdate)
        {
            // switch to next available array and buffer
            m_aiVertexArray   .Next();
            m_aiInstanceBuffer.Next();

            // map required area of the instance data buffer
            coreByte* pRange  = m_aiInstanceBuffer.Current().Map<coreByte>(0u, m_iCurEnabled * CORE_OBJECT3D_INSTANCE_SIZE, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);
            coreByte* pCursor = pRange;

            FOR_EACH(it, m_apObjectList)
            {
                const coreObject3D* pObject = (*it);

                // render only enabled objects
                if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
                {
                    // compress data
                    const coreUint64 iSize      = coreVector4(pObject->GetSize(), 0.0f).PackFloat4x16();
                    const coreUint64 iRotation  = pObject->GetRotation().PackSnorm4x16();
                    const coreUint32 iColor     = pObject->GetColor4  ().PackUnorm4x8 ();
                    const coreUint64 iTexParams = coreVector4(pObject->GetTexSize(), pObject->GetTexOffset()).PackFloat4x16();
                    ASSERT(pObject->GetColor4().Min() >= 0.0f && pObject->GetColor4().Max() <= 1.0f)

                    // write data to the buffer
                    std::memcpy(pCursor,                                                &pObject->GetPosition(), sizeof(coreVector3));
                    std::memcpy(pCursor + 3u*sizeof(coreFloat),                         &iSize,                  sizeof(coreUint64));
                    std::memcpy(pCursor + 3u*sizeof(coreFloat) + 2u*sizeof(coreUint32), &iRotation,              sizeof(coreUint64));
                    std::memcpy(pCursor + 3u*sizeof(coreFloat) + 4u*sizeof(coreUint32), &iColor,                 sizeof(coreUint32));
                    std::memcpy(pCursor + 3u*sizeof(coreFloat) + 5u*sizeof(coreUint32), &iTexParams,             sizeof(coreUint64));
                    pCursor += CORE_OBJECT3D_INSTANCE_SIZE;
                }
            }

            // unmap buffer
            m_aiInstanceBuffer.Current().Unmap(pRange);

            // reset the update status
            m_bUpdate = false;
        }

        // bind vertex array object
        if(m_aiVertexArray.Current()) glBindVertexArray(m_aiVertexArray.Current());
        else
        {
            // create vertex array object
            glGenVertexArrays(1, &m_aiVertexArray.Current());
            glBindVertexArray(m_aiVertexArray.Current());

            // set vertex data
            pModel->GetVertexBuffer(0u)->Activate(0u);
            m_aiInstanceBuffer.Current().Activate(1u);

            // set index data
            if(*pModel->GetIndexBuffer())
            {
                coreDataBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER, false);
                pModel->GetIndexBuffer()->Bind();
            }

            // enable vertex attribute array division
            if(CORE_GL_SUPPORT(ARB_vertex_attrib_binding)) glVertexBindingDivisor(1u, 1u);
            else
            {
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 1u);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,     1u);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM, 1u);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    1u);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM, 1u);
            }
        }

        // disable current model object (because of direct VAO use)
        coreModel::Disable(false);

        // draw the model instanced
        pModel->DrawInstanced(m_iCurEnabled);
    }
    else
    {
        // draw without instancing (no inheritance)
        FOR_EACH(it, m_apObjectList)
            (*it)->coreObject3D::Render(pProgramSingle);
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
    m_iCurEnabled = 0u;

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
    m_iCurEnabled = 0u;

    // compare first object with nothing (never true)
    coreFloat fOldDistance    = 0.0f;
    coreBool  bOldTransparent = false;

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
            const coreFloat fCurDistance    = (pObject->GetPosition() - Core::Graphics->GetCamPosition()).LengthSq();
            const coreBool  bCurTransparent = (pObject->GetAlpha() < 1.0f) ? true : false;

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
        this->Reallocate(m_iCurCapacity + m_iCurCapacity / 2u + 1u);
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
    WARN_IF(true) {}
}


// ****************************************************************
/* change current capacity */
void coreBatchList::Reallocate(const coreUint32& iNewCapacity)
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
    m_aiVertexArray.Fill(0u);
}


// ****************************************************************
/* reset with the resource manager */
void coreBatchList::__Reset(const coreResourceReset& bInit)
{
    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_uniform_buffer_object) || !CORE_GL_SUPPORT(ARB_vertex_array_object) || !CORE_GL_SUPPORT(ARB_half_float_vertex)) return;

    if(bInit)
    {
        if(m_aiInstanceBuffer[0]) return;

        // only allocate with enough capacity
        if(m_iCurCapacity >= CORE_OBJECT3D_INSTANCE_THRESHOLD)
        {
            FOR_EACH(it, *m_aiInstanceBuffer.List())
            {
                // create instance data buffers
                it->Create(m_iCurCapacity, CORE_OBJECT3D_INSTANCE_SIZE, NULL, CORE_DATABUFFER_STORAGE_PERSISTENT | CORE_DATABUFFER_STORAGE_FENCED);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 3u, GL_FLOAT,         false, 0u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,     4u, GL_HALF_FLOAT,    false, 3u*sizeof(coreFloat));
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM, 4u, GL_SHORT,         false, 3u*sizeof(coreFloat) + 2u*sizeof(coreUint32));
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    4u, GL_UNSIGNED_BYTE, false, 3u*sizeof(coreFloat) + 4u*sizeof(coreUint32));
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM, 4u, GL_HALF_FLOAT,    false, 3u*sizeof(coreFloat) + 5u*sizeof(coreUint32));
            }

            // invoke buffer update
            m_bUpdate = true;
        }
    }
    else
    {
        // delete vertex array objects
        if(m_aiVertexArray[0]) glDeleteVertexArrays(CORE_OBJECT3D_INSTANCE_BUFFERS, m_aiVertexArray);
        m_aiVertexArray.Fill(0u);

        // delete instance data buffers
        FOR_EACH(it, *m_aiInstanceBuffer.List())
            it->Delete();
    }
}