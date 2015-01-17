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
coreParticleSystem::coreParticleSystem(const coreUint& iNumParticles)noexcept
: m_iNumParticles (iNumParticles)
, m_iCurParticle  (0)
, m_aiVertexArray (0)
, m_bUpdate       (false)
{
    ASSERT(iNumParticles)

    // pre-allocate particles
    m_pParticle = new coreParticle[m_iNumParticles];

    // create empty particle effect object
    m_pEmptyEffect = new coreParticleEffect(this);

    // create vertex array object and instance data buffer
    this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
// destructor
coreParticleSystem::~coreParticleSystem()
{
    // delete particles
    this->ClearAll();
    SAFE_DELETE_ARRAY(m_pParticle)

    // delete empty particle effect object
    SAFE_DELETE(m_pEmptyEffect)

    // delete vertex array object and instance data buffer
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
}


// ****************************************************************
// undefine the visual appearance
void coreParticleSystem::Undefine()
{
    // reset all resource and memory pointers
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
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
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i)
        if(m_apTexture[i].IsUsable()) m_apTexture[i]->Enable(i);

    if(m_aiInstanceBuffer[0])
    {
        if(m_bUpdate)
        {
            // switch to next available array and buffer
            m_aiVertexArray.Next();
            m_aiInstanceBuffer.Next();

            // map required area of the instance data buffer
            coreByte* pRange  = m_aiInstanceBuffer.GetCur().Map<coreByte>(0, coreUint(m_apRenderList.size()) * CORE_PARTICLE_INSTANCE_SIZE, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);
            coreByte* pCursor = pRange;

            FOR_EACH_REV(it, m_apRenderList)
            {
                // get current particle state
                const coreParticle*            pParticle = (*it);
                const coreObject3D*            pOrigin   = pParticle->GetEffect()->GetOrigin();
                const coreParticle::coreState& oCurrent  = pParticle->GetCurrentState();

                // write position data to the buffer
                if(pOrigin)
                {
                    const coreVector3 vPosition = pOrigin->GetPosition() + oCurrent.vPosition;
                    std::memcpy(pCursor, &vPosition, sizeof(coreVector3));
                }
                else std::memcpy(pCursor, &oCurrent.vPosition, sizeof(coreVector3));

                // compress remaining data
                const coreVector3 vData  = coreVector3(oCurrent.fScale, oCurrent.fAngle, pParticle->GetValue());
                const coreUint    iColor = oCurrent.vColor.PackUnorm4x8();
                ASSERT(oCurrent.vColor.Min() >= 0.0f && oCurrent.vColor.Max() <= 1.0f)

                // write remaining data to the buffer
                std::memcpy(pCursor + 1*sizeof(coreVector3), &vData,  sizeof(coreVector3));
                std::memcpy(pCursor + 2*sizeof(coreVector3), &iColor, sizeof(coreUint));
                pCursor += CORE_PARTICLE_INSTANCE_SIZE;
            }

            // unmap buffer
            m_aiInstanceBuffer.GetCur().Unmap(pRange);

            // reset the update status
            m_bUpdate = false;
        }

        // disable current model object (because of direct VAO use)
        coreModel::Disable(false);

        // draw the model instanced
        glBindVertexArray(m_aiVertexArray.GetCur());
        Core::Manager::Object->GetLowModel()->DrawArraysInstanced(coreUint(m_apRenderList.size()));
    }
    else
    {
        // draw without instancing
        FOR_EACH_REV(it, m_apRenderList)
        {
            // get current particle state
            const coreParticle*            pParticle = (*it);
            const coreObject3D*            pOrigin   = pParticle->GetEffect()->GetOrigin();
            const coreParticle::coreState& oCurrent  = pParticle->GetCurrentState();

            // update all particle uniforms
            m_pProgram->SendUniform(CORE_SHADER_ATTRIBUTE_DIV_POSITION, pOrigin ? (pOrigin->GetPosition() + oCurrent.vPosition) : oCurrent.vPosition);
            m_pProgram->SendUniform(CORE_SHADER_ATTRIBUTE_DIV_DATA,     coreVector3(oCurrent.fScale, oCurrent.fAngle, pParticle->GetValue()));
            m_pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,          oCurrent.vColor);

            // draw the model
            Core::Manager::Object->GetLowModel()->Enable();
            Core::Manager::Object->GetLowModel()->DrawArrays();
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
        pParticle->Update();
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
    for(coreUint i = 0; i < m_iNumParticles; ++i)
    {
        if(++m_iCurParticle >= m_iNumParticles) m_iCurParticle = 0;

        // check current particle status
        coreParticle* pParticle = &m_pParticle[m_iCurParticle];
        if(!pParticle->IsActive())
        {
            // prepare particle and add to render list
            pParticle->Prepare(pEffect);
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
            // check origin object and transform position
            if(pEffect->GetOrigin()) pParticle->m_CurrentState.vPosition += pEffect->GetOrigin()->GetPosition();

            // reset associated particle effect object
            pParticle->m_pEffect = m_pEmptyEffect;
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
        coreParticle* pParticle = (*it);
        coreParticleEffect* pEffect = pParticle->GetEffect();

        // check origin object and transform position
        if(pEffect->GetOrigin()) pParticle->m_CurrentState.vPosition += pEffect->GetOrigin()->GetPosition();

        // reset associated particle effect object
        pParticle->m_pEffect = m_pEmptyEffect;
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
void coreParticleSystem::__Reset(const coreResourceReset& bInit)
{
    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_uniform_buffer_object) || !CORE_GL_SUPPORT(ARB_vertex_array_object)) return;

    if(bInit)
    {
        FOR_EACH(it, *m_aiInstanceBuffer.List())
        {
            // create vertex array objects
            glGenVertexArrays(1, &m_aiVertexArray.GetCur());
            glBindVertexArray(m_aiVertexArray.GetCur());
            m_aiVertexArray.Next();

            // create instance data buffers
            it->Create(m_iNumParticles, CORE_PARTICLE_INSTANCE_SIZE, NULL, CORE_DATABUFFER_STORAGE_PERSISTENT | CORE_DATABUFFER_STORAGE_FENCED);
            it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 3, GL_FLOAT,         0);
            it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM,     3, GL_FLOAT,         3*sizeof(float));
            it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    4, GL_UNSIGNED_BYTE, 6*sizeof(float));

            // set vertex data
            Core::Manager::Object->GetLowModel()->GetVertexBuffer(0)->Activate(0);
            it->Activate(1);

            // enable vertex attribute array division
            if(CORE_GL_SUPPORT(ARB_vertex_attrib_binding)) glVertexBindingDivisor(1, 1);
            else
            {
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 1);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM,     1);
                glVertexAttribDivisor(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    1);
            }
        }

        // disable current model object (to fully enable the next model)
        coreModel::Disable(false);

        // invoke buffer update
        m_bUpdate = true;
    }
    else
    {
        // delete vertex array objects
        glDeleteVertexArrays(3, m_aiVertexArray);
        m_aiVertexArray.List()->fill(0);

        // delete instance data buffers
        FOR_EACH(it, *m_aiInstanceBuffer.List())
            it->Delete();
    }
}


// ****************************************************************
// constructor
coreParticleEffect::coreParticleEffect(coreParticleSystem* pSystem)noexcept
: m_fCreation (0.0f)
, m_iTimeID   (-1)
, m_pOrigin   (NULL)
, m_pSystem   (pSystem)
, m_pThis     (pSystem->GetEmptyEffect())
{
}


// ****************************************************************
// change associated particle system object
void coreParticleEffect::ChangeSystem(coreParticleSystem* pSystem, const bool& bUnbind)
{
    ASSERT(pSystem)

    // check for dynamic behavior
    if(this->IsDynamic())
    {
        // unbind old particles (not unbining them may cause crash if not handled)
        if(bUnbind) m_pSystem->Unbind(this);
    }
    else m_pThis = pSystem->GetEmptyEffect();

    // set new particle system object
    m_pSystem = pSystem;
}