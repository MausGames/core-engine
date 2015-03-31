//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_PARTICLE_H_
#define _CORE_GUARD_PARTICLE_H_

// TODO: what about texture size and offset ? (make different base particles ? generic, performance, more coffee, solve in shader!)
// TODO: what about velocity
// TODO: texture arrays and texture index parameter to allow different objects to be rendered
// TODO: SSBO[index] really faster than a divisor ? check also for their use instead of VAO in general
// TODO: discard every X particle (create min 1) on lower systems ?
// TODO: high systems: currently CPU(move) is bottleneck, look for improvement with transform feedback(3.0) or compute shader(4.0)
// TODO: low systems: merge geometry to reduce draw calls
// TODO: try same sort-algorithm proposed for instance list, no time-sort, but position-sort
// TODO: automatic resizing function (preserve old values)
// TODO: culling (also on instancing)
// TODO: fix invalid returned particle (e.g. wrong effect, sort) when no free particle is available


// ****************************************************************
// particle definitions
#define CORE_PARTICLE_INSTANCE_SIZE    (2u*sizeof(coreVector3) + 1u*sizeof(coreUint32))   //!< instancing per-particle size (position, data, color)
#define CORE_PARTICLE_INSTANCE_BUFFERS (3u)                                               //!< number of concurrent instance data buffer


// ****************************************************************
// particle class
class coreParticle final
{
public:
    //! state structure
    struct coreState
    {
        coreVector3 vPosition;   //!< position of the particle
        coreFloat   fScale;      //!< scale-factor of the particle
        coreFloat   fAngle;      //!< orientation-angle of the particle
        coreVector4 vColor;      //!< RGBA color-value

        constexpr_func coreState()noexcept;
    };


private:
    coreState m_CurrentState;        //!< current calculated state
    coreState m_MoveState;           //!< difference between initial and final state

    coreFloat m_fValue;              //!< current animation value of the particle (between 0.0f and 1.0f)
    coreFloat m_fSpeed;              //!< speed factor of the particle

    coreParticleEffect* m_pEffect;   //!< associated particle effect object


private:
    constexpr_func coreParticle()noexcept;
    ~coreParticle() {}


public:
    FRIEND_CLASS(coreParticleSystem)
    ENABLE_COPY (coreParticle)

    //! check current status
    //! @{
    inline coreBool IsActive()const {return (m_fValue > 0.0f) ? true : false;}
    //! @}

    //! animate the particle relative
    //! @{
    inline void SetPositionRel(const coreVector3& vStart, const coreVector3& vMove) {m_CurrentState.vPosition = vStart; m_MoveState.vPosition = vMove;}
    inline void SetScaleRel   (const coreFloat&   fStart, const coreFloat&   fMove) {m_CurrentState.fScale    = fStart; m_MoveState.fScale    = fMove;}
    inline void SetAngleRel   (const coreFloat&   fStart, const coreFloat&   fMove) {m_CurrentState.fAngle    = fStart; m_MoveState.fAngle    = fMove;}
    inline void SetColor4Rel  (const coreVector4& vStart, const coreVector4& vMove) {m_CurrentState.vColor    = vStart; m_MoveState.vColor    = vMove;}
    //! @}

    //! animate the particle absolute
    //! @{
    inline void SetPositionAbs(const coreVector3& vStart, const coreVector3& vEnd) {this->SetPositionRel(vStart, vEnd - vStart);}
    inline void SetScaleAbs   (const coreFloat&   fStart, const coreFloat&   fEnd) {this->SetScaleRel   (fStart, fEnd - fStart);}
    inline void SetAngleAbs   (const coreFloat&   fStart, const coreFloat&   fEnd) {this->SetAngleRel   (fStart, fEnd - fStart);}
    inline void SetColor4Abs  (const coreVector4& vStart, const coreVector4& vEnd) {this->SetColor4Rel  (vStart, vEnd - vStart);}
    //! @}

    //! animate the particle static
    //! @{
    inline void SetPositionStc(const coreVector3& vStatic) {this->SetPositionRel(vStatic, coreVector3(0.0f,0.0f,0.0f));}
    inline void SetScaleStc   (const coreFloat&   fStatic) {this->SetScaleRel   (fStatic, 0.0f);}
    inline void SetAngleStc   (const coreFloat&   fStatic) {this->SetAngleRel   (fStatic, 0.0f);}
    inline void SetColor4Stc  (const coreVector4& vStatic) {this->SetColor4Rel  (vStatic, coreVector4(0.0f,0.0f,0.0f,0.0f));}
    //! @}

    //! set object properties
    //! @{
    inline void SetSpeed   (const coreFloat& fSpeed)    {m_fSpeed = fSpeed;}
    inline void SetLifetime(const coreFloat& fLifetime) {m_fSpeed = RCP(fLifetime);}
    //! @}

    //! get object properties
    //! @{
    inline const coreState&    GetCurrentState()const {return m_CurrentState;}
    inline const coreState&    GetMoveState   ()const {return m_MoveState;}
    inline const coreFloat&    GetValue       ()const {return m_fValue;}
    inline const coreFloat&    GetSpeed       ()const {return m_fSpeed;}
    inline       coreFloat     GetLifetime    ()const {return RCP(m_fSpeed);}
    inline coreParticleEffect* GetEffect      ()const {return m_pEffect;}
    //! @}


private:
    //! control the particle
    //! @{
    inline void __Prepare(coreParticleEffect* pEffect) {m_pEffect = pEffect; m_fValue = 1.0f;}
    inline void __Update();
    //! @}
};


// ****************************************************************
// particle system class
class coreParticleSystem final : public coreResourceRelation
{
private:
    coreParticle* m_pParticle;                                                         //!< pre-allocated particles
    coreUint32 m_iNumParticles;                                                        //!< number of particles
    coreUint32 m_iCurParticle;                                                         //!< current particle

    coreTexturePtr m_apTexture[CORE_TEXTURE_UNITS];                                    //!< multiple texture objects
    coreProgramPtr m_pProgram;                                                         //!< shader-program object

    std::list<coreParticle*> m_apRenderList;                                           //!< sorted render list with active particles
    coreParticleEffect* m_pEmptyEffect;                                                //!< empty particle effect object (dynamic, because of class order)

    coreSelect<GLuint,           CORE_PARTICLE_INSTANCE_BUFFERS> m_aiVertexArray;      //!< vertex array objects
    coreSelect<coreVertexBuffer, CORE_PARTICLE_INSTANCE_BUFFERS> m_aiInstanceBuffer;   //!< instance data buffers

    coreBool m_bUpdate;                                                                //!< buffer update status (dirty flag)


public:
    explicit coreParticleSystem(const coreUint32& iNumParticles)noexcept;
    ~coreParticleSystem();

    DISABLE_COPY(coreParticleSystem)

    //! define the visual appearance
    //! @{
    inline const coreTexturePtr& DefineTexture(const coreUintW& iUnit, const coreTexturePtr& pTexture) {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = pTexture;                                          return m_apTexture[iUnit];}
    inline const coreTexturePtr& DefineTexture(const coreUintW& iUnit, const coreChar*       pcName)   {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = Core::Manager::Resource->Get<coreTexture>(pcName); return m_apTexture[iUnit];}
    inline const coreProgramPtr& DefineProgram(const coreProgramPtr& pProgram)                         {m_pProgram = pProgram;                                          return m_pProgram;}
    inline const coreProgramPtr& DefineProgram(const coreChar*       pcName)                           {m_pProgram = Core::Manager::Resource->Get<coreProgram>(pcName); return m_pProgram;}
    void Undefine();
    //! @}

    //! render and move the particle system
    //! @{
    void Render();
    void Move  ();
    //! @}

    //! create new particles
    //! @{
    coreParticle*        CreateParticle(coreParticleEffect* pEffect);
    inline coreParticle* CreateParticle() {return this->CreateParticle(m_pEmptyEffect);}
    //! @}

    //! remove particle effect objects and particles
    //! @{
    void Unbind(coreParticleEffect* pEffect);
    void Clear (coreParticleEffect* pEffect);
    void UnbindAll();
    void ClearAll ();
    //! @}

    //! get object properties
    //! @{
    inline const coreTexturePtr& GetTexture           (const coreUintW& iUnit)const {ASSERT(iUnit < CORE_TEXTURE_UNITS) return m_apTexture[iUnit];}
    inline const coreProgramPtr& GetProgram           ()const                       {return m_pProgram;}
    inline const coreUint32&     GetNumParticles      ()const                       {return m_iNumParticles;}
    inline       coreUintW       GetNumActiveParticles()const                       {return m_apRenderList.size();}
    inline coreParticleEffect*   GetEmptyEffect       ()const                       {return m_pEmptyEffect;}
    //! @}


private:
    //! reset with the resource manager
    //! @{
    void __Reset(const coreResourceReset& bInit)override;
    //! @}
};


// ****************************************************************
// particle effect class
class coreParticleEffect final
{
private:
    coreFlow m_fCreation;            //!< status value for particle creation

    coreInt8      m_iTimeID;         //!< ID of the used frame time
    coreObject3D* m_pOrigin;         //!< origin object for relative movement

    coreParticleSystem* m_pSystem;   //!< associated particle system object
    coreParticleEffect* m_pThis;     //!< pointer to foreign empty object or to itself (to improve destructor performance)


public:
    explicit coreParticleEffect(coreParticleSystem* pSystem)noexcept;
    ~coreParticleEffect();

    DISABLE_COPY(coreParticleEffect)

    //! create new particles
    //! @{
    template <typename F> void CreateParticle(const coreUintW& iNum, const coreFloat& fFrequency, F&& nFunction);
    template <typename F> void CreateParticle(const coreUintW& iNum,                              F&& nFunction);
    inline coreParticle* CreateParticle() {return m_pSystem->CreateParticle(m_pThis);}
    //! @}

    //! change associated particle system object
    //! @{
    void ChangeSystem(coreParticleSystem* pSystem, const coreBool& bUnbind);
    //! @}

    //! check dynamic behavior
    //! @{
    inline coreBool IsDynamic()const {return (m_pThis == this) ? true : false;}
    //! @}

    //! set object properties
    //! @{
    inline void SetTimeID(const coreInt8& iTimeID) {m_pThis = this; m_iTimeID = iTimeID;}
    inline void SetOrigin(coreObject3D*   pOrigin) {m_pThis = this; m_pOrigin = pOrigin;}
    //! @}

    //! get object properties
    //! @{
    inline const coreInt8&     GetTimeID()const {return m_iTimeID;}
    inline coreObject3D*       GetOrigin()const {return m_pOrigin;}
    inline coreParticleSystem* GetSystem()const {return m_pSystem;}
    //! @}
};


// ****************************************************************
// constructor
constexpr_func coreParticle::coreState::coreState()noexcept
: vPosition (coreVector3(0.0f,0.0f,0.0f))
, fScale    (0.0f)
, fAngle    (0.0f)
, vColor    (coreVector4(1.0f,1.0f,1.0f,1.0f))
{
}


// ****************************************************************
// constructor
constexpr_func coreParticle::coreParticle()noexcept
: m_fValue  (0.0f)
, m_fSpeed  (1.0f)
, m_pEffect (NULL)
{
}


// ****************************************************************
// update the particle
inline void coreParticle::__Update()
{
    ASSERT(m_pEffect)

    // update current animation value
    const coreFloat fTime = m_fSpeed * Core::System->GetTime(m_pEffect->GetTimeID());
    m_fValue -= fTime;

    // update current state
    m_CurrentState.vPosition += m_MoveState.vPosition * fTime;
    m_CurrentState.fScale    += m_MoveState.fScale    * fTime;
    m_CurrentState.fAngle    += m_MoveState.fAngle    * fTime;
    m_CurrentState.vColor    += m_MoveState.vColor    * fTime;
}


// ****************************************************************
// create new particles
template <typename F> void coreParticleEffect::CreateParticle(const coreUintW& iNum, const coreFloat& fFrequency, F&& nFunction)
{
    ASSERT(fFrequency <= 60.0f)

    // update and check status value
    m_fCreation.Update(fFrequency, m_iTimeID);
    if(m_fCreation >= 1.0f)
    {
        // adjust status value
        m_fCreation -= FLOOR(m_fCreation);

        // create particles and call function
        for(coreUintW i = iNum; i--; )
            nFunction(this->CreateParticle());
    }
}

template <typename F> void coreParticleEffect::CreateParticle(const coreUintW& iNum, F&& nFunction)
{
    // create particles and call function
    for(coreUintW i = iNum; i--; )
        nFunction(this->CreateParticle());
}


#endif // _CORE_GUARD_PARTICLE_H_