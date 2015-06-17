//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_OBJECT3D_H_
#define _CORE_GUARD_OBJECT3D_H_

// TODO: remove SQRT in object-line collision
// TODO: instancing with more than one vertex array in the model ? (binding location)
// TODO: implement efficient batch list sort function
// TODO: compress rotation and texture parameters (2x16 ?)
// TODO: make coreBatchList to template class, check for every casting


// ****************************************************************
/* 3d-object definitions */
#define CORE_OBJECT3D_INSTANCE_SIZE      (3u*sizeof(coreFloat) + 7u*sizeof(coreUint32))   //!< instancing per-object size (position, size, rotation, color, texture-parameters)
#define CORE_OBJECT3D_INSTANCE_BUFFERS   (3u)                                             //!< number of concurrent instance data buffer
#define CORE_OBJECT3D_INSTANCE_THRESHOLD (4u)                                             //!< minimum number of objects to draw instanced


// ****************************************************************
/* 3d-object class */
class coreObject3D : public coreObject
{
private:
    coreVector3 m_vPosition;      //!< position of the 3d-object
    coreVector3 m_vSize;          //!< size-factor of the 3d-object
    coreVector3 m_vDirection;     //!< direction for the rotation quaternion
    coreVector3 m_vOrientation;   //!< orientation for the rotation quaternion


protected:
    coreModelPtr m_pModel;              //!< model object

    coreVector4 m_vRotation;            //!< separate rotation quaternion

    coreVector3 m_vCollisionModifier;   //!< size-modifier for collision detection
    coreVector3 m_vCollisionRange;      //!< total collision range (model range * size * modifier)
    coreFloat   m_fCollisionRadius;     //!< total collision radius (model radius * maximum size * maximum modifier)

    coreInt32 m_iType;                  //!< object type identifier (!0 = currently registered in the object manager)


public:
    constexpr_weak coreObject3D()noexcept;
    inline coreObject3D(const coreObject3D& c)noexcept;
    inline coreObject3D(coreObject3D&&      m)noexcept;
    virtual ~coreObject3D();

    /*! assignment operations */
    //! @{
    coreObject3D& operator = (const coreObject3D& c)noexcept;
    coreObject3D& operator = (coreObject3D&&      m)noexcept;
    //! @}

    /*! define the visual appearance */
    //! @{
    inline const coreModelPtr& DefineModel(const coreModelPtr& pModel) {m_pModel = pModel;                                          ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION) return m_pModel;}
    inline const coreModelPtr& DefineModel(const coreChar*     pcName) {m_pModel = Core::Manager::Resource->Get<coreModel>(pcName); ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION) return m_pModel;}
    void Undefine();
    //! @}

    /*! render and move the 3d-object */
    //! @{
    coreBool     Prepare(const coreProgramPtr& pProgram);
    coreBool     Prepare();
    virtual void Render (const coreProgramPtr& pProgram);
    virtual void Render ();
    virtual void Move   ();
    //! @}

    /*! change object type and manager registration */
    //! @{
    void ChangeType(const coreInt32& iType);
    //! @}

    /*! set object properties */
    //! @{
    inline void SetPosition         (const coreVector3& vPosition)          {m_vPosition = vPosition;}
    inline void SetSize             (const coreVector3& vSize)              {if(m_vSize              != vSize)              {ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION) m_vSize              = vSize;}}
    inline void SetDirection        (const coreVector3& vDirection)         {if(m_vDirection         != vDirection)         {ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vDirection         = vDirection;}   ASSERT(vDirection  .IsNormalized())}
    inline void SetOrientation      (const coreVector3& vOrientation)       {if(m_vOrientation       != vOrientation)       {ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vOrientation       = vOrientation;} ASSERT(vOrientation.IsNormalized())}
    inline void SetCollisionModifier(const coreVector3& vCollisionModifier) {if(m_vCollisionModifier != vCollisionModifier) {ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION) m_vCollisionModifier = vCollisionModifier;}}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreModelPtr& GetModel            ()const {return m_pModel;}
    inline const coreVector3&  GetPosition         ()const {return m_vPosition;}
    inline const coreVector3&  GetSize             ()const {return m_vSize;}
    inline const coreVector3&  GetDirection        ()const {return m_vDirection;}
    inline const coreVector3&  GetOrientation      ()const {return m_vOrientation;}
    inline const coreVector4&  GetRotation         ()const {return m_vRotation;}
    inline const coreVector3&  GetCollisionModifier()const {return m_vCollisionModifier;}
    inline const coreVector3&  GetCollisionRange   ()const {return m_vCollisionRange;}
    inline const coreFloat&    GetCollisionRadius  ()const {return m_fCollisionRadius;}
    inline const coreInt32&    GetType             ()const {return m_iType;}
    //! @}
};


// ****************************************************************
/* batch list class */
class coreBatchList final : public coreResourceRelation
{
private:
    std::vector<coreObject3D*> m_apObjectList;                                         //!< list with pointers to similar 3d-objects
    coreUint32 m_iCurCapacity;                                                         //!< current instance-capacity of all related resources
    coreUint32 m_iCurEnabled;                                                          //!< current number of render-enabled 3d-objects (render-count)

    coreProgramPtr m_pProgram;                                                         //!< shader-program object

    coreSelect<GLuint,           CORE_OBJECT3D_INSTANCE_BUFFERS> m_aiVertexArray;      //!< vertex array objects
    coreSelect<coreVertexBuffer, CORE_OBJECT3D_INSTANCE_BUFFERS> m_aiInstanceBuffer;   //!< instance data buffers

    coreBool m_bUpdate;                                                                //!< buffer update status (dirty flag)


public:
    explicit coreBatchList(const coreUint32& iStartCapacity)noexcept;
    ~coreBatchList();

    DISABLE_COPY(coreBatchList)

    /*! define the visual appearance */
    //! @{
    inline const coreProgramPtr& DefineProgram(const coreProgramPtr& pProgram) {m_pProgram = pProgram;                                          return m_pProgram;}
    inline const coreProgramPtr& DefineProgram(const coreChar*       pcName)   {m_pProgram = Core::Manager::Resource->Get<coreProgram>(pcName); return m_pProgram;}
    void Undefine();
    //! @}

    /*! render and move the batch list */
    //! @{
    void Render(const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramSingle);
    void Render    ();
    void MoveNormal();
    void MoveSort  ();
    //! @}

    /*! bind and unbind 3d-objects */
    //! @{
    void BindObject  (coreObject3D* pObject);
    void UnbindObject(coreObject3D* pObject);
    //! @}

    /*! control memory allocation */
    //! @{
    void        Reallocate(const coreUint32& iNewCapacity);
    void        Clear();
    inline void ShrinkToFit() {this->Reallocate(m_apObjectList.size()); m_apObjectList.shrink_to_fit();}
    //! @}

    /*! check for instancing status */
    //! @{
    inline coreBool IsInstanced()const {return (m_aiInstanceBuffer[0] && (m_iCurEnabled >= CORE_OBJECT3D_INSTANCE_THRESHOLD)) ? true : false;}
    //! @}

    /*! access 3d-object list directly */
    //! @{
    inline       std::vector<coreObject3D*>* List()      {return &m_apObjectList;}
    inline const std::vector<coreObject3D*>* List()const {return &m_apObjectList;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreProgramPtr& GetProgram    ()const {return m_pProgram;}
    inline const coreUint32&     GetCurCapacity()const {return m_iCurCapacity;}
    inline const coreUint32&     GetCurEnabled ()const {return m_iCurEnabled;}
    //! @}


private:
    /*! reset with the resource manager */
    //! @{
    void __Reset(const coreResourceReset& bInit)override;
    //! @}
};


// ****************************************************************
/* constructor */
constexpr_weak coreObject3D::coreObject3D()noexcept
: m_vPosition          (coreVector3(0.0f,0.0f,0.0f))
, m_vSize              (coreVector3(1.0f,1.0f,1.0f))
, m_vDirection         (coreVector3(0.0f,1.0f,0.0f))
, m_vOrientation       (coreVector3(0.0f,0.0f,1.0f))
, m_vRotation          (coreVector4::QuatIdentity())
, m_vCollisionModifier (coreVector3(1.0f,1.0f,1.0f))
, m_vCollisionRange    (coreVector3(0.0f,0.0f,0.0f))
, m_fCollisionRadius   (0.0f)
, m_iType              (0)
{
}

inline coreObject3D::coreObject3D(const coreObject3D& c)noexcept
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

inline coreObject3D::coreObject3D(coreObject3D&& m)noexcept
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


#endif /* _CORE_GUARD_OBJECT3D_H_ */