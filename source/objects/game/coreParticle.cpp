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
/* default particle animation function */
static coreParticle::coreAnim DefaultAnimateFunc(const coreParticle& oParticle, void* pData)
{
    coreParticle::coreAnim oAnim;

    oAnim.vPosition = LERP(oParticle.GetEndState().vPosition,   oParticle.GetBeginState().vPosition,   oParticle.GetValue());
    oAnim.fScale    = LERP(oParticle.GetEndState().fScale,      oParticle.GetBeginState().fScale,      oParticle.GetValue());
    oAnim.fAngle    = LERP(oParticle.GetEndState().fAngle,      oParticle.GetBeginState().fAngle,      oParticle.GetValue());
    oAnim.vColor    = LERP(oParticle.GetEndState().GetColor4(), oParticle.GetBeginState().GetColor4(), oParticle.GetValue());

    return oAnim;
}


// ****************************************************************
/* constructor */
coreParticleSystem::coreParticleSystem(const coreUint32 iStartSize)noexcept
: coreResourceRelation ()
, m_aParticle          {}
, m_iNumParticles      (iStartSize)
, m_iCurParticle       (0u)
, m_apTexture          {}
, m_pProgram           (NULL)
, m_apRenderList       {}
, m_pDefaultEffect     (NULL)
, m_aiVertexArray      {}
, m_aInstanceBuffer    {}
, m_pAnimateData       (NULL)
, m_nAnimateFunc       (DefaultAnimateFunc)
, m_bUpdate            (false)
{
    // pre-allocate particles
    m_aParticle.resize(iStartSize);

    // create default particle effect object
    m_pDefaultEffect = new coreParticleEffect(this);
    m_pDefaultEffect->m_pThis = m_pDefaultEffect;

    // create vertex array objects and instance data buffers
    m_aiVertexArray.fill(0u);
    if(iStartSize) this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
/* destructor */
coreParticleSystem::~coreParticleSystem()
{
    // delete particles
    this->ClearAll();
    m_aParticle.clear();

    // delete default particle effect object
    SAFE_DELETE(m_pDefaultEffect)

    // delete vertex array objects and instance data buffers
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
}


// ****************************************************************
/* undefine the visual appearance */
void coreParticleSystem::Undefine()
{
    // reset all resource and memory pointers
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i) m_apTexture[i] = NULL;
    m_pProgram = NULL;
}


// ****************************************************************
/* render the particle system */
void coreParticleSystem::Render()
{
    if(m_apRenderList.empty()) return;

    // enable the shader-program
    ASSERT(m_pProgram)
    if(!m_pProgram.IsUsable()) return;
    if(!m_pProgram->Enable())  return;

    // enable all active textures
    coreTexture::EnableAll(m_apTexture);

    if(m_aInstanceBuffer[0].IsValid())
    {
        if(m_bUpdate)
        {
            // invalidate and synchronize previous buffer
            m_aInstanceBuffer.current().Invalidate();
            m_aInstanceBuffer.current().Synchronize(CORE_DATABUFFER_MAP_INVALIDATE_ALL);

            // switch to next available array and buffer
            m_aiVertexArray  .next();
            m_aInstanceBuffer.next();

            if(CORE_GL_SUPPORT(ARB_half_float_vertex))
            {
                // map required area of the instance data buffer
                coreByte* pRange  = m_aInstanceBuffer.current().MapWrite(0u, m_apRenderList.size() * CORE_PARTICLE_INSTANCE_SIZE_HIGH, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
                coreByte* pCursor = pRange;

                FOR_EACH_REV(it, m_apRenderList)
                {
                    // get current particle state
                    const coreParticle* pParticle = (*it);
                    const coreObject3D* pOrigin   = pParticle->GetEffect()->GetOrigin();

                    // animate particle
                    const coreParticle::coreAnim oAnim = m_nAnimateFunc(*pParticle, m_pAnimateData);

                    // compress data
                    const coreVector3 vPosition = pOrigin ? (pOrigin->GetPosition() + oAnim.vPosition) : oAnim.vPosition;
                    const coreUint64  iData     = coreVector4(oAnim.fScale, oAnim.fAngle, pParticle->GetValue(), 0.0f).PackFloat4x16();
                    const coreUint32  iColor    = oAnim.vColor.PackUnorm4x8();
                    ASSERT((oAnim.vColor.Min() >= 0.0f) && (oAnim.vColor.Max() <= 1.0f))

                    // write data to the buffer
                    std::memcpy(pCursor,       &vPosition, sizeof(coreVector3));
                    std::memcpy(pCursor + 12u, &iData,     sizeof(coreUint64));
                    std::memcpy(pCursor + 20u, &iColor,    sizeof(coreUint32));
                    pCursor += CORE_PARTICLE_INSTANCE_SIZE_HIGH;
                }

                ASSERT(coreUint32(pCursor - pRange) == m_apRenderList.size() * CORE_PARTICLE_INSTANCE_SIZE_HIGH)
            }
            else
            {
                // map required area of the instance data buffer
                coreByte* pRange  = m_aInstanceBuffer.current().MapWrite(0u, m_apRenderList.size() * CORE_PARTICLE_INSTANCE_SIZE_LOW, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
                coreByte* pCursor = pRange;

                FOR_EACH_REV(it, m_apRenderList)
                {
                    // get current particle state
                    const coreParticle* pParticle = (*it);
                    const coreObject3D* pOrigin   = pParticle->GetEffect()->GetOrigin();

                    // animate particle
                    const coreParticle::coreAnim oAnim = m_nAnimateFunc(*pParticle, m_pAnimateData);

                    // compress data
                    const coreVector3 vPosition = pOrigin ? (pOrigin->GetPosition() + oAnim.vPosition) : oAnim.vPosition;
                    const coreVector3 vData     = coreVector3(oAnim.fScale, oAnim.fAngle, pParticle->GetValue());
                    const coreUint32  iColor    = oAnim.vColor.PackUnorm4x8();
                    ASSERT((oAnim.vColor.Min() >= 0.0f) && (oAnim.vColor.Max() <= 1.0f))

                    // write data to the buffer
                    std::memcpy(pCursor,       &vPosition, sizeof(coreVector3));
                    std::memcpy(pCursor + 12u, &vData,     sizeof(coreVector3));
                    std::memcpy(pCursor + 24u, &iColor,    sizeof(coreUint32));
                    pCursor += CORE_PARTICLE_INSTANCE_SIZE_LOW;
                }

                ASSERT(coreUint32(pCursor - pRange) == m_apRenderList.size() * CORE_PARTICLE_INSTANCE_SIZE_LOW)
            }

            // unmap buffer
            m_aInstanceBuffer.current().Unmap();

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

            // animate particle
            const coreParticle::coreAnim oAnim = m_nAnimateFunc(*pParticle, m_pAnimateData);

            // prepare data
            const coreVector3 vPosition = pOrigin ? (pOrigin->GetPosition() + oAnim.vPosition) : oAnim.vPosition;
            const coreVector3 vData     = coreVector3(oAnim.fScale, oAnim.fAngle, pParticle->GetValue());
            const coreVector4 vColor    = oAnim.vColor;

            // update all particle uniforms
            pProgram->SendUniform(CORE_SHADER_UNIFORM_DIV_POSITION, vPosition);
            pProgram->SendUniform(CORE_SHADER_UNIFORM_DIV_DATA,     vData);
            pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,        vColor);

            // draw the model
            pModel->Enable();
            pModel->DrawArrays();
        }
    }
}


// ****************************************************************
/* move the particle system */
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
        else DYN_KEEP(it, m_apRenderList)
    }

    // set the update status
    m_bUpdate = true;
}


// ****************************************************************
/* create new particle */
coreParticle* coreParticleSystem::CreateParticle(coreParticleEffect* pEffect)
{
    ASSERT(pEffect)

    // loop through all particles
    for(coreUintW i = m_iNumParticles; i--; )
    {
        if(++m_iCurParticle >= m_iNumParticles) m_iCurParticle = 0u;

        // check current particle status
        coreParticle* pParticle = &m_aParticle[m_iCurParticle];
        if(!pParticle->IsActive())
        {
            // prepare particle and add to render list
            pParticle->__Prepare(pEffect);
            m_apRenderList.push_back(pParticle);

            return pParticle;
        }
    }

    // increase current size by 50%
    const coreUint32 iSize = m_iNumParticles;   // might be 0
    this->Reallocate(iSize + iSize / 2u + 1u);

    // execute again with first new particle
    m_iCurParticle = iSize - 1u;
    return this->CreateParticle(pEffect);
}


// ****************************************************************
/* unbind particles */
void coreParticleSystem::Unbind(const coreParticleEffect* pEffect)
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
/* remove particles */
void coreParticleSystem::Clear(const coreParticleEffect* pEffect)
{
    ASSERT(pEffect)

    FOR_EACH_DYN(it, m_apRenderList)
    {
        coreParticle* pParticle = (*it);

        // check particle effect object
        if(pParticle->GetEffect() == pEffect)
        {
            // reset particle state
            pParticle->Disable();

            // remove particle
            DYN_REMOVE(it, m_apRenderList)
        }
        else DYN_KEEP(it, m_apRenderList)
    }
}


// ****************************************************************
/* unbind all particles */
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
/* remove all particles */
void coreParticleSystem::ClearAll()
{
    // reset all particle states
    FOR_EACH(it, m_apRenderList)
        (*it)->Disable();

    // clear memory
    m_apRenderList.clear();
}


// ****************************************************************
/* change current size */
void coreParticleSystem::Reallocate(const coreUint32 iSize)
{
         if(iSize == m_iNumParticles)       return;
    WARN_IF(iSize <  m_apRenderList.size()) return;

    const coreUintW iBefore = P_TO_UI(m_aParticle.data());

    // change current size
    m_iNumParticles = iSize;
    m_iCurParticle  = 0u;
    m_aParticle.resize(iSize);

    const coreUintW iAfter = P_TO_UI(m_aParticle.data());

    // fix addresses for all active particles
    FOR_EACH(it, m_apRenderList)
    {
        (*it) = s_cast<coreParticle*>(I_TO_P(P_TO_UI(*it) - iBefore + iAfter));
    }

    // reallocate the instance data buffers
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
    this->__Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
/* reset with the resource manager */
void coreParticleSystem::__Reset(const coreResourceReset eInit)
{
    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_vertex_array_object)) return;

    if(eInit)
    {
        WARN_IF(m_aInstanceBuffer[0].IsValid()) return;

        // only allocate with enough size
        if(m_iNumParticles)
        {
            FOR_EACH(it, m_aInstanceBuffer)
            {
                // create vertex array objects
                coreGenVertexArrays(1u, &m_aiVertexArray.current());
                glBindVertexArray(m_aiVertexArray.current());
                m_aiVertexArray.next();

                if(CORE_GL_SUPPORT(ARB_half_float_vertex))
                {
                    // create instance data buffers (high quality compression)
                    it->Create(m_iNumParticles, CORE_PARTICLE_INSTANCE_SIZE_HIGH, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 3u, GL_FLOAT,         12u, false, 0u, 0u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM,     4u, GL_HALF_FLOAT,    8u,  false, 0u, 12u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    4u, GL_UNSIGNED_BYTE, 4u,  false, 0u, 20u);
                }
                else
                {
                    // create instance data buffers (low quality compression)
                    it->Create(m_iNumParticles, CORE_PARTICLE_INSTANCE_SIZE_LOW, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM, 3u, GL_FLOAT,         12u, false, 0u, 0u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM,     3u, GL_FLOAT,         12u, false, 0u, 12u);
                    it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,    4u, GL_UNSIGNED_BYTE, 4u,  false, 0u, 24u);
                }

                // set vertex data
                Core::Manager::Object->GetLowQuad()->GetVertexBuffer(0u)->Activate(0u);
                it->Activate(1u);
            }

            // disable current model object (to fully enable the next model)
            coreModel::Disable(false);

            // invoke buffer update
            m_bUpdate = true;
        }
    }
    else
    {
        // delete vertex array objects
        if(m_aiVertexArray[0]) coreDelVertexArrays(CORE_PARTICLE_INSTANCE_BUFFERS, m_aiVertexArray.data());
        m_aiVertexArray.fill(0u);

        // delete instance data buffers
        FOR_EACH(it, m_aInstanceBuffer) it->Delete();

        // reset selected array and buffer (to synchronize)
        m_aiVertexArray  .select(0u);
        m_aInstanceBuffer.select(0u);
    }
}


// ****************************************************************
/* constructor */
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
/* destructor */
coreParticleEffect::~coreParticleEffect()
{
    // unbind all dynamic particles
    if(this->IsDynamic())
        this->Unbind();
}


// ****************************************************************
/* assignment operations */
coreParticleEffect& coreParticleEffect::operator = (const coreParticleEffect& c)noexcept
{
    // unbind all dynamic particles (if necessary)
    if(this->IsDynamic() && (m_pSystem != c.m_pSystem))
        this->Unbind();

    // copy properties
    m_fCreation = c.m_fCreation;
    m_iTimeID   = c.m_iTimeID;
    m_pOrigin   = c.m_pOrigin;
    m_pSystem   = c.m_pSystem;
    m_pThis     = c.IsDynamic() ? this : c.m_pThis;

    return *this;
}


// ****************************************************************
/* change associated particle system object */
void coreParticleEffect::ChangeSystem(coreParticleSystem* pSystem, const coreBool bUnbind)
{
    // check for dynamic behavior
    if(this->IsDynamic())
    {
        // unbind old particles (not unbinding them may cause crash if not handled)
        if(bUnbind) this->Unbind();
    }
    else m_pThis = pSystem ? pSystem->GetDefaultEffect() : NULL;

    // set new particle system object
    m_pSystem = pSystem;
}
