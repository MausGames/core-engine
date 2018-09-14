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
// constructor
coreParticleSystem::coreParticleSystem(const coreUint32 iNumParticles)noexcept
: coreResourceRelation ()
, m_pParticle          (NULL)
, m_iNumParticles      (iNumParticles)
, m_iCurParticle       (0u)
, m_apTexture          {}
, m_pProgram           (NULL)
, m_apRenderList       {}
, m_pDefaultEffect     (NULL)
, m_aiVertexArray      {}
, m_aiInstanceBuffer   {}
, m_bUpdate            (false)
{
    ASSERT(iNumParticles)

    // pre-allocate particles
    m_pParticle = ALIGNED_NEW(coreParticle, m_iNumParticles, ALIGNMENT_CACHE);
    std::memset(m_pParticle, 0, sizeof(coreParticle) * m_iNumParticles);

    // create default particle effect object
    m_pDefaultEffect = new coreParticleEffect(this);
    m_pDefaultEffect->m_pThis = m_pDefaultEffect;

    // create vertex array objects and instance data buffers
    m_aiVertexArray.fill(0u);
    this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
// destructor
coreParticleSystem::~coreParticleSystem()
{
    // delete particles
    this->ClearAll();
    ALIGNED_DELETE(m_pParticle)

    // delete default particle effect object
    SAFE_DELETE(m_pDefaultEffect)

    // delete vertex array objects and instance data buffers
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
}


// ****************************************************************
// undefine the visual appearance
void coreParticleSystem::Undefine()
{
    // reset all resource and memory pointers
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
}


// ****************************************************************
// render the particle system
void coreParticleSystem::Render()
{
    if(m_apRenderList.empty()) return;

    // enable the shader-program
    if(!m_pProgram.IsUsable()) return;
    if(!m_pProgram->Enable())  return;

    // enable all active textures
    coreTexture::EnableAll(m_apTexture);

    if(m_aiInstanceBuffer[0].IsValid())
    {
        if(m_bUpdate)
        {
            // invalidate previous buffer
            m_aiInstanceBuffer.current().Invalidate();

            // switch to next available array and buffer
            m_aiVertexArray   .next();
            m_aiInstanceBuffer.next();

            // map required area of the instance data buffer
            coreByte* pRange  = m_aiInstanceBuffer.current().Map(0u, m_apRenderList.size() * CORE_PARTICLE_INSTANCE_SIZE, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
            coreByte* pCursor = pRange;

            FOR_EACH_REV(it, m_apRenderList)
            {
                // get current particle state
                const coreParticle* pParticle = (*it);
                const coreObject3D* pOrigin   = pParticle->GetEffect()->GetOrigin();

                // compress data
                const coreVector3 vPosition = pOrigin ? (pOrigin->GetPosition() + pParticle->GetCurPosition()) : pParticle->GetCurPosition();
                const coreUint64  iData     = coreVector4(pParticle->GetCurScale(), pParticle->GetCurAngle(), pParticle->GetValue(), 0.0f).PackFloat4x16();
                const coreUint32  iColor    = pParticle->GetCurColor4().PackUnorm4x8();
                ASSERT((pParticle->GetCurColor4().Min() >= 0.0f) && (pParticle->GetCurColor4().Max() <= 1.0f))

                // write remaining data to the buffer
                std::memcpy(pCursor,                                                &vPosition, sizeof(coreVector3));
                std::memcpy(pCursor + 3u*sizeof(coreFloat),                         &iData,     sizeof(coreUint64));
                std::memcpy(pCursor + 3u*sizeof(coreFloat) + 2u*sizeof(coreUint32), &iColor,    sizeof(coreUint32));
                pCursor += CORE_PARTICLE_INSTANCE_SIZE;
            }

            // unmap buffer
            m_aiInstanceBuffer.current().Unmap(pRange);

            // reset the update status
            m_bUpdate = false;
        }

        // disable current model object (because of direct VAO use)
        coreModel::Disable(false);

        // draw the model instanced
        glBindVertexArray(m_aiVertexArray.current());
        Core::Manager::Object->GetLowQuad()->DrawArraysInstanced(m_apRenderList.size());
    }
    else
    {
        coreProgram* pProgram = m_pProgram.GetResource();
        coreModel*   pModel   = Core::Manager::Object->GetLowQuad().GetResource();

        // draw without instancing
        FOR_EACH_REV(it, m_apRenderList)
        {
            // get current particle state
            const coreParticle* pParticle = (*it);
            const coreObject3D* pOrigin   = pParticle->GetEffect()->GetOrigin();

            // update all particle uniforms
            pProgram->SendUniform(CORE_SHADER_ATTRIBUTE_DIV_POSITION, pOrigin ? (pOrigin->GetPosition() + pParticle->GetCurPosition()) : pParticle->GetCurPosition());
            pProgram->SendUniform(CORE_SHADER_ATTRIBUTE_DIV_DATA,     coreVector3(pParticle->GetCurScale(), pParticle->GetCurAngle(), pParticle->GetValue()));
            pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,          pParticle->GetCurColor4());

            // draw the model
            pModel->Enable();
            pModel->DrawArrays();
        }
    }
}


// ****************************************************************
// move the particle system
void coreParticleSystem::Move()
{
    FOR_EACH_DYN(it, m_apRenderList)
    {
        coreParticle* pParticle = (*it);

        // update particle
        pParticle->__Update();
        if(!pParticle->IsActive())
        {
            // remove finished particle
            DYN_REMOVE(it, m_apRenderList)
        }
        else DYN_KEEP(it)
    }

    // set the update status
    m_bUpdate = true;
}


// ****************************************************************
// create new particle
coreParticle* coreParticleSystem::CreateParticle(coreParticleEffect* pEffect)
{
    ASSERT(pEffect)

    // loop through all particles
    for(coreUintW i = m_iNumParticles; i--; )
    {
        if(++m_iCurParticle >= m_iNumParticles) m_iCurParticle = 0u;

        // check current particle status
        coreParticle* pParticle = &m_pParticle[m_iCurParticle];
        if(!pParticle->IsActive())
        {
            // prepare particle and add to render list
            pParticle->__Prepare(pEffect);
            m_apRenderList.push_back(pParticle);

            return pParticle;
        }
    }

    // no free particle available
    WARN_IF(true) {}
    return &m_pParticle[m_iCurParticle];
}


// ****************************************************************
// remove particle effect objects
void coreParticleSystem::Unbind(coreParticleEffect* pEffect)
{
    ASSERT(pEffect)

    FOR_EACH(it, m_apRenderList)
    {
        coreParticle* pParticle = (*it);

        // check particle effect object
        if(pParticle->GetEffect() == pEffect)
        {
            const coreObject3D* pOrigin = pEffect->GetOrigin();

            // check origin object and transform position
            if(pOrigin)
            {
                pParticle->m_BeginState.vPosition += pOrigin->GetPosition();
                pParticle->m_EndState  .vPosition += pOrigin->GetPosition();
            }

            // reset associated particle effect object
            pParticle->m_pEffect = m_pDefaultEffect;
        }
    }
}


// ****************************************************************
// remove particles
void coreParticleSystem::Clear(coreParticleEffect* pEffect)
{
    ASSERT(pEffect)

    FOR_EACH_DYN(it, m_apRenderList)
    {
        coreParticle* pParticle = (*it);

        // check particle effect object
        if(pParticle->GetEffect() == pEffect)
        {
            // reset particle animation
            pParticle->m_fValue = 0.0f;

            // remove particle
            DYN_REMOVE(it, m_apRenderList)
        }
        else DYN_KEEP(it)
    }
}


// ****************************************************************
// remove all particle effect objects
void coreParticleSystem::UnbindAll()
{
    FOR_EACH(it, m_apRenderList)
    {
        coreParticle*       pParticle = (*it);
        const coreObject3D* pOrigin   = pParticle->GetEffect()->GetOrigin();

        // check origin object and transform position
        if(pOrigin)
        {
            pParticle->m_BeginState.vPosition += pOrigin->GetPosition();
            pParticle->m_EndState  .vPosition += pOrigin->GetPosition();
        }

        // reset associated particle effect object
        pParticle->m_pEffect = m_pDefaultEffect;
    }
}


// ****************************************************************
// remove all particles
void coreParticleSystem::ClearAll()
{
    // reset all particle animations
    FOR_EACH(it, m_apRenderList)
        (*it)->m_fValue = 0.0f;

    // clear memory
    m_apRenderList.clear();
}


// ****************************************************************
// reset with the resource manager
void coreParticleSystem::__Reset(const coreResourceReset bInit)
{
    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_uniform_buffer_object) || !CORE_GL_SUPPORT(ARB_vertex_array_object) || !CORE_GL_SUPPORT(ARB_half_float_vertex)) return;

    if(bInit)
    {
        FOR_EACH(it, m_aiInstanceBuffer)
        {
            // create vertex array objects
            coreGenVertexArrays(1u, &m_aiVertexArray.current());
            glBindVertexArray(m_aiVertexArray.current());
            m_aiVertexArray.next();

            // create instance data buffers
            it->Create(m_iNumParticles, CORE_PARTICLE_INSTANCE_SIZE, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC | CORE_DATABUFFER_STORAGE_FENCED);
            it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 3u, GL_FLOAT,         false, 0u);
            it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM,     4u, GL_HALF_FLOAT,    false, 3u*sizeof(coreFloat));
            it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    4u, GL_UNSIGNED_BYTE, false, 3u*sizeof(coreFloat) + 2u*sizeof(coreUint32));

            // set vertex data
            Core::Manager::Object->GetLowQuad()->GetVertexBuffer(0u)->Activate(0u);
            it->ActivateDivided(1u, 1u);
        }

        // disable current model object (to fully enable the next model)
        coreModel::Disable(false);

        // invoke buffer update
        m_bUpdate = true;
    }
    else
    {
        // delete vertex array objects
        glDeleteVertexArrays(3, m_aiVertexArray.data());
        m_aiVertexArray.fill(0u);

        // delete instance data buffers
        FOR_EACH(it, m_aiInstanceBuffer) it->Delete();

        // reset selected array and buffer (to synchronize)
        m_aiVertexArray   .select(0u);
        m_aiInstanceBuffer.select(0u);
    }
}


// ****************************************************************
// constructor
coreParticleEffect::coreParticleEffect(coreParticleSystem* pSystem)noexcept
: m_fCreation (0.0f)
, m_iTimeID   (-1)
, m_pOrigin   (NULL)
, m_pSystem   (pSystem)
, m_pThis     (pSystem ? pSystem->GetDefaultEffect() : NULL)
{
}

coreParticleEffect::coreParticleEffect(const coreParticleEffect& c)noexcept
: m_fCreation (c.m_fCreation)
, m_iTimeID   (c.m_iTimeID)
, m_pOrigin   (c.m_pOrigin)
, m_pSystem   (c.m_pSystem)
, m_pThis     (NULL)
{
    m_pThis = c.IsDynamic() ? this : c.m_pThis;
}


// ****************************************************************
// destructor
coreParticleEffect::~coreParticleEffect()
{
    // unbind all dynamic particles
    if(this->IsDynamic())
        m_pSystem->Unbind(this);
}


// ****************************************************************
// assignment operations
coreParticleEffect& coreParticleEffect::operator = (const coreParticleEffect& c)noexcept
{
    // unbind all dynamic particles (if necessary)
    if(this->IsDynamic() && (m_pSystem != c.m_pSystem))
        m_pSystem->Unbind(this);

    // copy properties
    m_fCreation = c.m_fCreation;
    m_iTimeID   = c.m_iTimeID;
    m_pOrigin   = c.m_pOrigin;
    m_pSystem   = c.m_pSystem;
    m_pThis     = c.IsDynamic() ? this : c.m_pThis;

    return *this;
}


// ****************************************************************
// change associated particle system object
void coreParticleEffect::ChangeSystem(coreParticleSystem* pSystem, const coreBool bUnbind)
{
    // check for dynamic behavior
    if(this->IsDynamic())
    {
        // unbind old particles (not unbinding them may cause crash if not handled)
        if(bUnbind) m_pSystem->Unbind(this);
    }
    else m_pThis = pSystem ? pSystem->GetDefaultEffect() : NULL;

    // set new particle system object
    m_pSystem = pSystem;
}
