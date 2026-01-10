///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_OBJECT3D_H_
#define _CORE_GUARD_OBJECT3D_H_

// TODO 5: instancing with more than one vertex array in the model ? (binding location)
// TODO 5: compress rotation and texture parameters (2x16 ?), color 4b, position and size
// TODO 3: add MoveNormal() and MoveSort() (with callback)
// TODO 3: SetPosition2, SetPosition3, same for other properties, same for Get
// TODO 3: temporarily cache move (and render) function in batchlist, with assert-checks
// TODO 2: handle broken volumes, reset to NULL ?


// ****************************************************************
/* 3d-object definitions */
#define CORE_BATCHLIST_INSTANCE_SIZE_HIGH (40u)   // instancing per-object size (high quality) (position (12), size (8),  rotation (8), color (4), texture-parameters (8))
#define CORE_BATCHLIST_INSTANCE_SIZE_LOW  (52u)   // instancing per-object size (low quality)  (position (12), size (12), rotation (8), color (4), texture-parameters (16))
#define CORE_BATCHLIST_INSTANCE_BUFFERS   (3u)    // number of concurrent instance data buffer
#define CORE_BATCHLIST_INSTANCE_THRESHOLD (4u)    // minimum number of objects to draw instanced

enum coreBatchListUpdate : coreUint8
{
    CORE_BATCHLIST_UPDATE_NOTHING  = 0x00u,   // update nothing
    CORE_BATCHLIST_UPDATE_INSTANCE = 0x01u,   // update instance data buffers
    CORE_BATCHLIST_UPDATE_CUSTOM   = 0x02u,   // update custom attribute buffers
    CORE_BATCHLIST_UPDATE_ALL      = 0x03u    // update everything
};
ENABLE_BITWISE(coreBatchListUpdate)


// ****************************************************************
/* 3d-object class */
class coreObject3D : public coreObject
{
private:
    coreVector3 m_vPosition;      // position of the 3d-object
    coreVector3 m_vSize;          // size-factor of the 3d-object
    coreVector3 m_vDirection;     // direction for the rotation quaternion
    coreVector3 m_vOrientation;   // orientation for the rotation quaternion


protected:
    coreModelPtr m_pModel;              // model object
    coreModelPtr m_pVolume;             // bounding volume object

    coreVector4 m_vRotation;            // separate rotation quaternion

    coreVector3 m_vCollisionModifier;   // size-modifier for collision detection
    coreVector3 m_vCollisionRange;      // total collision range (volume range * modified size)
    coreFloat   m_fCollisionRadius;     // total collision radius (volume radius * maximum modified size)

    coreVector3 m_vVisualRange;         // visual range (model range)
    coreFloat   m_fVisualRadius;        // visual radius (model radius)

    coreInt32 m_iType;                  // object type identifier (!0 = currently registered in the object manager)


public:
    coreObject3D()noexcept;
    coreObject3D(const coreObject3D& c)noexcept;
    coreObject3D(coreObject3D&&      m)noexcept;
    virtual ~coreObject3D();

    CUSTOM_COPY(coreObject3D)

    /* assignment operations */
    coreObject3D& operator = (const coreObject3D& c)noexcept;
    coreObject3D& operator = (coreObject3D&&      m)noexcept;

    /* define the visual appearance */
    inline void DefineModel (std::nullptr_t)                {m_pModel  = NULL;                                           ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_COLLISION | CORE_OBJECT_UPDATE_VISUAL)}
    inline void DefineModel (const coreModelPtr&   pModel)  {m_pModel  = pModel;                                         ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_COLLISION | CORE_OBJECT_UPDATE_VISUAL)}
    inline void DefineModel (const coreHashString& sName)   {m_pModel  = Core::Manager::Resource->Get<coreModel>(sName); ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_COLLISION | CORE_OBJECT_UPDATE_VISUAL)}
    inline void DefineVolume(std::nullptr_t)                {m_pVolume = NULL;                                           ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_COLLISION)}
    inline void DefineVolume(const coreModelPtr&   pVolume) {m_pVolume = pVolume;                                        ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_COLLISION)}
    inline void DefineVolume(const coreHashString& sName)   {m_pVolume = Core::Manager::Resource->Get<coreModel>(sName); ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_COLLISION)}
    void Undefine();

    /* render and move the 3d-object */
    coreBool     Prepare(const coreProgramPtr& pProgram);
    coreBool     Prepare();
    virtual void Render (const coreProgramPtr& pProgram);
    virtual void Render ();
    virtual void Move   ();

    /* change object type and manager registration */
    void ChangeType(const coreInt32 iType);

    /* set object properties */
    inline void SetPosition         (const coreVector3 vPosition)          {m_vPosition = vPosition;}
    inline void SetSize             (const coreVector3 vSize)              {if(m_vSize              != vSize)              {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_COLLISION | CORE_OBJECT_UPDATE_VISUAL) m_vSize              = vSize;}}
    inline void SetDirection        (const coreVector3 vDirection)         {if(m_vDirection         != vDirection)         {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM)                             m_vDirection         = vDirection;}   ASSERT(vDirection  .IsNormalized())}
    inline void SetOrientation      (const coreVector3 vOrientation)       {if(m_vOrientation       != vOrientation)       {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM)                             m_vOrientation       = vOrientation;} ASSERT(vOrientation.IsNormalized())}
    inline void SetCollisionModifier(const coreVector3 vCollisionModifier) {if(m_vCollisionModifier != vCollisionModifier) {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_COLLISION)                             m_vCollisionModifier = vCollisionModifier;}}

    /* get object properties */
    inline const coreModelPtr& GetModel            ()const {return m_pModel;}
    inline const coreModelPtr& GetVolume           ()const {return m_pVolume ? m_pVolume : m_pModel;}
    inline const coreVector3&  GetPosition         ()const {return m_vPosition;}
    inline const coreVector3&  GetSize             ()const {return m_vSize;}
    inline const coreVector3&  GetDirection        ()const {return m_vDirection;}
    inline const coreVector3&  GetOrientation      ()const {return m_vOrientation;}
    inline const coreVector4&  GetRotation         ()const {return m_vRotation;}
    inline const coreVector3&  GetCollisionModifier()const {return m_vCollisionModifier;}
    inline const coreVector3&  GetCollisionRange   ()const {return m_vCollisionRange;}
    inline const coreFloat&    GetCollisionRadius  ()const {return m_fCollisionRadius;}
    inline const coreVector3&  GetVisualRange      ()const {return m_vVisualRange;}
    inline const coreFloat&    GetVisualRadius     ()const {return m_fVisualRadius;}
    inline const coreInt32&    GetType             ()const {return m_iType;}
};


// ****************************************************************
/* batch list class */
class coreBatchList final : public coreResourceRelation
{
private:
    /* internal types */
    using coreDefineBuffer = void (*) (coreVertexBuffer* OUTPUT);
    using coreUpdateData   = void (*) (coreByte* OUTPUT, const coreObject3D*);
    using coreUpdateShader = void (*) (const coreProgramPtr&, const coreObject3D*);


private:
    coreSet<coreObject3D*> m_apObjectList;                                            // list with pointers to similar 3d-objects
    coreUint32 m_iNumInstances;                                                       // current instance-capacity of all buffers
    coreUint32 m_iNumEnabled;                                                         // current number of render-enabled 3d-objects (render-count)

    coreProgramPtr m_pProgram;                                                        // shader-program object

    GLuint m_iLastModel;                                                              // vertex buffer identifier of the last used model (to detect changes and update the vertex array)

    coreRing<GLuint,           CORE_BATCHLIST_INSTANCE_BUFFERS>  m_aiVertexArray;     // vertex array objects
    coreRing<coreVertexBuffer, CORE_BATCHLIST_INSTANCE_BUFFERS>  m_aInstanceBuffer;   // instance data buffers
    coreRing<coreVertexBuffer, CORE_BATCHLIST_INSTANCE_BUFFERS>* m_paCustomBuffer;    // optional custom attribute buffers

    coreDefineBuffer m_nDefineBufferFunc;                                             // function for defining the vertex structure of the custom attribute buffers
    coreUpdateData   m_nUpdateDataFunc;                                               // function for updating custom attributes with instancing
    coreUpdateShader m_nUpdateShaderFunc;                                             // function for updating custom attributes through shader uniforms
    coreUint8        m_iCustomSize;                                                   // vertex size for the custom attribute buffers

    coreUint8           m_iFilled;                                                    // vertex array fill status
    coreBatchListUpdate m_eUpdate;                                                    // buffer update status (dirty flag)


public:
    explicit coreBatchList(const coreUint32 iStartCapacity = 0u)noexcept;
    ~coreBatchList()final;

    DISABLE_COPY(coreBatchList)

    /* define the visual appearance */
    inline void DefineProgram(std::nullptr_t)                 {m_pProgram = NULL;}
    inline void DefineProgram(const coreProgramPtr& pProgram) {m_pProgram = pProgram;}
    inline void DefineProgram(const coreHashString& sName)    {m_pProgram = Core::Manager::Resource->Get<coreProgram>(sName);}
    void Undefine();

    /* render and move the batch list */
    void Render    (const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramSingle);
    void Render    ();
    void MoveNormal();
    void MoveSort  ();

    /* bind and unbind 3d-objects */
    void BindObject      (coreObject3D* pObject);
    void BindObjectUnsafe(coreObject3D* pObject);
    void UnbindObject    (coreObject3D* pObject);
    void Clear           ();

    /* control memory allocation */
    void        Reallocate (const coreUint32 iSize);
    inline void Reserve    (const coreUint32 iCapacity) {m_apObjectList.reserve(iCapacity);}
    inline void ShrinkToFit()                           {m_apObjectList.shrink_to_fit();}

    /* control custom vertex attributes */
    template <typename F, typename G, typename H> void CreateCustom(const coreUint8 iVertexSize, F&& nDefineBufferFunc, G&& nUpdateDataFunc, H&& nUpdateShaderFunc);   // [](coreVertexBuffer* OUTPUT pBuffer) -> void, [](coreByte* OUTPUT pData, const coreObject3D* pObject) -> void, [](const coreProgramPtr& pProgram, const coreObject3D* pObject) -> void

    /* check for instancing status */
    inline coreBool IsInstanced()const {return (CORE_GL_SUPPORT(ARB_instanced_arrays) && CORE_GL_SUPPORT(ARB_vertex_array_object) && (m_iNumEnabled >= CORE_BATCHLIST_INSTANCE_THRESHOLD));}
    inline coreBool IsCustom   ()const {return (m_paCustomBuffer != NULL);}

    /* access 3d-object list directly */
    inline       coreSet<coreObject3D*>* List()      {return &m_apObjectList;}
    inline const coreSet<coreObject3D*>* List()const {return &m_apObjectList;}

    /* get object properties */
    inline const coreProgramPtr& GetProgram   ()const {return m_pProgram;}
    inline       coreUint32      GetSize      ()const {return m_apObjectList.size();}
    inline       coreUint32      GetCapacity  ()const {return m_apObjectList.capacity();}
    inline const coreUint32&     GetNumEnabled()const {return m_iNumEnabled;}


private:
    /* reset with the resource manager */
    void __Reset(const coreResourceReset eInit)final;

    /* reshape with the resource manager */
    inline void __Reshape()final {}

    /* render the batch list */
    void __RenderDefault(const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramSingle, const coreUint32 iRenderCount);
    void __RenderCustom (const coreProgramPtr& pProgramInstanced, const coreProgramPtr& pProgramSingle, const coreUint32 iRenderCount);
};


// ****************************************************************
/* create buffer for custom vertex attributes */
template <typename F, typename G, typename H> void coreBatchList::CreateCustom(const coreUint8 iVertexSize, F&& nDefineBufferFunc, G&& nUpdateDataFunc, H&& nUpdateShaderFunc)
{
    WARN_IF(this->IsCustom()) return;

    // allocate custom attribute buffer memory
    m_paCustomBuffer = new coreRing<coreVertexBuffer, CORE_BATCHLIST_INSTANCE_BUFFERS>();

    // save functions and vertex size
    m_nDefineBufferFunc = std::forward<F>(nDefineBufferFunc);
    m_nUpdateDataFunc   = std::forward<G>(nUpdateDataFunc);
    m_nUpdateShaderFunc = std::forward<H>(nUpdateShaderFunc);
    m_iCustomSize       = iVertexSize;

    // immediately initialize if instance data buffers are already valid
    if(m_aInstanceBuffer[0].IsValid())
    {
        for(coreUintW i = 0u; i < CORE_BATCHLIST_INSTANCE_BUFFERS; ++i)
        {
            coreVertexBuffer& oBuffer = (*m_paCustomBuffer)[i];

            // bind vertex array object
            glBindVertexArray(m_aiVertexArray[i]);

            // create custom attribute buffer
            oBuffer.Create(m_iNumInstances, m_iCustomSize, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
            m_nDefineBufferFunc(&oBuffer);

            // set vertex data (custom only)
            oBuffer.Activate(1u);
        }

        // disable current model object (to fully enable the next model)
        coreModel::Disable(false);
    }
}


#endif /* _CORE_GUARD_OBJECT3D_H_ */