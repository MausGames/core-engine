///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_PARTICLE_H_
#define _CORE_GUARD_PARTICLE_H_

// TODO 3: make different base particles ? generic
// TODO 5: what about texture size and offset
// TODO 5: what about velocity
// TODO 5: what about texture index parameter to allow different objects to be rendered
// TODO 5: SSBO[index] really faster than a divisor ? check also for their use instead of VAO in general
// TODO 5: high systems: currently CPU(move) is bottleneck, look for improvement with transform feedback(3.0) or compute shader(4.0)
// TODO 5: low systems: merge geometry to reduce draw calls
// TODO 3: sorting based on position
// TODO 3: culling (also on instancing)
// TODO 5: <old comment style>


// ****************************************************************
/* particle definitions */
#define CORE_PARTICLE_INSTANCE_SIZE_HIGH (24u)   // instancing per-particle size (high quality) (position (12), data (8),  color (4))
#define CORE_PARTICLE_INSTANCE_SIZE_LOW  (28u)   // instancing per-particle size (low quality)  (position (12), data (12), color (4))
#define CORE_PARTICLE_INSTANCE_BUFFERS   (3u)    // number of concurrent instance data buffer


// ****************************************************************
/* particle class */
class alignas(ALIGNMENT_CACHE) coreParticle final
{
public:
    /* state structure */
    struct coreState final
    {
        coreVector3 vPosition;   // configured position
        coreFloat   fScale;      // configured scale-factor
        coreFloat   fAngle;      // configured orientation-angle
        coreUint32  iColor;      // configured RGBA color-value (packed)

        inline coreVector4 GetColor4()const {return coreVector4::UnpackUnorm4x8(iColor);}
    };

    /* animation structure */
    struct coreAnim final
    {
        coreVector3 vPosition;   // interpolated position
        coreFloat   fScale;      // interpolated scale-factor
        coreFloat   fAngle;      // interpolated orientation-angle
        coreVector4 vColor;      // interpolated RGBA color-value
    };


private:
    coreState m_BeginState;          // initial state
    coreState m_EndState;            // final state

    coreFloat m_fValue;              // current simulation value (from 1.0f to 0.0f)
    coreFloat m_fSpeed;              // speed factor

    coreParticleEffect* m_pEffect;   // associated particle effect object


public:
    coreParticle() = default;

    FRIEND_CLASS(coreParticleSystem)
    ENABLE_COPY (coreParticle)

    /* check current status */
    inline coreBool IsActive()const {return (m_fValue > 0.0f);}
    inline void     Disable ()      {m_fValue = 0.0f;}

    /* animate the particle absolute */
    inline void SetPositionAbs(const coreVector3 vBegin, const coreVector3 vEnd) {m_BeginState.vPosition = vBegin;                m_EndState.vPosition = vEnd;}
    inline void SetScaleAbs   (const coreFloat   fBegin, const coreFloat   fEnd) {m_BeginState.fScale    = fBegin;                m_EndState.fScale    = fEnd;}
    inline void SetAngleAbs   (const coreFloat   fBegin, const coreFloat   fEnd) {m_BeginState.fAngle    = fBegin;                m_EndState.fAngle    = fEnd;}
    inline void SetColor4Abs  (const coreVector4 vBegin, const coreVector4 vEnd) {m_BeginState.iColor    = vBegin.PackUnorm4x8(); m_EndState.iColor    = vEnd.PackUnorm4x8(); ASSERT((vBegin.Min() >= 0.0f) && (vBegin.Max() <= 1.0f) && (vEnd.Min() >= 0.0f) && (vEnd.Max() <= 1.0f))}

    /* animate the particle relative */
    inline void SetPositionRel(const coreVector3 vBegin, const coreVector3 vMove) {this->SetPositionAbs(vBegin, vBegin + vMove);}
    inline void SetScaleRel   (const coreFloat   fBegin, const coreFloat   fMove) {this->SetScaleAbs   (fBegin, fBegin + fMove);}
    inline void SetAngleRel   (const coreFloat   fBegin, const coreFloat   fMove) {this->SetAngleAbs   (fBegin, fBegin + fMove);}
    inline void SetColor4Rel  (const coreVector4 vBegin, const coreVector4 vMove) {this->SetColor4Abs  (vBegin, vBegin + vMove);}

    /* animate the particle static */
    inline void SetPositionStc(const coreVector3 vStatic) {this->SetPositionAbs(vStatic, vStatic);}
    inline void SetScaleStc   (const coreFloat   fStatic) {this->SetScaleAbs   (fStatic, fStatic);}
    inline void SetAngleStc   (const coreFloat   fStatic) {this->SetAngleAbs   (fStatic, fStatic);}
    inline void SetColor4Stc  (const coreVector4 vStatic) {m_BeginState.iColor = m_EndState.iColor = vStatic.PackUnorm4x8(); ASSERT((vStatic.Min() >= 0.0f) && (vStatic.Max() <= 1.0f))}

    /* set object properties */
    inline void SetSpeed(const coreFloat fSpeed) {m_fSpeed = fSpeed;}

    /* edit state directly */
    inline coreState& EditBeginState() {return m_BeginState;}
    inline coreState& EditEndState  () {return m_EndState;}

    /* get object properties */
    inline const coreState&    GetBeginState()const {return m_BeginState;}
    inline const coreState&    GetEndState  ()const {return m_EndState;}
    inline const coreFloat&    GetValue     ()const {return m_fValue;}
    inline const coreFloat&    GetSpeed     ()const {return m_fSpeed;}
    inline coreParticleEffect* GetEffect    ()const {return m_pEffect;}


private:
    /* control the particle */
    inline void __Prepare(coreParticleEffect* pEffect) {m_pEffect = pEffect; m_fValue = 1.0f;}
    inline void __Update();
};


// ****************************************************************
/* particle system class */
class coreParticleSystem final : public coreResourceRelation
{
public:
    /* internal types */
    using coreAnimate = coreParticle::coreAnim (*) (const coreParticle& oParticle);


private:
    coreList<coreParticle> m_aParticle;                                             // pre-allocated particles
    coreUint32             m_iNumParticles;                                         // number of particles
    coreUint32             m_iCurParticle;                                          // current particle

    coreTexturePtr m_apTexture[CORE_TEXTURE_UNITS];                                 // multiple texture objects
    coreProgramPtr m_pProgram;                                                      // shader-program object

    std::deque<coreParticle*> m_apRenderList;                                       // sorted render list with active particles
    coreParticleEffect*       m_pDefaultEffect;                                     // default particle effect object (dynamic, because of class order)

    coreRing<GLuint,           CORE_PARTICLE_INSTANCE_BUFFERS> m_aiVertexArray;     // vertex array objects
    coreRing<coreVertexBuffer, CORE_PARTICLE_INSTANCE_BUFFERS> m_aInstanceBuffer;   // instance data buffers

    coreAnimate m_nAnimateFunc;                                                     // particle animation function

    coreBool m_bUpdate;                                                             // buffer update status (dirty flag)


public:
    explicit coreParticleSystem(const coreUint32 iStartSize = 0u)noexcept;
    ~coreParticleSystem()final;

    DISABLE_COPY(coreParticleSystem)

    /* define the visual appearance */
    inline void DefineTexture(const coreUintW iUnit, std::nullptr_t)                 {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = NULL;}
    inline void DefineTexture(const coreUintW iUnit, const coreTexturePtr& pTexture) {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = pTexture;}
    inline void DefineTexture(const coreUintW iUnit, const coreHashString& sName)    {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = Core::Manager::Resource->Get<coreTexture>(sName);}
    inline void DefineProgram(std::nullptr_t)                                        {m_pProgram = NULL;}
    inline void DefineProgram(const coreProgramPtr& pProgram)                        {m_pProgram = pProgram;}
    inline void DefineProgram(const coreHashString& sName)                           {m_pProgram = Core::Manager::Resource->Get<coreProgram>(sName);}
    void Undefine();

    /* render and move the particle system */
    void Render();
    void Move  ();

    /* create new particles */
    coreParticle*        CreateParticle(coreParticleEffect* pEffect);
    inline coreParticle* CreateParticle() {return this->CreateParticle(m_pDefaultEffect);}

    /* unbind and remove particles */
    void Unbind(const coreParticleEffect* pEffect);
    void Clear (const coreParticleEffect* pEffect);
    void UnbindAll();
    void ClearAll ();

    /* control memory allocation */
    void Reallocate(const coreUint32 iSize);

    /* update particles with custom simulation */
    template <typename F> void ForEachParticle   (const coreParticleEffect* pEffect, F&& nUpdateFunc);   // [](coreParticle* OUTPUT pParticle, const coreUintW i) -> void
    template <typename F> void ForEachParticleAll(F&& nUpdateFunc);                                      // [](coreParticle* OUTPUT pParticle, const coreUintW i) -> void

    /* override particle animation function */
    template <typename F> inline void SetAnimateFunc(F&& nAnimateFunc) {m_nAnimateFunc = nAnimateFunc;}   // [](const coreParticle& oParticle) -> coreParticleAnim

    /* get object properties */
    inline const coreTexturePtr& GetTexture           (const coreUintW iUnit)const {ASSERT(iUnit < CORE_TEXTURE_UNITS) return m_apTexture[iUnit];}
    inline const coreProgramPtr& GetProgram           ()const                      {return m_pProgram;}
    inline const coreUint32&     GetNumParticles      ()const                      {return m_iNumParticles;}
    inline       coreUintW       GetNumActiveParticles()const                      {return m_apRenderList.size();}
    inline coreParticleEffect*   GetDefaultEffect     ()const                      {return m_pDefaultEffect;}


private:
    /* reset with the resource manager */
    void __Reset(const coreResourceReset eInit)final;

    /* reshape with the resource manager */
    inline void __Reshape()final {}
};


// ****************************************************************
/* particle effect class */
class coreParticleEffect final
{
private:
    coreFlow m_fCreation;            // status value for particle creation

    coreInt8      m_iTimeID;         // ID of the used frame time
    coreObject3D* m_pOrigin;         // origin object for relative movement

    coreParticleSystem* m_pSystem;   // associated particle system object
    coreParticleEffect* m_pThis;     // pointer to foreign default object or to itself (to improve destructor performance)


public:
    explicit coreParticleEffect(coreParticleSystem* pSystem = NULL)noexcept;
    coreParticleEffect(const coreParticleEffect& c)noexcept;
    ~coreParticleEffect();

    FRIEND_CLASS(coreParticleSystem)

    /* assignment operations */
    coreParticleEffect& operator = (const coreParticleEffect& c)noexcept;

    /* create new particles */
    template <typename F> void CreateParticle(const coreUintW iNum, const coreFloat fFrequency, F&& nInitFunc);   // [](coreParticle* OUTPUT pParticle) -> void
    template <typename F> void CreateParticle(const coreUintW iNum,                             F&& nInitFunc);   // [](coreParticle* OUTPUT pParticle) -> void
    inline coreParticle* CreateParticle() {ASSERT(m_pSystem) return m_pSystem->CreateParticle(m_pThis);}

    /* unbind and remove particles */
    inline void Unbind() {if(m_pSystem) m_pSystem->Unbind(m_pThis);}
    inline void Clear () {if(m_pSystem) m_pSystem->Clear (m_pThis);}

    /* update particles with custom simulation */
    template <typename F> inline void ForEachParticle(F&& nUpdateFunc) {ASSERT(m_pSystem) m_pSystem->ForEachParticle(m_pThis, nUpdateFunc);}   // [](coreParticle* OUTPUT pParticle, const coreUintW i) -> void

    /* change associated particle system object */
    void ChangeSystem(coreParticleSystem* pSystem, const coreBool bUnbind);

    /* check for dynamic behavior */
    inline coreBool IsDynamic()const {return (m_pThis == this);}

    /* set object properties */
    inline void SetTimeID(const coreInt8 iTimeID) {m_pThis = this; m_iTimeID = iTimeID;}
    inline void SetOrigin(coreObject3D*  pOrigin) {m_pThis = this; m_pOrigin = pOrigin;}

    /* get object properties */
    inline const coreInt8&     GetTimeID()const {return m_iTimeID;}
    inline coreObject3D*       GetOrigin()const {return m_pOrigin;}
    inline coreParticleSystem* GetSystem()const {return m_pSystem;}
};


// ****************************************************************
/* update the particle */
inline void coreParticle::__Update()
{
    ASSERT(m_pEffect)

    // update current simulation value
    m_fValue -= m_fSpeed * Core::System->GetTime(m_pEffect->GetTimeID());
}


// ****************************************************************
/* update particles with custom simulation */
template <typename F> void coreParticleSystem::ForEachParticle(const coreParticleEffect* pEffect, F&& nUpdateFunc)
{
    ASSERT(pEffect)

    coreUintW i = 0u;

    FOR_EACH(it, m_apRenderList)
    {
        coreParticle* pParticle = (*it);

        // check particle effect object
        if(pParticle->GetEffect() == pEffect)
        {
            // call custom update function
            nUpdateFunc(pParticle, i++);
        }
    }
}

template <typename F> void coreParticleSystem::ForEachParticleAll(F&& nUpdateFunc)
{
    coreUintW i = 0u;

    FOR_EACH(it, m_apRenderList)
    {
        coreParticle* pParticle = (*it);

        // call custom update function
        nUpdateFunc(pParticle, i++);
    }
}


// ****************************************************************
/* create new particles */
template <typename F> void coreParticleEffect::CreateParticle(const coreUintW iNum, const coreFloat fFrequency, F&& nInitFunc)
{
    ASSERT(fFrequency <= 60.0f)

    // update and check status value
    m_fCreation.Update(fFrequency, m_iTimeID);
    if(m_fCreation >= 1.0f)
    {
        // adjust status value
        const coreUintW iComp = MIN(F_TO_UI(m_fCreation), 3u);
        m_fCreation = FRACT(m_fCreation);

        // create particles and call init function
        for(coreUintW i = iNum * iComp; i--; )
            nInitFunc(this->CreateParticle());
    }
}

template <typename F> void coreParticleEffect::CreateParticle(const coreUintW iNum, F&& nInitFunc)
{
    // create particles and call init function
    for(coreUintW i = iNum; i--; )
        nInitFunc(this->CreateParticle());
}


// ****************************************************************
/* additional checks */
STATIC_ASSERT(std::is_trivial<coreParticle>::value)


#endif /* _CORE_GUARD_PARTICLE_H_ */