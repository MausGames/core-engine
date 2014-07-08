//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#define CORE_PARTICLE_SIZE (2*sizeof(coreVector3) + 1*sizeof(coreUint))

coreModel* coreParticleSystem::s_pModel = NULL;


// ****************************************************************
// constructor
coreParticleSystem::coreParticleSystem(const coreUint& iSize)noexcept
: m_iNumParticle (iSize)
, m_iCurParticle (0)
, m_iVertexArray (0)
, m_bUpdate      (false)
{
    ASSERT(iSize)

    // pre-allocate particles
    m_pParticle = new coreParticle[iSize];

    // create empty particle effect object
    m_pEmptyEffect = new coreParticleEffect(this);

    // create vertex array object and instance data buffer
    this->__Reset(true);
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
    this->__Reset(false);
}


// ****************************************************************
// define texture through resource file
const coreTexturePtr& coreParticleSystem::DefineTextureFile(const coreByte& iUnit, const char* pcPath)
{
    ASSERT(iUnit < CORE_TEXTURE_UNITS)

    // set and return texture object
    m_apTexture[iUnit] = Core::Manager::Resource->LoadFile<coreTexture>(pcPath);
    return m_apTexture[iUnit];
}


// ****************************************************************
// define texture through linked resource
const coreTexturePtr& coreParticleSystem::DefineTextureLink(const coreByte& iUnit, const char* pcName)
{
    ASSERT(iUnit < CORE_TEXTURE_UNITS)

    // set and return texture object
    m_apTexture[iUnit] = Core::Manager::Resource->LoadLink<coreTexture>(pcName);
    return m_apTexture[iUnit];
}


// ****************************************************************
// define shader-program through shared memory
const coreProgramShr& coreParticleSystem::DefineProgramShare(const char* pcName)
{
    // set shader-program object
    m_pProgram = Core::Manager::Memory->Share<coreProgram>(pcName);

    if(m_iInstanceBuffer)
    {
        // bind particle-specific attributes
        m_pProgram->BindAttribute(CORE_PARTICLE_ATTRIBUTE_POSITION, CORE_PARTICLE_ATTRIBUTE_POSITION_NUM);
        m_pProgram->BindAttribute(CORE_PARTICLE_ATTRIBUTE_DATA,     CORE_PARTICLE_ATTRIBUTE_DATA_NUM);
        m_pProgram->BindAttribute(CORE_PARTICLE_ATTRIBUTE_COLOR,    CORE_PARTICLE_ATTRIBUTE_COLOR_NUM);
    }

    return m_pProgram;
}


// ****************************************************************
// undefine the visual appearance
void coreParticleSystem::Undefine()
{
    // reset all resource and memory pointer
    for(int i = 0; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
}


// ****************************************************************
// render the particle system
void coreParticleSystem::Render()
{
    if(m_apRenderList.empty()) return;

    // enable the shader-program
    if(!m_pProgram) return;
    if(!m_pProgram->Enable()) return;

    // update normal matrix uniform
    m_pProgram->SendUniform(CORE_SHADER_UNIFORM_3D_NORMAL, Core::Graphics->GetCamera().m123().Invert(), false);

    // enable all active textures
    for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
        if(m_apTexture[i].IsActive()) m_apTexture[i]->Enable(i);

    if(m_iInstanceBuffer)
    {
        // reset current model object
        coreModel::Disable(false);

        if(m_bUpdate)
        {
            // invalidate the instance data buffer
            m_iInstanceBuffer.Invalidate();

            coreModel::Lock();
            {
                // map required area of the instance data buffer
                const coreUint iLength = m_apRenderList.size()*CORE_PARTICLE_SIZE;
                coreByte*      pRange  = m_iInstanceBuffer.Map<coreByte>(0, iLength, true);

                FOR_EACH_REV(it, m_apRenderList)
                {
                    // get current particle state
                    const coreParticle*            pParticle = (*it);
                    const coreObject3D*            pOrigin   = pParticle->GetEffect()->GetOrigin();
                    const coreParticle::coreState& Current   = pParticle->GetCurrentState();

                    // write position data to the buffer
                    if(pOrigin)
                    {
                        const coreVector3 vPosition = pOrigin->GetPosition() + Current.vPosition;
                        std::memcpy(pRange, &vPosition, sizeof(coreVector3));
                    }
                    else std::memcpy(pRange, &Current.vPosition, sizeof(coreVector3));

                    // compress remaining data
                    const coreVector3 vData  = coreVector3(Current.fScale, Current.fAngle, pParticle->GetValue());
                    const coreUint    iColor = Current.vColor.ColorPack();

                    // write remaining data to the buffer
                    std::memcpy(pRange + 1*sizeof(coreVector3), &vData,  sizeof(coreVector3));
                    std::memcpy(pRange + 2*sizeof(coreVector3), &iColor, sizeof(coreUint));
                    pRange += CORE_PARTICLE_SIZE;
                }

                // unmap buffer
                m_iInstanceBuffer.Unmap(pRange - iLength);
            }
            coreModel::Unlock();

            // reset the update status
            m_bUpdate = false;
        }

        // draw the model instanced
        glBindVertexArray(m_iVertexArray);
        s_pModel->DrawArraysInstanced(m_apRenderList.size());
    }
    else
    {
        // draw without instancing
        FOR_EACH_REV(it, m_apRenderList)
        {
            // get current particle state
            const coreParticle*            pParticle = (*it);
            const coreObject3D*            pOrigin   = pParticle->GetEffect()->GetOrigin();
            const coreParticle::coreState& Current   = pParticle->GetCurrentState();

            // update all particle uniforms
            m_pProgram->SendUniform(CORE_PARTICLE_UNIFORM_POSITION, pOrigin ? (pOrigin->GetPosition() + Current.vPosition) : Current.vPosition);
            m_pProgram->SendUniform(CORE_PARTICLE_UNIFORM_DATA,     coreVector3(Current.fScale, Current.fAngle, pParticle->GetValue()));
            m_pProgram->SendUniform(CORE_PARTICLE_UNIFORM_COLOR,    Current.vColor);
            
            // draw the model
            s_pModel->Enable();
            s_pModel->DrawArrays();
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
    for(coreUint i = 0; i < m_iNumParticle; ++i)
    {
        if(++m_iCurParticle >= m_iNumParticle) m_iCurParticle = 0;

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

    ASSERT(false)
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
void coreParticleSystem::__Reset(const bool& bInit)
{
    // check for OpenGL extensions
    if(!GLEW_ARB_instanced_arrays || !GLEW_ARB_vertex_array_object) return;

    if(bInit)
    {
        // reset current model object
        coreModel::Disable(false);

        // create vertex array object
        glGenVertexArrays(1, &m_iVertexArray);
        glBindVertexArray(m_iVertexArray);

        coreModel::Lock();
        {
            // create instance data buffer
            m_iInstanceBuffer.Create(m_iNumParticle, CORE_PARTICLE_SIZE, NULL, GL_DYNAMIC_DRAW);
            m_iInstanceBuffer.DefineAttribute(CORE_PARTICLE_ATTRIBUTE_POSITION_NUM, 3, GL_FLOAT,        0);
            m_iInstanceBuffer.DefineAttribute(CORE_PARTICLE_ATTRIBUTE_DATA_NUM,     3, GL_FLOAT,        3*sizeof(float));
            m_iInstanceBuffer.DefineAttribute(CORE_PARTICLE_ATTRIBUTE_COLOR_NUM,    1, GL_UNSIGNED_INT, 6*sizeof(float));

            // set vertex data
            s_pModel->GetVertexBuffer(0)->Activate(0);
            m_iInstanceBuffer.Activate(1);
        }
        coreModel::Unlock();

        // enable vertex attribute array division
        if(GLEW_ARB_vertex_attrib_binding) glVertexBindingDivisor(1, 1);
        else
        {
            glVertexAttribDivisorARB(CORE_PARTICLE_ATTRIBUTE_POSITION_NUM, 1);
            glVertexAttribDivisorARB(CORE_PARTICLE_ATTRIBUTE_DATA_NUM,     1);
            glVertexAttribDivisorARB(CORE_PARTICLE_ATTRIBUTE_COLOR_NUM,    1);
        }

        // invoke buffer update
        m_bUpdate = true;
    }
    else
    {
        // delete vertex array object
        glDeleteVertexArrays(1, &m_iVertexArray);
        m_iVertexArray = 0;

        // delete instance data buffer
        m_iInstanceBuffer.Delete();
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