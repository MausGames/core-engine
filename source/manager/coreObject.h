//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_OBJECT_H_
#define _CORE_GUARD_OBJECT_H_

// TODO: re-implement relative object behavior (additional classes?)
// TODO: GL_NV_fill_rectangle for low-memory model (state-tracking for polygon-mode ?)
// TODO: single triangle for fullscreen processing ?
// TODO: fix resource-manager reset, change location of global resources/objects (incl. single triangle)
// TODO: add pre-registering interface to object manager ? accessing a non-existing list can cause issues
// TODO: [const coreObject3D*] should be [const coreObject3D&], but pointers are swapped (for performance reasons)


// ****************************************************************
/* object definitions */
enum coreObjectUpdate : coreUint8
{
    CORE_OBJECT_UPDATE_NOTHING   = 0x00u,   //!< update nothing
    CORE_OBJECT_UPDATE_TRANSFORM = 0x01u,   //!< update transformation values
    CORE_OBJECT_UPDATE_COLLISION = 0x02u,   //!< update collision-relevant values
    CORE_OBJECT_UPDATE_ALL       = 0x03u    //!< update everything
};
ENABLE_BITWISE(coreObjectUpdate)

enum coreObjectEnable : coreUint8
{
    CORE_OBJECT_ENABLE_NOTHING = 0x00u,   //!< do nothing
    CORE_OBJECT_ENABLE_RENDER  = 0x01u,   //!< enable render routine
    CORE_OBJECT_ENABLE_MOVE    = 0x02u,   //!< enable move routine
    CORE_OBJECT_ENABLE_ALL     = 0x03u    //!< enable all routines
};
ENABLE_BITWISE(coreObjectEnable)


// ****************************************************************
/* object interface */
class INTERFACE coreObject
{
protected:
    coreTexturePtr m_apTexture[CORE_TEXTURE_UNITS];   //!< multiple texture objects
    coreProgramPtr m_pProgram;                        //!< shader-program object

    coreVector4 m_vColor;                             //!< RGBA color-value
    coreVector2 m_vTexSize;                           //!< size-factor of the texture
    coreVector2 m_vTexOffset;                         //!< offset of the texture

    coreObjectUpdate m_iUpdate;                       //!< update status (dirty flag)
    coreObjectEnable m_iEnabled;                      //!< enabled object routines
    coreInt32 m_iStatus;                              //!< numeric status-value for individual use


protected:
    coreObject()noexcept;
    ~coreObject() = default;


public:
    FRIEND_CLASS(coreObjectManager)
    ENABLE_COPY (coreObject)

    /*! define the visual appearance */
    //! @{
    inline void DefineTexture(const coreUintW iUnit, std::nullptr_t)                 {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = NULL;}
    inline void DefineTexture(const coreUintW iUnit, const coreTexturePtr& pTexture) {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = pTexture;}
    inline void DefineTexture(const coreUintW iUnit, const coreHashString& sName)    {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = Core::Manager::Resource->Get<coreTexture>(sName);}
    inline void DefineProgram(std::nullptr_t)                                        {m_pProgram = NULL;}
    inline void DefineProgram(const coreProgramPtr& pProgram)                        {m_pProgram = pProgram;}
    inline void DefineProgram(const coreHashString& sName)                           {m_pProgram = Core::Manager::Resource->Get<coreProgram>(sName);}
    //! @}

    /*! check for enabled object routines */
    //! @{
    inline coreBool IsEnabled(const coreObjectEnable iEnabled)const {ASSERT(iEnabled) return CONTAINS_FLAG(m_iEnabled, iEnabled) ? true : false;}
    //! @}

    /*! set object properties */
    //! @{
    inline void SetColor4   (const coreVector4&     vColor)     {m_vColor     = vColor;}
    inline void SetColor3   (const coreVector3&     vColor)     {m_vColor.xyz(vColor);}
    inline void SetAlpha    (const coreFloat        fAlpha)     {m_vColor.a   = fAlpha;}
    inline void SetTexSize  (const coreVector2&     vTexSize)   {m_vTexSize   = vTexSize;}
    inline void SetTexOffset(const coreVector2&     vTexOffset) {m_vTexOffset = vTexOffset;}
    inline void SetEnabled  (const coreObjectEnable iEnabled)   {m_iEnabled   = iEnabled;}
    inline void SetStatus   (const coreInt32        iStatus)    {m_iStatus    = iStatus;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreTexturePtr& GetTexture  (const coreUintW iUnit)const {ASSERT(iUnit < CORE_TEXTURE_UNITS) return m_apTexture[iUnit];}
    inline const coreProgramPtr& GetProgram  ()const                      {return m_pProgram;}
    inline const coreVector4&    GetColor4   ()const                      {return m_vColor;}
    inline       coreVector3     GetColor3   ()const                      {return m_vColor.xyz();}
    inline const coreFloat&      GetAlpha    ()const                      {return m_vColor.a;}
    inline const coreVector2&    GetTexSize  ()const                      {return m_vTexSize;}
    inline const coreVector2&    GetTexOffset()const                      {return m_vTexOffset;}
    inline const coreInt32&      GetStatus   ()const                      {return m_iStatus;}
    //! @}
};


// ****************************************************************
/* object manager */
class coreObjectManager final
{
private:
    /*! internal types */
    using coreObjectList = std::vector<coreObject3D*>;

    /*! object collision structure */
    struct coreObjectCollision final
    {
        const coreObject3D* pObject1;     //!< first 3d-object
        const coreObject3D* pObject2;     //!< second 3d-object
        coreUint32          iLastFrame;   //!< frame number of their last collision
    };


private:
    coreLookup<coreInt32, coreObjectList> m_aapObjectList;   //!< lists with pointers to registered 3d-objects <type, list>
    std::vector<coreObjectCollision> m_aObjectCollision;     //!< currently recorded collisions

    coreModelPtr  m_pLowQuad;                                //!< low-memory square model (4 bytes per vertex, 16 total)
    coreModelPtr  m_pLowTriangle;                            //!< low-memory triangle model (8 bytes per vertex, 24 total)
    coreObject2D* m_pBlitFallback;                           //!< 2d-object used for fallback-blitting onto the default frame buffer

    static coreSet<coreObject2D*> s_apSpriteList;            //!< list with pointers to all existing 2d-objects


private:
    coreObjectManager()noexcept;
    ~coreObjectManager();


public:
    FRIEND_CLASS(Core)
    FRIEND_CLASS(coreObject2D)
    FRIEND_CLASS(coreObject3D)
    DISABLE_COPY(coreObjectManager)

    /*! test collision between different structures */
    //! @{
    template <typename F> void TestCollision(const coreInt32     iType,                                                             F&& nCallback);   //!< [](coreObject3D* OUTPUT pObjectA,     coreObject3D* OUTPUT pObjectB,     const coreVector3& vIntersection, const coreBool bFirstHit) -> void
    template <typename F> void TestCollision(const coreInt32     iType1,   const coreInt32     iType2,                              F&& nCallback);   //!< [](coreObject3D* OUTPUT pObjectType1, coreObject3D* OUTPUT pObjectType2, const coreVector3& vIntersection, const coreBool bFirstHit) -> void
    template <typename F> void TestCollision(const coreInt32     iType,    const coreObject3D* pObject,                             F&& nCallback);   //!< [](coreObject3D* OUTPUT pObject,                                         const coreVector3& vIntersection, const coreBool bFirstHit) -> void
    template <typename F> void TestCollision(const coreInt32     iType,    const coreVector3&  vRayPos, const coreVector3& vRayDir, F&& nCallback);   //!< [](coreObject3D* OUTPUT pObject,                                         const coreFloat    fDistance,     const coreBool bFirstHit) -> void
    static coreBool            TestCollision(const coreObject3D* pObject1, const coreObject3D* pObject2,                            coreVector3* OUTPUT pvIntersection);
    static coreBool            TestCollision(const coreObject3D* pObject,  const coreVector3&  vRayPos, const coreVector3& vRayDir, coreFloat*   OUTPUT pfDistance);
    //! @}

    /*! get manager properties */
    //! @{
    inline const coreObjectList& GetObjectList  (const coreInt32 iType)const {return m_aapObjectList.at(iType);}
    inline const coreModelPtr&   GetLowQuad     ()const                      {return m_pLowQuad;}
    inline const coreModelPtr&   GetLowTriangle ()const                      {return m_pLowTriangle;}
    inline       coreObject2D*   GetBlitFallback()const                      {return m_pBlitFallback;}
    //! @}


private:
    /*! reset with the resource manager */
    //! @{
    void __Reset(const coreResourceReset bInit);
    //! @}

    /*! update all objects and collisions */
    //! @{
    void __UpdateObjects();
    //! @}

    /*! bind and unbind 3d-objects to types */
    //! @{
    void __BindObject  (coreObject3D* pObject, const coreInt32 iType);
    void __UnbindObject(coreObject3D* pObject, const coreInt32 iType);
    //! @}

    /*! bind and unbind 2d-objects */
    //! @{
    inline void __BindSprite  (coreObject2D* pSprite) {ASSERT(!s_apSpriteList.count(pSprite)) s_apSpriteList.insert(pSprite);}
    inline void __UnbindSprite(coreObject2D* pSprite) {ASSERT( s_apSpriteList.count(pSprite)) s_apSpriteList.erase (pSprite);}
    //! @}

    /*! handle and track new collisions */
    //! @{
    coreBool __NewCollision(const coreObject3D* pObject1, const coreObject3D* pObject2);
    //! @}
};


// ****************************************************************
/* test collision within a list */
template <typename F> void coreObjectManager::TestCollision(const coreInt32 iType, F&& nCallback)
{
    ASSERT(iType)

    // get requested list
    const coreObjectList& oList = m_aapObjectList[iType];

    // loop through all objects
    for(coreUintW i = 0u, ie = oList.size(); i < ie; ++i)
    {
        coreObject3D* pObject1 = oList[i];
        if(!pObject1) continue;

        for(coreUintW j = i + 1u; j < ie; ++j)
        {
            coreObject3D* pObject2 = oList[j];
            if(!pObject2) continue;

            // test collision and call function
            coreVector3 vIntersection;
            if(coreObjectManager::TestCollision(pObject1, pObject2, &vIntersection))
            {
                nCallback(s_cast<typename TRAIT_ARG_TYPE(F, 0u)>(pObject1),
                          s_cast<typename TRAIT_ARG_TYPE(F, 1u)>(pObject2),
                          vIntersection, this->__NewCollision(pObject1, pObject2));
            }
        }
    }
}


// ****************************************************************
/* test collision between two different lists */
template <typename F> void coreObjectManager::TestCollision(const coreInt32 iType1, const coreInt32 iType2, F&& nCallback)
{
    ASSERT(iType1 && iType2 && (iType1 != iType2))

    // make sure both lists are available (first reference may get invalid otherwise)
    m_aapObjectList[iType2]; m_aapObjectList[iType1];

    // get requested lists
    const coreObjectList& oList1 = m_aapObjectList.at(iType1);
    const coreObjectList& oList2 = m_aapObjectList.at(iType2);

    // loop through all objects
    for(coreUintW i = 0u, ie = oList1.size(); i < ie; ++i)
    {
        coreObject3D* pObject1 = oList1[i];
        if(!pObject1) continue;

        for(coreUintW j = 0u, je = oList2.size(); j < je; ++j)
        {
            coreObject3D* pObject2 = oList2[j];
            if(!pObject2) continue;

            // test collision and call function
            coreVector3 vIntersection;
            if(coreObjectManager::TestCollision(pObject1, pObject2, &vIntersection))
            {
                nCallback(s_cast<typename TRAIT_ARG_TYPE(F, 0u)>(pObject1),
                          s_cast<typename TRAIT_ARG_TYPE(F, 1u)>(pObject2),
                          vIntersection, this->__NewCollision(pObject1, pObject2));
            }
        }
    }
}


// ****************************************************************
/* test collision between list and 3d-object */
template <typename F> void coreObjectManager::TestCollision(const coreInt32 iType, const coreObject3D* pObject, F&& nCallback)
{
    ASSERT(iType && pObject)

    // get requested list
    const coreObjectList& oList = m_aapObjectList[iType];

    // loop through all objects
    for(coreUintW i = 0u, ie = oList.size(); i < ie; ++i)
    {
        coreObject3D* pCurObject = oList[i];
        if(!pCurObject) continue;

        // test collision and call function
        coreVector3 vIntersection;
        if(coreObjectManager::TestCollision(pCurObject, pObject, &vIntersection))
        {
            nCallback(s_cast<typename TRAIT_ARG_TYPE(F, 0u)>(pCurObject),
                      vIntersection, this->__NewCollision(pCurObject, pObject));
        }
    }
}


// ****************************************************************
/* test collision between list and ray */
template <typename F> void coreObjectManager::TestCollision(const coreInt32 iType, const coreVector3& vRayPos, const coreVector3& vRayDir, F&& nCallback)
{
    ASSERT(iType)

    // get requested list
    const coreObjectList& oList = m_aapObjectList[iType];

    // loop through all objects
    for(coreUintW i = 0u, ie = oList.size(); i < ie; ++i)
    {
        coreObject3D* pCurObject = oList[i];
        if(!pCurObject) continue;

        // test collision and call function
        coreFloat fDistance;
        if(coreObjectManager::TestCollision(pCurObject, vRayPos, vRayDir, &fDistance))
        {
            nCallback(s_cast<typename TRAIT_ARG_TYPE(F, 0u)>(pCurObject),
                      fDistance, this->__NewCollision(pCurObject, r_cast<coreObject3D*>(&nCallback)));
        }
    }
}


#endif /* _CORE_GUARD_OBJECT_H_ */