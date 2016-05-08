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
    coreObject3D()noexcept;
    coreObject3D(const coreObject3D& c)noexcept;
    coreObject3D(coreObject3D&&      m)noexcept;
    virtual ~coreObject3D();

    /*! assignment operations */
    //! @{
    coreObject3D& operator = (const coreObject3D& c)noexcept;
    coreObject3D& operator = (coreObject3D&&      m)noexcept;
    //! @}

    /*! define the visual appearance */
    //! @{
    inline void DefineModel(std::nullptr_t)               {m_pModel = NULL;                                           ADD_FLAG(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION)}
    inline void DefineModel(const coreModelPtr&   pModel) {m_pModel = pModel;                                         ADD_FLAG(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION)}
    inline void DefineModel(const coreHashString& sName)  {m_pModel = Core::Manager::Resource->Get<coreModel>(sName); ADD_FLAG(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION)}
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
    void ChangeType(const coreInt32 iType);
    //! @}

    /*! set object properties */
    //! @{
    inline void SetPosition         (const coreVector3& vPosition)          {m_vPosition = vPosition;}
    inline void SetSize             (const coreVector3& vSize)              {if(m_vSize              != vSize)              {ADD_FLAG(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION) m_vSize              = vSize;}}
    inline void SetDirection        (const coreVector3& vDirection)         {if(m_vDirection         != vDirection)         {ADD_FLAG(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vDirection         = vDirection;}   ASSERT(vDirection  .IsNormalized())}
    inline void SetOrientation      (const coreVector3& vOrientation)       {if(m_vOrientation       != vOrientation)       {ADD_FLAG(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vOrientation       = vOrientation;} ASSERT(vOrientation.IsNormalized())}
    inline void SetCollisionModifier(const coreVector3& vCollisionModifier) {if(m_vCollisionModifier != vCollisionModifier) {ADD_FLAG(m_iUpdate, CORE_OBJECT_UPDATE_COLLISION) m_vCollisionModifier = vCollisionModifier;}}
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
    coreSet<coreObject3D*> m_apObjectList;                                            //!< list with pointers to similar 3d-objects
    coreUint32 m_iCurCapacity;                                                        //!< current instance-capacity of all related resources
    coreUint32 m_iCurEnabled;                                                         //!< current number of render-enabled 3d-objects (render-count)

    coreProgramPtr m_pProgram;                                                        //!< shader-program object

    coreArray<GLuint,           CORE_OBJECT3D_INSTANCE_BUFFERS>  m_aiVertexArray;     //!< vertex array objects
    coreArray<coreVertexBuffer, CORE_OBJECT3D_INSTANCE_BUFFERS>  m_aInstanceBuffer;   //!< instance data buffers
    coreArray<coreVertexBuffer, CORE_OBJECT3D_INSTANCE_BUFFERS>* m_paCustomBuffer;    //!< optional custom attribute buffers

    const void* m_pLastModel;                                                         //!< pointer to last used model (to detect changes and update the vertex array)

    coreUint8 m_iFilled;                                                              //!< vertex array fill status
    coreUint8 m_iUpdate;                                                              //!< buffer update status (dirty flag)


public:
    explicit coreBatchList(const coreUint32 iStartCapacity = 0u)noexcept;
    ~coreBatchList()override;

    DISABLE_COPY(coreBatchList)

    /*! define the visual appearance */
    //! @{
    inline void DefineProgram(std::nullptr_t)                 {m_pProgram = NULL;}
    inline void DefineProgram(const coreProgramPtr& pProgram) {m_pProgram = pProgram;}
    inline void DefineProgram(const coreHashString& sName)    {m_pProgram = Core::Manager::Resource->Get<coreProgram>(sName);}
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
    void Reallocate(const coreUint32 iNewCapacity);
    void Clear();
    void ShrinkToFit();
    //! @}

    /*! control custom vertex attributes */
    //! @{
    template <typename F> void CreateCustom(const coreUintW iVertexSize, F&& nDefineBufferFunc);   //!< [](coreVertexBuffer* OUTPUT pBuffer) -> void
    template <typename F> void UpdateCustom(F&& nUpdateDataFunc);                                  //!< [](coreByte* OUTPUT pData, const coreObject3D* pObject) -> void
    //! @}

    /*! check for instancing status */
    //! @{
    inline coreBool IsInstanced()const {return (m_aInstanceBuffer[0] && (m_iCurEnabled >= CORE_OBJECT3D_INSTANCE_THRESHOLD)) ? true : false;}
    //! @}

    /*! access 3d-object list directly */
    //! @{
    inline       coreSet<coreObject3D*>* List()      {return &m_apObjectList;}
    inline const coreSet<coreObject3D*>* List()const {return &m_apObjectList;}
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
    void __Reset(const coreResourceReset bInit)override;
    //! @}
};


// ****************************************************************
/* create buffer for custom vertex attributes */
template <typename F> void coreBatchList::CreateCustom(const coreUintW iVertexSize, F&& nDefineBufferFunc)
{
    ASSERT(m_iCurCapacity >= CORE_OBJECT3D_INSTANCE_THRESHOLD)
    WARN_IF(m_paCustomBuffer) return;

    // allocate custom attribute buffer memory
    m_paCustomBuffer = new coreArray<coreVertexBuffer, CORE_OBJECT3D_INSTANCE_BUFFERS>();

    for(coreUintW i = 0u; i < CORE_OBJECT3D_INSTANCE_BUFFERS; ++i)
    {
        coreVertexBuffer& oBuffer = (*m_paCustomBuffer)[i];

        // bind corresponding vertex array object
        glBindVertexArray(m_aiVertexArray[i]);

        // create custom attribute buffers
        oBuffer.Create(m_iCurCapacity, iVertexSize, NULL, CORE_DATABUFFER_STORAGE_PERSISTENT | CORE_DATABUFFER_STORAGE_FENCED);
        nDefineBufferFunc(&oBuffer);

        // set vertex data
        oBuffer.ActivateDivided(2u, 1u);
    }
}


// ****************************************************************
/* update custom vertex attributes per active object */
template <typename F> void coreBatchList::UpdateCustom(F&& nUpdateDataFunc)
{
    ASSERT(m_paCustomBuffer)
    if(!this->IsInstanced() || !CONTAINS_BIT(m_iUpdate, 1u)) return;

    // get vertex size
    const coreUintW iVertexSize = (*m_paCustomBuffer)[0].GetVertexSize();

    // switch to next available buffer
    m_paCustomBuffer->select(m_aInstanceBuffer.index());
    if(CONTAINS_BIT(m_iUpdate, 0u)) m_paCustomBuffer->next();

    // map required area of the custom attribute buffer
    coreByte* pRange  = m_paCustomBuffer->current().Map<coreByte>(0u, m_iCurEnabled * iVertexSize, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);
    coreByte* pCursor = pRange;

    FOR_EACH(it, m_apObjectList)
    {
        coreObject3D* pObject = (*it);

        // render only enabled objects
        if(pObject->IsEnabled(CORE_OBJECT_ENABLE_RENDER))
        {
            // write data to the buffer
            nUpdateDataFunc(r_cast<typename TRAIT_ARG_TYPE(F, 0u)>(pCursor),
                            s_cast<typename TRAIT_ARG_TYPE(F, 1u)>(pObject));
            pCursor += iVertexSize;
        }
    }

    // unmap buffer
    m_paCustomBuffer->current().Unmap(pRange);

    // reset the update status
    REMOVE_BIT(m_iUpdate, 1u)
}


#endif /* _CORE_GUARD_OBJECT3D_H_ */