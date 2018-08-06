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
/* constructor */
coreObject3D::coreObject3D()noexcept
: coreObject           ()
, m_vPosition          (coreVector3(FLT_MAX,FLT_MAX,FLT_MAX))
, m_vSize              (coreVector3(1.0f,1.0f,1.0f))
, m_vDirection         (coreVector3(0.0f,1.0f,0.0f))
, m_vOrientation       (coreVector3(0.0f,0.0f,1.0f))
, m_pModel             (NULL)
, m_vRotation          (coreVector4::QuatIdentity())
, m_vCollisionModifier (coreVector3(1.0f,1.0f,1.0f))
, m_vCollisionRange    (coreVector3(0.0f,0.0f,0.0f))
, m_fCollisionRadius   (0.0f)
, m_iType              (0)
{
}

coreObject3D::coreObject3D(const coreObject3D& c)noexcept
: coreObject           (c)
, m_vPosition          (c.m_vPosition)
, m_vSize              (c.m_vSize)
, m_vDirection         (c.m_vDirection)
, m_vOrientation       (c.m_vOrientation)
, m_pModel             (c.m_pModel)
, m_vRotation          (c.m_vRotation)
, m_vCollisionModifier (c.m_vCollisionModifier)
, m_vCollisionRange    (c.m_vCollisionRange)
, m_fCollisionRadius   (c.m_fCollisionRadius)
, m_iType              (0)
{
    // bind to object manager
    this->ChangeType(c.m_iType);
}

coreObject3D::coreObject3D(coreObject3D&& m)noexcept
: coreObject           (std::move(m))
, m_vPosition          (m.m_vPosition)
, m_vSize              (m.m_vSize)
, m_vDirection         (m.m_vDirection)
, m_vOrientation       (m.m_vOrientation)
, m_pModel             (std::move(m.m_pModel))
, m_vRotation          (m.m_vRotation)
, m_vCollisionModifier (m.m_vCollisionModifier)
, m_vCollisionRange    (m.m_vCollisionRange)
, m_fCollisionRadius   (m.m_fCollisionRadius)
, m_iType              (0)
{
    // bind to object manager
    this->ChangeType(m.m_iType);
}


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
    this->coreObject::operator = (c);
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
    this->coreObject::operator = (std::move(m));
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
    coreProgram* pLocal = pProgram.GetResource();
    pLocal->SendUniform(CORE_SHADER_UNIFORM_3D_POSITION, m_vPosition);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_3D_SIZE,     m_vSize);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_3D_ROTATION, m_vRotation);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_COLOR,       m_vColor);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_TEXPARAM,    coreVector4(m_vTexSize, m_vTexOffset));

    // enable all active textures
    coreTexture::EnableAll(m_apTexture);

    // enable the model
    m_pModel->Enable();
    return true;
}

coreBool coreObject3D::Prepare()
{
    // enable default shader-program
    return this->Prepare(m_pProgram);
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
    // render with default shader-program (no inheritance)
    this->coreObject3D::Render(m_pProgram);
}


// ****************************************************************
/* move the 3d-object */
void coreObject3D::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // check current update status
    if(m_iUpdate)
    {
        if(CONTAINS_FLAG(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
        {
            // update rotation quaternion
            m_vRotation = coreMatrix4::Orientation(m_vDirection, m_vOrientation).m123().ToQuat();
        }
        if(CONTAINS_FLAG(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION))
        {
            // cancel update without valid model
            if(!m_pModel.IsUsable() || !m_pModel->GetBoundingRadius()) return;

            // calculate extend and correction (resizing short range-axis should have low effect on radius)
            const coreVector3 vExtend     = this->GetSize() * m_vCollisionModifier;
            const coreVector3 vCorrection = m_pModel->GetBoundingRange() * RCP(m_pModel->GetBoundingRadius());

            // update collision range and radius
            m_vCollisionRange  = m_pModel->GetBoundingRange () * (vExtend);
            m_fCollisionRadius = m_pModel->GetBoundingRadius() * (vExtend * (vCorrection * RCP(vCorrection.Max()))).Max();
        }

        // reset the update status
        m_iUpdate = CORE_OBJECT_UPDATE_NOTHING;
    }
}


// ****************************************************************
/* change object type and manager registration */
void coreObject3D::ChangeType(const coreInt32 iType)
{
    if(m_iType == iType) return;

    // unbind from old type and bind to new type
    if(m_iType) Core::Manager::Object->__UnbindObject(this, m_iType);
    m_iType = iType;
    if(m_iType) Core::Manager::Object->__BindObject  (this, m_iType);
}


// ****************************************************************
/* constructor */
coreBatchList::coreBatchList(const coreUint32 iStartCapacity)noexcept
: coreResourceRelation ()
, m_apObjectList       {}
, m_iCurCapacity       (iStartCapacity)
, m_iCurEnabled        (0u)
, m_pProgram           (NULL)
, m_aiVertexArray      {}
, m_aInstanceBuffer    {}
, m_paCustomBuffer     (NULL)
, m_pLastModel         (NULL)
, m_nDefineBufferFunc  (NULL)
, m_nUpdateDataFunc    (NULL)
, m_nUpdateShaderFunc  (NULL)
, m_iCustomSize        (0u)
, m_iFilled            (0u)
, m_iUpdate            (0u)
{
    // reserve memory for objects
    m_apObjectList.reserve(iStartCapacity);

    // create vertex array objects and instance data buffers
    m_aiVertexArray.fill(0u);
    if(iStartCapacity) this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
/* destructor */
coreBatchList::~coreBatchList()
{
    // clear memory
    m_apObjectList.clear();

    // delete vertex array objects and instance data buffers
    this->__Reset(CORE_RESOURCE_RESET_EXIT);

    // free custom attribute buffer memory
    SAFE_DELETE(m_paCustomBuffer)
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

    // re-determine render-count (may have changed between move and render)
    if(m_iUpdate) m_iCurEnabled = std::count_if(m_apObjectList.begin(), m_apObjectList.end(), [](const coreObject3D* pObject) {return pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER);});
    if(!m_iCurEnabled) return;

    // check for custom vertex attributes
    if(this->IsCustom()) this->__RenderCustom (pProgramInstanced, pProgramSingle);
                    else this->__RenderDefault(pProgramInstanced, pProgramSingle);
}

void coreBatchList::Render()
{
    if(m_apObjectList.empty()) return;

    // render with default shader-programs
    this->Render(m_pProgram, m_apObjectList.front()->GetProgram());
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
    m_iUpdate = 3u;
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
    m_iUpdate = 3u;
}


// ****************************************************************
/* bind 3d-object */
void coreBatchList::BindObject(coreObject3D* pObject)
{
    ASSERT(m_iCurCapacity)

    if(m_apObjectList.size() >= m_iCurCapacity)
    {
        // increase current capacity by 50%
        this->Reallocate(m_iCurCapacity + m_iCurCapacity / 2u + 1u);
    }

    // add object to list
    m_apObjectList.insert(pObject);

    // set the update status
    m_iUpdate = 3u;
}


// ****************************************************************
/* unbind 3d-object */
void coreBatchList::UnbindObject(coreObject3D* pObject)
{
    // remove object from list
    m_apObjectList.erase(pObject);

    // set the update status
    m_iUpdate = 3u;
}


// ****************************************************************
/* change current capacity */
void coreBatchList::Reallocate(const coreUint32 iNewCapacity)
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
    // just clear memory
    m_apObjectList.clear();
}


// ****************************************************************
/* remove unused capacity */
void coreBatchList::ShrinkToFit()
{
    // reallocate to current number of bound objects
    this->Reallocate(m_apObjectList.size());
    m_apObjectList.shrink_to_fit();
}


// ****************************************************************
/* reset with the resource manager */
void coreBatchList::__Reset(const coreResourceReset bInit)
{
    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_uniform_buffer_object) || !CORE_GL_SUPPORT(ARB_vertex_array_object) || !CORE_GL_SUPPORT(ARB_half_float_vertex)) return;

    if(bInit)
    {
        WARN_IF(m_aInstanceBuffer[0].IsValid()) return;

        // only allocate with enough capacity
        if(m_iCurCapacity >= CORE_OBJECT3D_INSTANCE_THRESHOLD)
        {
            FOR_EACH(it, m_aInstanceBuffer)
            {
                // create vertex array object
                coreGenVertexArrays(1u, &m_aiVertexArray.current());
                glBindVertexArray(m_aiVertexArray.current());
                m_aiVertexArray.next();

                // create instance data buffer
                it->Create(m_iCurCapacity, CORE_OBJECT3D_INSTANCE_SIZE, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC | CORE_DATABUFFER_STORAGE_FENCED);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 3u, GL_FLOAT,         false, 0u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,     4u, GL_HALF_FLOAT,    false, 3u*sizeof(coreFloat));
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM, 4u, GL_SHORT,         false, 3u*sizeof(coreFloat) + 2u*sizeof(coreUint32));
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    4u, GL_UNSIGNED_BYTE, false, 3u*sizeof(coreFloat) + 4u*sizeof(coreUint32));
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM, 4u, GL_HALF_FLOAT,    false, 3u*sizeof(coreFloat) + 5u*sizeof(coreUint32));

                // set vertex data (instancing only)
                it->ActivateDivided(1u, 1u);

                if(this->IsCustom())
                {
                    coreVertexBuffer& oBuffer = m_paCustomBuffer->current();
                    m_paCustomBuffer->next();

                    // create custom attribute buffer
                    oBuffer.Create(m_iCurCapacity, m_iCustomSize, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC | CORE_DATABUFFER_STORAGE_FENCED);
                    m_nDefineBufferFunc(&oBuffer);

                    // set vertex data (custom only)
                    oBuffer.ActivateDivided(2u, 1u);
                }
            }

            // disable current model object (to fully enable the next model)
            coreModel::Disable(false);

            // invoke vertex array and buffer update
            m_iFilled = 0u;
            m_iUpdate = 3u;
        }
    }
    else
    {
        // delete vertex array objects
        if(m_aiVertexArray[0]) glDeleteVertexArrays(CORE_OBJECT3D_INSTANCE_BUFFERS, m_aiVertexArray.data());
        m_aiVertexArray.fill(0u);

        // delete instance data buffers
        FOR_EACH(it, m_aInstanceBuffer) it->Delete();

        // reset selected array and buffer (to synchronize)
        m_aiVertexArray  .select(0u);
        m_aInstanceBuffer.select(0u);

        if(this->IsCustom())
        {
            // delete custom attribute buffers
            FOR_EACH(it, *m_paCustomBuffer) it->Delete();

            // reset selected buffer (to synchronize)
            m_paCustomBuffer->select(0u);
        }
    }
}


// ****************************************************************
/* render without inheritance or additional attributes */
void coreBatchList::__RenderDefault(const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramSingle)
{
    if(this->IsInstanced())
    {
        // get first object from list
        const coreObject3D* pFirst = m_apObjectList.front();
        const coreModelPtr& pModel = pFirst->GetModel();

        // check for model status
        if(!pModel.IsUsable()) return;

        // detect model changes and invoke update
        if(m_pLastModel != pModel.GetHandle()) m_iFilled = 0u;
        m_pLastModel = pModel.GetHandle();

        // enable the shader-program
        if(!pProgramInstanced.IsUsable()) return;
        if(!pProgramInstanced->Enable())  return;

        // enable all active textures
        coreTexture::EnableAll(&pFirst->GetTexture(0u));

        if(CONTAINS_BIT(m_iUpdate, 0u))
        {
            // invalidate previous buffer
            m_aInstanceBuffer.current().Invalidate();

            // switch to next available array and buffer
            m_aiVertexArray  .next();
            m_aInstanceBuffer.next();

            // map required area of the instance data buffer
            coreByte* pRange  = m_aInstanceBuffer.current().Map(0u, m_iCurEnabled * CORE_OBJECT3D_INSTANCE_SIZE, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
            coreByte* pCursor = pRange;

            FOR_EACH(it, m_apObjectList)
            {
                const coreObject3D* pObject = (*it);

                // render only enabled objects
                if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
                {
                    // compress data
                    const coreUint64 iSize      = coreVector4(pObject->GetSize(), 0.0f)                      .PackFloat4x16();
                    const coreUint64 iRotation  = pObject->GetRotation()                                     .PackSnorm4x16();
                    const coreUint32 iColor     = pObject->GetColor4  ()                                     .PackUnorm4x8 ();
                    const coreUint64 iTexParams = coreVector4(pObject->GetTexSize(), pObject->GetTexOffset()).PackFloat4x16();
                    ASSERT((pObject->GetColor4   ().Min() >=  0.0f)  && (pObject->GetColor4   ().Max() <= 1.0f))
                    ASSERT((pObject->GetTexOffset().Min() >= -50.0f) && (pObject->GetTexOffset().Max() <= 50.0f))

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
            m_aInstanceBuffer.current().Unmap(pRange);

            // reset the update status
            REMOVE_BIT(m_iUpdate, 0u)
        }

        // disable current model object (because of direct VAO use)
        coreModel::Disable(false);

        // bind vertex array object
        glBindVertexArray(m_aiVertexArray.current());

        // activate missing geometry data
        if(!CONTAINS_BIT(m_iFilled, m_aiVertexArray.index()))
        {
            ADD_BIT(m_iFilled, m_aiVertexArray.index())
            STATIC_ASSERT(sizeof(m_iFilled)*8u >= CORE_OBJECT3D_INSTANCE_BUFFERS)

            // set vertex data (model only)
            pModel->GetVertexBuffer(0u)->Activate(0u);

            // set index data
            if(pModel->GetIndexBuffer()->IsValid())
            {
                coreDataBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER, false);
                pModel->GetIndexBuffer()->Bind();
            }
        }

        // draw the model instanced
        pModel->DrawInstanced(m_iCurEnabled);
    }
    else
    {
        FOR_EACH(it, m_apObjectList)
        {
            coreObject3D* pObject = (*it);

            // render only enabled objects
            if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
            {
                // draw without instancing (no inheritance)
                pObject->coreObject3D::Render(pProgramSingle);
            }
        }
    }
}


// ****************************************************************
/* render with custom vertex attributes per active object */
void coreBatchList::__RenderCustom(const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramSingle)
{
    ASSERT(this->IsCustom())

    if(this->IsInstanced())
    {
        if(CONTAINS_BIT(m_iUpdate, 1u))
        {
            // invalidate previous buffer
            m_paCustomBuffer->current().Invalidate();

            // switch to next available buffer
            m_paCustomBuffer->select(m_aInstanceBuffer.index());
            if(CONTAINS_BIT(m_iUpdate, 0u)) m_paCustomBuffer->next();

            // map required area of the custom attribute buffer
            coreByte* pRange  = m_paCustomBuffer->current().Map(0u, m_iCurEnabled * m_iCustomSize, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
            coreByte* pCursor = pRange;

            FOR_EACH(it, m_apObjectList)
            {
                coreObject3D* pObject = (*it);

                // render only enabled objects
                if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
                {
                    // write data to the buffer
                    m_nUpdateDataFunc(pCursor, pObject);
                    pCursor += m_iCustomSize;
                }
            }

            // unmap buffer
            m_paCustomBuffer->current().Unmap(pRange);

            // reset the update status
            REMOVE_BIT(m_iUpdate, 1u)
        }

        // render the batch list
        this->__RenderDefault(pProgramInstanced, pProgramSingle);
    }
    else
    {
        FOR_EACH(it, m_apObjectList)
        {
            coreObject3D* pObject = (*it);

            // render only enabled objects
            if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
            {
                // update shader manually
                m_nUpdateShaderFunc(pProgramSingle, pObject);

                // draw without instancing (no inheritance)
                pObject->coreObject3D::Render(pProgramSingle);
            }
        }
    }
}