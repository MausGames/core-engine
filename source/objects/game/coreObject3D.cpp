///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
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
, m_pVolume            (NULL)
, m_vRotation          (coreVector4::QuatIdentity())
, m_vCollisionModifier (coreVector3(1.0f,1.0f,1.0f))
, m_vCollisionRange    (coreVector3(0.0f,0.0f,0.0f))
, m_fCollisionRadius   (0.0f)
, m_vVisualRange       (coreVector3(0.0f,0.0f,0.0f))
, m_fVisualRadius      (0.0f)
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
, m_pVolume            (c.m_pVolume)
, m_vRotation          (c.m_vRotation)
, m_vCollisionModifier (c.m_vCollisionModifier)
, m_vCollisionRange    (c.m_vCollisionRange)
, m_fCollisionRadius   (c.m_fCollisionRadius)
, m_vVisualRange       (c.m_vVisualRange)
, m_fVisualRadius      (c.m_fVisualRadius)
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
, m_pVolume            (std::move(m.m_pVolume))
, m_vRotation          (m.m_vRotation)
, m_vCollisionModifier (m.m_vCollisionModifier)
, m_vCollisionRange    (m.m_vCollisionRange)
, m_fCollisionRadius   (m.m_fCollisionRadius)
, m_vVisualRange       (m.m_vVisualRange)
, m_fVisualRadius      (m.m_fVisualRadius)
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

    // copy properties
    this->coreObject::operator = (c);
    m_vPosition          = c.m_vPosition;
    m_vSize              = c.m_vSize;
    m_vDirection         = c.m_vDirection;
    m_vOrientation       = c.m_vOrientation;
    m_pModel             = c.m_pModel;
    m_pVolume            = c.m_pVolume;
    m_vRotation          = c.m_vRotation;
    m_vCollisionModifier = c.m_vCollisionModifier;
    m_vCollisionRange    = c.m_vCollisionRange;
    m_fCollisionRadius   = c.m_fCollisionRadius;
    m_vVisualRange       = c.m_vVisualRange;
    m_fVisualRadius      = c.m_fVisualRadius;

    return *this;
}

coreObject3D& coreObject3D::operator = (coreObject3D&& m)noexcept
{
    // bind to object manager
    this->ChangeType(m.m_iType);

    // move properties
    this->coreObject::operator = (std::move(m));
    m_vPosition          = m.m_vPosition;
    m_vSize              = m.m_vSize;
    m_vDirection         = m.m_vDirection;
    m_vOrientation       = m.m_vOrientation;
    m_pModel             = std::move(m.m_pModel);
    m_pVolume            = std::move(m.m_pVolume);
    m_vRotation          = m.m_vRotation;
    m_vCollisionModifier = m.m_vCollisionModifier;
    m_vCollisionRange    = m.m_vCollisionRange;
    m_fCollisionRadius   = m.m_fCollisionRadius;
    m_vVisualRange       = m.m_vVisualRange;
    m_fVisualRadius      = m.m_fVisualRadius;

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
    m_pVolume  = NULL;
}


// ****************************************************************
/* separately enable all resources for rendering */
coreBool coreObject3D::Prepare(const coreProgramPtr& pProgram)
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return false;

    // check for model status
    ASSERT(m_pModel)
    if(!m_pModel.IsUsable()) return false;

    // enable the shader-program
    ASSERT(pProgram)
    if(!pProgram.IsUsable()) return false;
    if(!pProgram->Enable())  return false;

    // update all object uniforms
    coreProgram* pLocal = pProgram.GetResource();
    pLocal->SendUniform(CORE_SHADER_UNIFORM_3D_POSITION, m_vPosition);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_3D_SIZE,     m_vSize);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_3D_ROTATION, m_vRotation);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_COLOR,       m_vColor);
    pLocal->SendUniform(CORE_SHADER_UNIFORM_TEXPARAM,    coreVector4(m_vTexSize, m_vTexOffset));
    ASSERT(pLocal->RetrieveUniform(CORE_SHADER_UNIFORM_3D_POSITION) >= 0)

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
    if(m_eUpdate)
    {
        if(HAS_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
        {
            // update rotation quaternion
            m_vRotation = coreMatrix4::Orientation(m_vDirection, m_vOrientation).m123().ToQuat();
        }
        if(HAS_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_COLLISION))
        {
            // cancel update without valid volume
            const coreModelPtr& pVolume = m_pVolume ? m_pVolume : m_pModel;
            if(!pVolume.IsUsable()) return;

            // calculate extent and correction
            const coreVector3 vExtent     = m_vSize * m_vCollisionModifier;
            const coreVector3 vCorrection = pVolume->GetBoundingRange() / MAX(pVolume->GetBoundingRange().Max(), CORE_MATH_PRECISION);

            // update collision range and radius
            m_vCollisionRange  = pVolume->GetBoundingRange () * (vExtent);
            m_fCollisionRadius = pVolume->GetBoundingRadius() * (vExtent * vCorrection).Max();
        }
        if(HAS_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_VISUAL))
        {
            // cancel update without valid model
            if(!m_pModel.IsUsable()) return;

            // calculate correction
            const coreVector3 vCorrection = m_pModel->GetBoundingRange() / MAX(m_pModel->GetBoundingRange().Max(), CORE_MATH_PRECISION);

            // update visual range and radius
            m_vVisualRange  = m_pModel->GetBoundingRange () * (m_vSize);
            m_fVisualRadius = m_pModel->GetBoundingRadius() * (m_vSize * vCorrection).Max();
        }

        // reset the update status
        m_eUpdate = CORE_OBJECT_UPDATE_NOTHING;
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
, m_iNumInstances      (0u)
, m_iNumEnabled        (0u)
, m_pProgram           (NULL)
, m_aiVertexArray      {}
, m_aInstanceBuffer    {}
, m_paCustomBuffer     (NULL)
, m_iLastModel         (UINT32_MAX)
, m_nDefineBufferFunc  (NULL)
, m_nUpdateDataFunc    (NULL)
, m_nUpdateShaderFunc  (NULL)
, m_iCustomSize        (0u)
, m_iFilled            (0u)
, m_eUpdate            (CORE_BATCHLIST_UPDATE_NOTHING)
{
    // reserve memory for objects
    m_apObjectList.reserve(iStartCapacity);
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
    const coreUint32 iRenderCount = std::count_if(m_apObjectList.begin(), m_apObjectList.end(), [](const coreObject3D* pObject) {return pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER);});
    if(!iRenderCount) return;

    // adjust buffer capacity (with additional space)
    if(iRenderCount > m_iNumInstances) this->Reallocate(MIN(iRenderCount + iRenderCount / 10u + 1u, m_apObjectList.capacity()));

    // check for custom vertex attributes
    if(this->IsCustom()) this->__RenderCustom (pProgramInstanced, pProgramSingle, iRenderCount);
                    else this->__RenderDefault(pProgramInstanced, pProgramSingle, iRenderCount);
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
    m_iNumEnabled = 0u;

    FOR_EACH(it, m_apObjectList)
    {
        coreObject3D* pObject = (*it);

        // move object
        pObject->Move();

        // increase render-count (# after move)
        if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
            ++m_iNumEnabled;
    }

    // set the update status
    m_eUpdate = CORE_BATCHLIST_UPDATE_ALL;
}


// ****************************************************************
/* move the batch list with automatic temporal sort */
void coreBatchList::MoveSort()
{
    auto et = m_apObjectList.begin();

    // reset render-count
    m_iNumEnabled = 0u;

    // compare first object with nothing (never true)
    coreFloat fOldDistance    = 0.0f;
    coreBool  bOldTransparent = false;

    FOR_EACH(it, m_apObjectList)
    {
        coreObject3D* pObject = (*it);

        // move only enabled objects
        if(pObject->IsEnabled(CORE_OBJECT_ENABLE_MOVE))
        {
            // move object
            pObject->Move();

            // calculate properties of current object
            const coreFloat fCurDistance    = (pObject->GetPosition() - Core::Graphics->GetCamPosition()).LengthSq();
            const coreBool  bCurTransparent = (pObject->GetAlpha() < 1.0f);

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

        // increase render-count (# after move)
        if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
            ++m_iNumEnabled;
    }

    // set the update status
    m_eUpdate = CORE_BATCHLIST_UPDATE_ALL;
}


// ****************************************************************
/* bind 3d-object */
void coreBatchList::BindObject(coreObject3D* pObject)
{
    // add object to list
    m_apObjectList.insert(pObject);

    // set the update status
    m_eUpdate = CORE_BATCHLIST_UPDATE_ALL;
}


// ****************************************************************
/* unbind 3d-object */
void coreBatchList::UnbindObject(coreObject3D* pObject)
{
    // remove object from list
    m_apObjectList.erase(pObject);

    // set the update status
    m_eUpdate = CORE_BATCHLIST_UPDATE_ALL;
}


// ****************************************************************
/* remove all 3d-objects */
void coreBatchList::Clear()
{
    // just clear memory
    m_apObjectList.clear();
}


// ****************************************************************
/* change current size */
void coreBatchList::Reallocate(const coreUint32 iSize)
{
         if(iSize == m_iNumInstances) return;
    WARN_IF(iSize <  m_iNumEnabled)   return;

    // change current size
    m_iNumInstances = iSize;

    // reallocate the instance data buffers
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
    this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
/* reset with the resource manager */
void coreBatchList::__Reset(const coreResourceReset eInit)
{
    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_vertex_array_object)) return;

    if(eInit)
    {
        WARN_IF(m_aInstanceBuffer[0].IsValid()) return;

        // only allocate with enough capacity
        if(m_iNumInstances >= CORE_BATCHLIST_INSTANCE_THRESHOLD)
        {
            FOR_EACH(it, m_aInstanceBuffer)
            {
                // create vertex array object
                coreGenVertexArrays(1u, &m_aiVertexArray.current());
                glBindVertexArray(m_aiVertexArray.current());
                m_aiVertexArray.next();

                if(CORE_GL_SUPPORT(ARB_half_float_vertex))
                {
                    // create instance data buffer (high quality compression)
                    it->Create(m_iNumInstances, CORE_BATCHLIST_INSTANCE_SIZE_HIGH, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 3u, GL_FLOAT,         12u, false, 0u, 0u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,     4u, GL_HALF_FLOAT,    8u,  false, 0u, 12u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM, 4u, GL_SHORT,         8u,  false, 0u, 20u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    4u, GL_UNSIGNED_BYTE, 4u,  false, 0u, 28u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM, 4u, GL_HALF_FLOAT,    8u,  false, 0u, 32u);
                }
                else
                {
                    // create instance data buffer (low quality compression)
                    it->Create(m_iNumInstances, CORE_BATCHLIST_INSTANCE_SIZE_LOW, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 3u, GL_FLOAT,         12u, false, 0u, 0u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,     3u, GL_FLOAT,         12u, false, 0u, 12u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM, 4u, GL_SHORT,         8u,  false, 0u, 24u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    4u, GL_UNSIGNED_BYTE, 4u,  false, 0u, 32u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM, 4u, GL_FLOAT,         16u, false, 0u, 36u);
                }

                // set vertex data (instancing only)
                it->Activate(1u);

                if(this->IsCustom())
                {
                    coreVertexBuffer& oBuffer = m_paCustomBuffer->current();
                    m_paCustomBuffer->next();

                    // create custom attribute buffer
                    oBuffer.Create(m_iNumInstances, m_iCustomSize, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
                    m_nDefineBufferFunc(&oBuffer);

                    // set vertex data (custom only)
                    oBuffer.Activate(1u);
                }
            }

            // disable current model object (to fully enable the next model)
            coreModel::Disable(false);

            // invoke vertex array and buffer update
            m_iFilled = 0u;
            m_eUpdate = CORE_BATCHLIST_UPDATE_ALL;
        }
    }
    else
    {
        // delete vertex array objects
        if(m_aiVertexArray[0]) coreDelVertexArrays(CORE_BATCHLIST_INSTANCE_BUFFERS, m_aiVertexArray.data());
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
void coreBatchList::__RenderDefault(const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramSingle, const coreUint32 iRenderCount)
{
    ASSERT(iRenderCount)

    if(this->IsInstanced())
    {
        // get first object from list
        const coreObject3D* pFirst = m_apObjectList.front();
        const coreModelPtr& pModel = pFirst->GetModel();

        // check for model status
        ASSERT(pModel)
        if(!pModel.IsUsable()) return;

        // detect model changes and invoke update
        const GLuint iIdentifier = pModel->GetVertexBuffer(0u)->GetIdentifier();
        if(m_iLastModel != iIdentifier) m_iFilled = 0u;
        m_iLastModel = iIdentifier;

        // enable the shader-program
        ASSERT(pProgramInstanced)
        if(!pProgramInstanced.IsUsable()) return;
        if(!pProgramInstanced->Enable())  return;

        // enable all active textures
        coreTexture::EnableAll(&pFirst->GetTexture(0u));

        if(HAS_FLAG(m_eUpdate, CORE_BATCHLIST_UPDATE_INSTANCE))
        {
            // invalidate and synchronize previous buffer
            m_aInstanceBuffer.current().Invalidate();
            m_aInstanceBuffer.current().SyncWrite(CORE_DATABUFFER_MAP_INVALIDATE_ALL);

            // switch to next available array and buffer
            m_aiVertexArray  .next();
            m_aInstanceBuffer.next();

            if(CORE_GL_SUPPORT(ARB_half_float_vertex))
            {
                // map required area of the instance data buffer
                coreByte* pRange  = m_aInstanceBuffer.current().MapWrite(0u, iRenderCount * CORE_BATCHLIST_INSTANCE_SIZE_HIGH, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
                coreByte* pCursor = pRange;

                FOR_EACH(it, m_apObjectList)
                {
                    const coreObject3D* pObject = (*it);

                    // render only enabled objects
                    if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
                    {
                        // compress data
                        const coreVector3 vPosition  = pObject->GetPosition();
                        const coreUint64  iSize      = coreVector4(pObject->GetSize(), 0.0f)                      .PackFloat4x16();
                        const coreUint64  iRotation  = pObject->GetRotation()                                     .PackSnorm4x16();
                        const coreUint32  iColor     = pObject->GetColor4  ()                                     .PackUnorm4x8 ();
                        const coreUint64  iTexParams = coreVector4(pObject->GetTexSize(), pObject->GetTexOffset()).PackFloat4x16();
                        ASSERT((pObject->GetColor4   ().Min() >=  0.0f) && (pObject->GetColor4   ().Max() <= 1.0f))
                        ASSERT((pObject->GetTexOffset().Min() >= -4.0f) && (pObject->GetTexOffset().Max() <= 4.0f))

                        // write data to the buffer
                        std::memcpy(pCursor,       &vPosition,  sizeof(coreVector3));
                        std::memcpy(pCursor + 12u, &iSize,      sizeof(coreUint64));
                        std::memcpy(pCursor + 20u, &iRotation,  sizeof(coreUint64));
                        std::memcpy(pCursor + 28u, &iColor,     sizeof(coreUint32));
                        std::memcpy(pCursor + 32u, &iTexParams, sizeof(coreUint64));
                        pCursor += CORE_BATCHLIST_INSTANCE_SIZE_HIGH;
                    }
                }

                ASSERT(coreUint32(pCursor - pRange) == iRenderCount * CORE_BATCHLIST_INSTANCE_SIZE_HIGH)
            }
            else
            {
                // map required area of the instance data buffer
                coreByte* pRange  = m_aInstanceBuffer.current().MapWrite(0u, iRenderCount * CORE_BATCHLIST_INSTANCE_SIZE_LOW, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
                coreByte* pCursor = pRange;

                FOR_EACH(it, m_apObjectList)
                {
                    const coreObject3D* pObject = (*it);

                    // render only enabled objects
                    if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
                    {
                        // compress data
                        const coreVector3 vPosition  = pObject->GetPosition();
                        const coreVector3 vSize      = pObject->GetSize    ();
                        const coreUint64  iRotation  = pObject->GetRotation().PackSnorm4x16();
                        const coreUint32  iColor     = pObject->GetColor4  ().PackUnorm4x8 ();
                        const coreVector4 vTexParams = coreVector4(pObject->GetTexSize(), pObject->GetTexOffset());
                        ASSERT((pObject->GetColor4   ().Min() >=  0.0f) && (pObject->GetColor4   ().Max() <= 1.0f))
                        ASSERT((pObject->GetTexOffset().Min() >= -4.0f) && (pObject->GetTexOffset().Max() <= 4.0f))

                        // write data to the buffer
                        std::memcpy(pCursor,       &vPosition,  sizeof(coreVector3));
                        std::memcpy(pCursor + 12u, &vSize,      sizeof(coreVector3));
                        std::memcpy(pCursor + 24u, &iRotation,  sizeof(coreUint64));
                        std::memcpy(pCursor + 32u, &iColor,     sizeof(coreUint32));
                        std::memcpy(pCursor + 36u, &vTexParams, sizeof(coreVector4));
                        pCursor += CORE_BATCHLIST_INSTANCE_SIZE_LOW;
                    }
                }

                ASSERT(coreUint32(pCursor - pRange) == iRenderCount * CORE_BATCHLIST_INSTANCE_SIZE_LOW)
            }

            // unmap buffer
            m_aInstanceBuffer.current().Unmap();

            // reset the update status
            REMOVE_FLAG(m_eUpdate, CORE_BATCHLIST_UPDATE_INSTANCE)
        }

        // disable current model object (because of direct VAO use)
        coreModel::Disable(false);

        // bind vertex array object
        glBindVertexArray(m_aiVertexArray.current());

        // activate missing geometry data
        if(!HAS_BIT(m_iFilled, m_aiVertexArray.index()))
        {
            ADD_BIT(m_iFilled, m_aiVertexArray.index())
            STATIC_ASSERT(CORE_BATCHLIST_INSTANCE_BUFFERS <= BITSOF(m_iFilled))

            // set vertex data (model only)
            for(coreUintW i = 0u, ie = pModel->GetNumVertexBuffers(); i < ie; ++i)
                pModel->GetVertexBuffer(i)->Activate(0u);

            // set index data
            if(pModel->GetIndexBuffer()->IsValid())
            {
                coreDataBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER, false);
                pModel->GetIndexBuffer()->Bind();
            }
        }

        // draw the model instanced
        pModel->DrawInstanced(iRenderCount);
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
void coreBatchList::__RenderCustom(const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramSingle, const coreUint32 iRenderCount)
{
    ASSERT(iRenderCount && this->IsCustom())

    if(this->IsInstanced())
    {
        if(HAS_FLAG(m_eUpdate, CORE_BATCHLIST_UPDATE_CUSTOM))
        {
            // invalidate and synchronize previous buffer
            m_paCustomBuffer->current().Invalidate();
            m_paCustomBuffer->current().SyncWrite(CORE_DATABUFFER_MAP_INVALIDATE_ALL);

            // switch to next available buffer
            m_paCustomBuffer->select(m_aInstanceBuffer.index());
            if(HAS_FLAG(m_eUpdate, CORE_BATCHLIST_UPDATE_INSTANCE)) m_paCustomBuffer->next();

            // map required area of the custom attribute buffer
            coreByte* pRange  = m_paCustomBuffer->current().MapWrite(0u, iRenderCount * m_iCustomSize, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
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

            ASSERT(coreUint32(pCursor - pRange) == iRenderCount * m_iCustomSize)

            // unmap buffer
            m_paCustomBuffer->current().Unmap();

            // reset the update status
            REMOVE_FLAG(m_eUpdate, CORE_BATCHLIST_UPDATE_CUSTOM)
        }

        // render the batch list
        this->__RenderDefault(pProgramInstanced, pProgramSingle, iRenderCount);
    }
    else
    {
        // enable the shader-program
        ASSERT(pProgramSingle)
        if(!pProgramSingle.IsUsable()) return;
        if(!pProgramSingle->Enable())  return;

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