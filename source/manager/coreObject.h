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
// TODO: new collisions with lines are not saved correctly (no line identification)
// TODO: GL_NV_fill_rectangle for low-memory model (state-tracking for polygon-mode ?)


// ****************************************************************
/* object definitions */
enum coreObjectUpdate : coreByte
{
    CORE_OBJECT_UPDATE_NOTHING   = 0x00,   //!< update nothing
    CORE_OBJECT_UPDATE_TRANSFORM = 0x01,   //!< update transformation values
    CORE_OBJECT_UPDATE_COLLISION = 0x02,   //!< update collision-relevant values
    CORE_OBJECT_UPDATE_ALL       = 0x03    //!< update everything
};
ENABLE_BITWISE(coreObjectUpdate)

enum coreObjectEnable : coreByte
{
    CORE_OBJECT_ENABLE_NOTHING = 0x00,   //!< do nothing
    CORE_OBJECT_ENABLE_RENDER  = 0x01,   //!< enable render routine
    CORE_OBJECT_ENABLE_MOVE    = 0x02,   //!< enable move routine
    CORE_OBJECT_ENABLE_ALL     = 0x03    //!< enable all routines
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
    int m_iStatus;                                    //!< numeric status-value for individual use


protected:
    constexpr_weak coreObject()noexcept;
    inline coreObject(const coreObject& c)noexcept;
    inline coreObject(coreObject&&      m)noexcept;
    ~coreObject() {}


public:
    /*! assignment operations */
    //! @{
    coreObject& operator = (const coreObject& c)noexcept;
    coreObject& operator = (coreObject&&      m)noexcept;
    //! @}

    /*! define the visual appearance */
    //! @{
    inline const coreTexturePtr& DefineTexture(const coreByte& iUnit, const coreTexturePtr& pTexture) {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = pTexture;                                          return m_apTexture[iUnit];}
    inline const coreTexturePtr& DefineTexture(const coreByte& iUnit, const char*           pcName)   {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = Core::Manager::Resource->Get<coreTexture>(pcName); return m_apTexture[iUnit];}
    inline const coreProgramPtr& DefineProgram(const coreProgramPtr& pProgram)                        {m_pProgram = pProgram;                                          return m_pProgram;}
    inline const coreProgramPtr& DefineProgram(const char*           pcName)                          {m_pProgram = Core::Manager::Resource->Get<coreProgram>(pcName); return m_pProgram;}
    //! @}

    /*! check for enabled object routines */
    //! @{
    inline bool IsEnabled(const coreObjectEnable& iEnabled)const {return (m_iEnabled & iEnabled) ? true : false;}
    //! @}

    /*! set object properties */
    //! @{
    inline void SetColor4   (const coreVector4&      vColor)     {m_vColor     = vColor;}
    inline void SetColor3   (const coreVector3&      vColor)     {m_vColor.xyz(vColor);}
    inline void SetAlpha    (const float&            fAlpha)     {m_vColor.a   = fAlpha;}
    inline void SetTexSize  (const coreVector2&      vTexSize)   {m_vTexSize   = vTexSize;}
    inline void SetTexOffset(const coreVector2&      vTexOffset) {m_vTexOffset = vTexOffset;}
    inline void SetEnabled  (const coreObjectEnable& iEnabled)   {m_iEnabled   = iEnabled;}
    inline void SetStatus   (const int&              iStatus)    {m_iStatus    = iStatus;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreTexturePtr& GetTexture  (const coreByte& iUnit)const {ASSERT(iUnit < CORE_TEXTURE_UNITS) return m_apTexture[iUnit];}
    inline const coreProgramPtr& GetProgram  ()const                      {return m_pProgram;}
    inline const coreVector4&    GetColor4   ()const                      {return m_vColor;}
    inline coreVector3           GetColor3   ()const                      {return m_vColor.xyz();}
    inline const float&          GetAlpha    ()const                      {return m_vColor.a;}
    inline const coreVector2&    GetTexSize  ()const                      {return m_vTexSize;}
    inline const coreVector2&    GetTexOffset()const                      {return m_vTexOffset;}
    inline const int&            GetStatus   ()const                      {return m_iStatus;}
    //! @}
};


// ****************************************************************
/* object manager */
class coreObjectManager final : public coreResourceRelation
{
private:
    /*! internal types */
    typedef std::vector<coreObject3D*> coreObjectList;

    /*! object collision structure */
    struct coreObjectCollision
    {
        const coreObject3D* pObject1;     //!< first 3d-object
        const coreObject3D* pObject2;     //!< second 3d-object
        coreUint            iLastFrame;   //!< frame number of their last collision

        constexpr_func coreObjectCollision()noexcept;
    };


private:
    coreLookup<int, coreObjectList>  m_aapObjectList;      //!< lists with pointers to registered 3d-objects
    std::vector<coreObjectCollision> m_aObjectCollision;   //!< currently recorded collisions

    coreModelPtr  m_pLowModel;                             //!< low-memory model object (4 bytes per vertex, simple square)
    coreObject2D* m_pBlitFallback;                         //!< 2d-object used for fallback-blitting onto the default frame buffer


private:
    coreObjectManager()noexcept;
    ~coreObjectManager();


public:
    FRIEND_CLASS(Core)
    FRIEND_CLASS(coreObject3D)
    DISABLE_COPY(coreObjectManager)

    /*! test collision between different structures */
    //! @{
    template <typename F> void TestCollision(const int&          iType,                                                               F&& nCallback);
    template <typename F> void TestCollision(const int&          iType1,   const int&          iType2,                                F&& nCallback);
    template <typename F> void TestCollision(const int&          iType,    const coreObject3D* pObject,                               F&& nCallback);
    template <typename F> void TestCollision(const int&          iType,    const coreVector3&  vLinePos, const coreVector3& vLineDir, F&& nCallback);
    static bool                TestCollision(const coreObject3D* pObject1, const coreObject3D* pObject2);
    static float               TestCollision(const coreObject3D* pObject,  const coreVector3&  vLinePos, const coreVector3& vLineDir);
    //! @}

    /*! get manager properties */
    //! @{
    inline const coreObjectList& GetObjectList  (const int& iType)const {return m_aapObjectList.at(iType);}
    inline const coreModelPtr&   GetLowModel    ()const                 {return m_pLowModel;}
    inline coreObject2D*         GetBlitFallback()const                 {return m_pBlitFallback;}
    //! @}


private:
    /*! reset with the resource manager */
    //! @{
    void __Reset(const coreResourceReset& bInit)override;
    //! @}

    /*! update all objects and collisions */
    //! @{
    void __UpdateObjects();
    //! @}

    /*! bind and unbind 3d-objects to types */
    //! @{
    void __BindObject  (coreObject3D* pObject, const int& iType);
    void __UnbindObject(coreObject3D* pObject, const int& iType);
    //! @}

    /*! handle and track new collisions */
    //! @{
    bool __NewCollision(const coreObject3D* pObject1, const coreObject3D* pObject2);
    //! @}
};


// ****************************************************************
/* constructor */
constexpr_weak coreObject::coreObject()noexcept
: m_vColor     (coreVector4(1.0f,1.0f,1.0f,1.0f))
, m_vTexSize   (coreVector2(1.0f,1.0f))
, m_vTexOffset (coreVector2(0.0f,0.0f))
, m_iUpdate    (CORE_OBJECT_UPDATE_ALL)
, m_iEnabled   (CORE_OBJECT_ENABLE_ALL)
, m_iStatus    (0)
{
}

inline coreObject::coreObject(const coreObject& c)noexcept
: m_pProgram   (c.m_pProgram)
, m_vColor     (c.m_vColor)
, m_vTexSize   (c.m_vTexSize)
, m_vTexOffset (c.m_vTexOffset)
, m_iUpdate    (c.m_iUpdate)
, m_iEnabled   (c.m_iEnabled)
, m_iStatus    (c.m_iStatus)
{
    // copy texture objects
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i)
        m_apTexture[i] = c.m_apTexture[i];
}

inline coreObject::coreObject(coreObject&& m)noexcept
: m_pProgram   (std::move(m.m_pProgram))
, m_vColor     (m.m_vColor)
, m_vTexSize   (m.m_vTexSize)
, m_vTexOffset (m.m_vTexOffset)
, m_iUpdate    (m.m_iUpdate)
, m_iEnabled   (m.m_iEnabled)
, m_iStatus    (m.m_iStatus)
{
    // move texture objects
    for(coreByte i = 0; i < CORE_TEXTURE_UNITS; ++i)
        m_apTexture[i] = std::move(m.m_apTexture[i]);
}


// ****************************************************************
/* constructor */
constexpr_func coreObjectManager::coreObjectCollision::coreObjectCollision()noexcept
: pObject1   (NULL)
, pObject2   (NULL)
, iLastFrame (0)
{
}


// ****************************************************************
/* test collision within a list */
template <typename F> void coreObjectManager::TestCollision(const int& iType, F&& nCallback)
{
    ASSERT(iType)

    // get requested list
    const coreObjectList& oList = m_aapObjectList[iType];

    // loop through all objects
    FOR_EACH(it, oList)
    {
        coreObject3D* pObject1 = (*it);
        if(!pObject1) continue;

        FOR_EACH_SET(et, it+1, oList)
        {
            coreObject3D* pObject2 = (*et);
            if(!pObject2) continue;

            // test collision and call function
            if(coreObjectManager::TestCollision(pObject1, pObject2))
            {
                nCallback(s_cast<function_traits<F>::arg<0>::type>(pObject1),
                          s_cast<function_traits<F>::arg<1>::type>(pObject2),
                          this->__NewCollision(pObject1, pObject2));
            }
        }
    }
}


// ****************************************************************
/* test collision between two different lists */
template <typename F> void coreObjectManager::TestCollision(const int& iType1, const int& iType2, F&& nCallback)
{
    ASSERT(iType1 && iType2 && iType1 != iType2)

    // get requested lists
    const coreObjectList& oList1 = m_aapObjectList[iType1];
    const coreObjectList& oList2 = m_aapObjectList[iType2];

    // loop through all objects
    FOR_EACH(it, oList1)
    {
        coreObject3D* pObject1 = (*it);
        if(!pObject1) continue;

        FOR_EACH(et, oList2)
        {
            coreObject3D* pObject2 = (*et);
            if(!pObject2) continue;

            // test collision and call function
            if(coreObjectManager::TestCollision(pObject1, pObject2))
            {
                nCallback(s_cast<function_traits<F>::arg<0>::type>(pObject1),
                          s_cast<function_traits<F>::arg<1>::type>(pObject2),
                          this->__NewCollision(pObject1, pObject2));
            }
        }
    }
}


// ****************************************************************
/* test collision between list and 3d-object */
template <typename F> void coreObjectManager::TestCollision(const int& iType, const coreObject3D* pObject, F&& nCallback)
{
    ASSERT(iType && pObject)

    // get requested list
    const coreObjectList& oList = m_aapObjectList[iType];

    // loop through all objects
    FOR_EACH(it, oList)
    {
        coreObject3D* pCurObject = (*it);
        if(!pCurObject) continue;

        // test collision and call function
        if(coreObjectManager::TestCollision(pCurObject, pObject))
        {
            nCallback(s_cast<function_traits<F>::arg<0>::type>(pCurObject),
                      this->__NewCollision(pCurObject, pObject));
        }
    }
}


// ****************************************************************
/* test collision between list and line */
template <typename F> void coreObjectManager::TestCollision(const int& iType, const coreVector3& vLinePos, const coreVector3& vLineDir, F&& nCallback)
{
    ASSERT(iType)

    // get requested list
    const coreObjectList& oList = m_aapObjectList[iType];

    // loop through all objects
    FOR_EACH(it, oList)
    {
        coreObject3D* pCurObject = (*it);
        if(!pCurObject) continue;

        // test collision and call function
        const float fDistance = coreObjectManager::TestCollision(pCurObject, vLinePos, vLineDir);
        if(fDistance)
        {
            nCallback(s_cast<function_traits<F>::arg<0>::type>(pCurObject),
                      fDistance, this->__NewCollision(pCurObject, NULL));
        }
    }
}


#endif /* _CORE_GUARD_OBJECT_H_ */