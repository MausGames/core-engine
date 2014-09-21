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
// TODO: in the list, sort in the normal iteration by checking for current and next object and swap them if necessary
// TODO: instancing with more than one vertex array in the model ? (binding location)
// TODO: implement efficient batch list sort function


// ****************************************************************
/* 3d-object definitions */
#define CORE_OBJECT3D_INSTANCE_SIZE      (1*sizeof(coreMatrix4) + 1*sizeof(coreVector4) + 1*sizeof(coreUint))   //!< instancing per-object size (transformation, color, texture-parameters)
#define CORE_OBJECT3D_INSTANCE_BUFFERS   (3u)                                                                   //!< number of concurrent instance data buffer
#define CORE_OBJECT3D_INSTANCE_THRESHOLD (10u)                                                                  //!< minimum number of objects to draw instanced


// ****************************************************************
/* 3d-object class */
class coreObject3D : public coreObject
{
private:
    coreVector3 m_vPosition;      //!< position of the 3d-object
    coreVector3 m_vSize;          //!< size-factor of the 3d-object
    coreVector3 m_vDirection;     //!< direction for the rotation matrix
    coreVector3 m_vOrientation;   //!< orientation for the rotation matrix


protected:
    coreModelPtr m_pModel;     //!< model object

    float m_fCollisionRange;   //!< range factor used for collision detection

    bool m_bManaged;           //!< currently listed in the object manager
    int  m_iType;              //!< identifier for the object type (0 = undefined)


public:
    constexpr_obj coreObject3D()noexcept;
    virtual ~coreObject3D() {}
    friend class coreObjectManager;

    /*! define the visual appearance */
    //! @{
    inline const coreModelPtr& DefineModel(const coreModelPtr& pModel) {m_pModel = pModel;                                          return m_pModel;}
    inline const coreModelPtr& DefineModel(const char*         pcName) {m_pModel = Core::Manager::Resource->Get<coreModel>(pcName); return m_pModel;}
    void Undefine();
    //! @}

    /*! render and move the 3d-object */
    //! @{
    bool         Enable(const coreProgramPtr& pProgram)hot_func;
    bool         Enable()hot_func;
    virtual void Render(const coreProgramPtr& pProgram)hot_func;
    virtual void Render()hot_func;
    virtual void Move  ()hot_func;
    //! @}
    
    /*! handle collision between different structures */
    //! @{
    static bool  Collision(const coreObject3D& Object1, const coreObject3D& Object2);
    static float Collision(const coreObject3D& Object,  const coreVector3&  vLinePos, const coreVector3& vLineDir);
    //! @}

    /*! set object properties */
    //! @{
    inline void SetPosition      (const coreVector3& vPosition)       {if(m_vPosition    != vPosition)    {m_iUpdate |= CORE_OBJECT_UPDATE_TRANSFORM; m_vPosition    = vPosition;}}
    inline void SetSize          (const coreVector3& vSize)           {if(m_vSize        != vSize)        {m_iUpdate |= CORE_OBJECT_UPDATE_TRANSFORM; m_vSize        = vSize;}}
    inline void SetDirection     (const coreVector3& vDirection)      {if(m_vDirection   != vDirection)   {m_iUpdate  = CORE_OBJECT_UPDATE_ALL;       m_vDirection   = vDirection;}   ASSERT(vDirection.IsNormalized())}
    inline void SetOrientation   (const coreVector3& vOrientation)    {if(m_vOrientation != vOrientation) {m_iUpdate  = CORE_OBJECT_UPDATE_ALL;       m_vOrientation = vOrientation;} ASSERT(vOrientation.IsNormalized())}
    inline void SetCollisionRange(const float&       fCollisionRange) {m_fCollisionRange = fCollisionRange;}
    inline void SetType          (const int&         iType)           {m_iType           = iType;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreModelPtr& GetModel         ()const {return m_pModel;}
    inline const coreVector3&  GetPosition      ()const {return m_vPosition;}
    inline const coreVector3&  GetSize          ()const {return m_vSize;}
    inline const coreVector3&  GetDirection     ()const {return m_vDirection;}
    inline const coreVector3&  GetOrientation   ()const {return m_vOrientation;}
    inline const float&        GetCollisionRange()const {return m_fCollisionRange;}
    inline const int&          GetType          ()const {return m_iType;}
    //! @}
};


// ****************************************************************
/* batch list class */
class coreBatchList final : public coreResourceRelation
{
private:
    std::vector<coreObject3D*> m_apObjectList;                                         //!< list with pointers to similar 3d-objects
    coreUint m_iCurCapacity;                                                           //!< current instance-capacity of all related resources
    coreUint m_iCurEnabled;                                                            //!< current number of enabled 3d-objects

    coreProgramPtr m_pProgram;                                                         //!< shader-program object

    coreSelect<GLuint,           CORE_OBJECT3D_INSTANCE_BUFFERS> m_aiVertexArray;      //!< vertex array objects
    coreSelect<coreVertexBuffer, CORE_OBJECT3D_INSTANCE_BUFFERS> m_aiInstanceBuffer;   //!< instance data buffers

    bool m_bUpdate;                                                                    //!< buffer update status


public:
    explicit coreBatchList(const coreUint& iStartCapacity)noexcept;
    ~coreBatchList();

    /*! define the visual appearance */
    //! @{
    inline const coreProgramPtr& DefineProgram(const coreProgramPtr& pProgram) {m_pProgram = pProgram;                                          return m_pProgram;}
    inline const coreProgramPtr& DefineProgram(const char*           pcName)   {m_pProgram = Core::Manager::Resource->Get<coreProgram>(pcName); return m_pProgram;}
    void Undefine();
    //! @}

    /*! render and move the batch list */
    //! @{
    void Render(const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramNormal)hot_func;
    void Render    ()hot_func;
    void MoveNormal()hot_func;
    void MoveSort  ()hot_func;
    //! @}
    
    /*! bind and unbind 3d-objects */
    //! @{
    void BindObject  (coreObject3D* pObject);
    void UnbindObject(coreObject3D* pObject);
    //! @}

    /*! control memory allocation */
    //! @{
    void        Reallocate(const coreUint& iNewCapacity);
    void        Clear();
    inline void ShrinkToFit() {this->Reallocate(m_apObjectList.size());}
    //! @}

    /*! access 3d-object list directly */
    //! @{
    inline std::vector<coreObject3D*>* List() {return &m_apObjectList;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreProgramPtr& GetProgram    ()const {return m_pProgram;}  
    inline const coreUint&       GetCurCapacity()const {return m_iCurCapacity;}
    inline const coreUint&       GetCurEnabled ()const {return m_iCurEnabled;}
    //! @}


private:
    DISABLE_COPY(coreBatchList)

    /*! reset with the resource manager */
    //! @{
    void __Reset(const coreResourceReset& bInit)override;
    //! @}
};


// ****************************************************************
/* constructor */
constexpr_obj coreObject3D::coreObject3D()noexcept
: m_vPosition       (coreVector3(0.0f,0.0f, 0.0f))
, m_vSize           (coreVector3(1.0f,1.0f, 1.0f))
, m_vDirection      (coreVector3(0.0f,0.0f,-1.0f))
, m_vOrientation    (coreVector3(0.0f,1.0f, 0.0f))
, m_fCollisionRange (1.0f)
, m_bManaged        (false)
, m_iType           (0)
{
}


#endif /* _CORE_GUARD_OBJECT3D_H_ */