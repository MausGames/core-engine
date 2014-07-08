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


// ****************************************************************
// particle definitions
#define CORE_PARTICLE_ATTRIBUTE_POSITION     "a_v3DivPosition"
#define CORE_PARTICLE_ATTRIBUTE_DATA         "a_v3DivData"
#define CORE_PARTICLE_ATTRIBUTE_COLOR        "a_iDivColor"
#define CORE_PARTICLE_ATTRIBUTE_POSITION_NUM 4
#define CORE_PARTICLE_ATTRIBUTE_DATA_NUM     5
#define CORE_PARTICLE_ATTRIBUTE_COLOR_NUM    6

#define CORE_PARTICLE_UNIFORM_POSITION       "u_v3DivPosition"
#define CORE_PARTICLE_UNIFORM_DATA           "u_v3DivData"
#define CORE_PARTICLE_UNIFORM_COLOR          "u_v4DivColor"


// ****************************************************************
// particle class
// TODO: what about texture size and offset ? (make different base particles ? generic, performance, more coffee, solve in shader!)
// TODO: what about velocity
class coreParticle final
{
public:
    //! state structure
    struct coreState
    {
        coreVector3 vPosition;   //!< position of the particle
        float fScale;            //!< scale-factor of the particle
        float fAngle;            //!< orientation-angle of the particle
        coreVector4 vColor;      //!< RGBA color-value

        constexpr_func coreState()noexcept;
    };


private:
    coreState m_CurrentState;        //!< current calculated state
    coreState m_MoveState;           //!< difference between initial and final state

    float m_fValue;                  //!< current animation value of the particle (between 0.0f and 1.0f)
    float m_fSpeed;                  //!< speed factor of the particle

    coreParticleEffect* m_pEffect;   //!< associated particle effect object


private:
    constexpr_func coreParticle()noexcept;
    ~coreParticle() {}
    friend class coreParticleSystem;

    //! control the particle
    //! @{
    inline void Prepare(coreParticleEffect* pEffect)noexcept {m_pEffect = pEffect; m_fValue = 1.0f;}
    inline void Update()noexcept;
    //! @}


public:
    //! check current status
    //! @{
    inline bool IsActive()const noexcept {return (m_fValue > 0.0f) ? true : false;}
    //! @}

    //! animate the particle relative
    //! @{
    inline void SetPositionRel(const coreVector3& vStart, const coreVector3& vMove)noexcept {m_CurrentState.vPosition = vStart; m_MoveState.vPosition = vMove;}
    inline void SetScaleRel(const float& fStart, const float& fMove)noexcept                {m_CurrentState.fScale    = fStart; m_MoveState.fScale    = fMove;}
    inline void SetAngleRel(const float& fStart, const float& fMove)noexcept                {m_CurrentState.fAngle    = fStart; m_MoveState.fAngle    = fMove;}
    inline void SetColor4Rel(const coreVector4& vStart, const coreVector4& vMove)noexcept   {m_CurrentState.vColor    = vStart; m_MoveState.vColor    = vMove;}
    //! @}

    //! animate the particle absolute
    //! @{
    inline void SetPositionAbs(const coreVector3& vStart, const coreVector3& vEnd)noexcept {this->SetPositionRel(vStart, vEnd - vStart);}
    inline void SetScaleAbs(const float& fStart, const float& fEnd)noexcept                {this->SetScaleRel   (fStart, fEnd - fStart);}
    inline void SetAngleAbs(const float& fStart, const float& fEnd)noexcept                {this->SetAngleRel   (fStart, fEnd - fStart);}
    inline void SetColor4Abs(const coreVector4& vStart, const coreVector4& vEnd)noexcept   {this->SetColor4Rel  (vStart, vEnd - vStart);}
    //! @}

    //! animate the particle static
    //! @{
    inline void SetPositionStc(const coreVector3& vStatic)noexcept {this->SetPositionRel(vStatic, coreVector3(0.0f,0.0f,0.0f));}
    inline void SetScaleStc(const float& fStatic)noexcept          {this->SetScaleRel   (fStatic, 0.0f);}
    inline void SetAngleStc(const float& fStatic)noexcept          {this->SetAngleRel   (fStatic, 0.0f);}
    inline void SetColor4Stc(const coreVector4& vStatic)noexcept   {this->SetColor4Rel  (vStatic, coreVector4(0.0f,0.0f,0.0f,0.0f));}
    //! @}

    //! set object properties
    //! @{
    inline void SetSpeed(const float& fSpeed)noexcept       {m_fSpeed = fSpeed;}
    inline void SetLifetime(const float& fLifetime)noexcept {m_fSpeed = RCP(fLifetime);}
    //! @}

    //! get object properties
    //! @{
    inline const coreState& GetCurrentState()const noexcept {return m_CurrentState;}
    inline const coreState& GetMoveState()const noexcept    {return m_MoveState;}
    inline const float& GetValue()const noexcept            {return m_fValue;}
    inline const float& GetSpeed()const noexcept            {return m_fSpeed;}
    inline float GetLifetime()const noexcept                {return RCP(m_fSpeed);}
    inline coreParticleEffect* GetEffect()const noexcept    {return m_pEffect;}
    //! @}
};


// ****************************************************************
// particle system class
// TODO: texture arrays and texture index parameter to allow different objects to be rendered
// TODO: SSBO[index] really faster than a divisior ? check also for their use instead of VAO in general
// TODO: discard every X particle (create min 1) on lower systems ?
// TODO: high systems: currently CPU(move) is bottleneck, look for improvement with transform feedback(3.0) or compute shader(4.0)
// TODO: low systems: merge geometry to reduce draw calls
class coreParticleSystem final : public coreReset
{
private:
    coreParticle* m_pParticle;                        //!< pre-allocated particles
    coreUint m_iNumParticle;                          //!< number of particles
    coreUint m_iCurParticle;                          //!< current particle

    static coreModel* s_pModel;                       //!< global model object
    coreTexturePtr m_apTexture[CORE_TEXTURE_UNITS];   //!< multiple texture objects
    coreProgramShr m_pProgram;                        //!< shader-program object

    std::list<coreParticle*> m_apRenderList;          //!< sorted render list with active particles
    coreParticleEffect* m_pEmptyEffect;               //!< empty particle effect object

    GLuint m_iVertexArray;                            //!< vertex array object

    coreVertexBuffer m_iInstanceBuffer;               //!< instance data buffer
    bool m_bUpdate;                                   //!< buffer update status


public:
    explicit coreParticleSystem(const coreUint& iSize)noexcept;
    ~coreParticleSystem();
    friend class coreObjectManager;

    //! define the visual appearance
    //! @{
    inline void DefineTexture(const coreByte& iUnit, const coreTexturePtr& pTexture) {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = pTexture;}
    inline void DefineProgram(const coreProgramShr& pProgram)                        {m_pProgram = pProgram;}
    const coreTexturePtr& DefineTextureFile(const coreByte& iUnit, const char* pcPath);
    const coreTexturePtr& DefineTextureLink(const coreByte& iUnit, const char* pcName);
    const coreProgramShr& DefineProgramShare(const char* pcName);
    void Undefine();
    //! @}

    //! render and move the particle system
    //! @{
    void Render() hot_func;
    void Move() hot_func;
    //! @}

    //! create new particles
    //! @{
    coreParticle* CreateParticle(coreParticleEffect* pEffect);
    inline coreParticle* CreateParticle() {return this->CreateParticle(m_pEmptyEffect);}
    //! @}

    //! remove particle effect objects and particles
    //! @{
    void Unbind(coreParticleEffect* pEffect);
    void UnbindAll();
    void Clear(coreParticleEffect* pEffect);
    void ClearAll();
    //! @}

    //! get object properties
    //! @{
    inline const coreUint& GetNumParticle()const     {return m_iNumParticle;}
    inline const coreUint& GetCurParticle()const     {return m_iCurParticle;}
    inline coreParticleEffect* GetEmptyEffect()const {return m_pEmptyEffect;}
    //! @}


private:
    DISABLE_COPY(coreParticleSystem)

    //! reset with the resource manager
    //! @{
    void __Reset(const bool& bInit)override;
    //! @}
};


// ****************************************************************
// particle effect class
class coreParticleEffect final
{
private:
    coreFlow m_fCreation;            //!< status value for particle creation

    int m_iTimeID;                   //!< ID of the used frame time
    coreObject3D* m_pOrigin;         //!< origin object for relative movement

    coreParticleSystem* m_pSystem;   //!< associated particle system object
    coreParticleEffect* m_pThis;     //!< pointer to foreign empty object or to itself (to improve destructor performance)


public:
    explicit coreParticleEffect(coreParticleSystem* pSystem)noexcept;
    ~coreParticleEffect() {if(this->IsDynamic()) m_pSystem->Unbind(this);}

    //! create new particles
    //! @{
    template <typename F> void CreateParticle(const coreUint& iNum, const float& fFrequency, F&& pFunction);
    template <typename F> void CreateParticle(const coreUint& iNum, F&& pFunction);
    inline coreParticle* CreateParticle() {return m_pSystem->CreateParticle(m_pThis);}
    //! @}

    //! change associated particle system object
    //! @{
    void ChangeSystem(coreParticleSystem* pSystem, const bool& bUnbind);
    //! @}

    //! check dynamic behavior
    //! @{
    inline bool IsDynamic()const {return (m_pThis == this) ? true : false;}
    //! @}

    //! set object properties
    //! @{
    inline void SetTimeID(const int& iTimeID)    {m_pThis = this; m_iTimeID = iTimeID;}
    inline void SetOrigin(coreObject3D* pOrigin) {m_pThis = this; m_pOrigin = pOrigin;}
    //! @}

    //! get object properties
    //! @{
    inline const int& GetTimeID()const          {return m_iTimeID;}
    inline coreObject3D* GetOrigin()const       {return m_pOrigin;}
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
inline void coreParticle::Update()noexcept
{
    ASSERT(m_pEffect)

    // update current animation value
    const float fTime = m_fSpeed * Core::System->GetTime(m_pEffect->GetTimeID());
    m_fValue -= fTime;

    // update current state
    m_CurrentState.vPosition += m_MoveState.vPosition * fTime;
    m_CurrentState.fScale    += m_MoveState.fScale    * fTime;
    m_CurrentState.fAngle    += m_MoveState.fAngle    * fTime;
    m_CurrentState.vColor    += m_MoveState.vColor    * fTime;
}


// ****************************************************************
// create new particles
template <typename F> void coreParticleEffect::CreateParticle(const coreUint& iNum, const float& fFrequency, F&& pFunction)
{
    ASSERT(fFrequency <= 60.0f)

    // update and check status value
    m_fCreation.Update(fFrequency, m_iTimeID);
    if(m_fCreation >= 1.0f)
    {
        // adjust status value
        m_fCreation -= std::floor(m_fCreation);

        // create particles and call function
        for(coreUint i = 0; i < iNum; ++i)
            pFunction(this->CreateParticle());
    }
}

template <typename F> void coreParticleEffect::CreateParticle(const coreUint& iNum, F&& pFunction)
{
    // create particles and call function
    for(coreUint i = 0; i < iNum; ++i)
        pFunction(this->CreateParticle());
}


#endif // _CORE_GUARD_PARTICLE_H_