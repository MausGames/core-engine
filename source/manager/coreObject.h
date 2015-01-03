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


// ****************************************************************
// object definitions
enum coreObjectUpdate : coreByte
{
    CORE_OBJECT_UPDATE_NOTHING   = 0x00,   //!< update nothing
    CORE_OBJECT_UPDATE_TRANSFORM = 0x01,   //!< update transformation
    CORE_OBJECT_UPDATE_ALL       = 0xFF    //!< update rotation and transformation
};
EXTEND_ENUM(coreObjectUpdate)

enum coreObjectEnable : coreByte
{
    CORE_OBJECT_ENABLE_NOTHING = 0x00,   //!< do nothing
    CORE_OBJECT_ENABLE_RENDER  = 0x01,   //!< enable render routine
    CORE_OBJECT_ENABLE_MOVE    = 0x02,   //!< enable move routine
    CORE_OBJECT_ENABLE_ALL     = 0xFF    //!< enable all routines
};
EXTEND_ENUM(coreObjectEnable)


// ****************************************************************
// object interface
class INTERFACE coreObject
{
protected:
    coreTexturePtr m_apTexture[CORE_TEXTURE_UNITS];   //!< multiple texture objects
    coreProgramPtr m_pProgram;                        //!< shader-program object

    coreVector4 m_vColor;                             //!< RGBA color-value
    coreVector2 m_vTexSize;                           //!< size-factor of the texture
    coreVector2 m_vTexOffset;                         //!< offset of the texture

    coreObjectUpdate m_iUpdate;                       //!< update status
    coreObjectEnable m_iEnabled;                      //!< enabled object routines
    int m_iStatus;                                    //!< numeric status-value for individual use


protected:
    constexpr_weak coreObject()noexcept;
    ~coreObject() {}


public:
    //! define the visual appearance
    //! @{
    inline const coreTexturePtr& DefineTexture(const coreByte& iUnit, const coreTexturePtr& pTexture) {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = pTexture;                                          return m_apTexture[iUnit];}
    inline const coreTexturePtr& DefineTexture(const coreByte& iUnit, const char*           pcName)   {ASSERT(iUnit < CORE_TEXTURE_UNITS) m_apTexture[iUnit] = Core::Manager::Resource->Get<coreTexture>(pcName); return m_apTexture[iUnit];}
    inline const coreProgramPtr& DefineProgram(const coreProgramPtr& pProgram)                        {m_pProgram = pProgram;                                          return m_pProgram;}
    inline const coreProgramPtr& DefineProgram(const char*           pcName)                          {m_pProgram = Core::Manager::Resource->Get<coreProgram>(pcName); return m_pProgram;}
    //! @}

    //! check for enabled object routines
    //! @{
    inline bool IsEnabled(const coreObjectEnable& iEnabled)const {return (m_iEnabled & iEnabled) ? true : false;}
    //! @}

    //! set object properties
    //! @{
    inline void SetColor4   (const coreVector4&      vColor)     {m_vColor     = vColor;}
    inline void SetColor3   (const coreVector3&      vColor)     {m_vColor.xyz(vColor);}
    inline void SetAlpha    (const float&            fAlpha)     {m_vColor.a   = fAlpha;}
    inline void SetTexSize  (const coreVector2&      vTexSize)   {m_vTexSize   = vTexSize;}
    inline void SetTexOffset(const coreVector2&      vTexOffset) {m_vTexOffset = vTexOffset;}
    inline void SetEnabled  (const coreObjectEnable& iEnabled)   {m_iEnabled   = iEnabled;}
    inline void SetStatus   (const int&              iStatus)    {m_iStatus    = iStatus;}
    //! @}

    //! get object properties
    //! @{
    inline const coreTexturePtr&   GetTexture  (const coreByte& iUnit)const {ASSERT(iUnit < CORE_TEXTURE_UNITS) return m_apTexture[iUnit];}
    inline const coreProgramPtr&   GetProgram  ()const                      {return m_pProgram;}
    inline const coreVector4&      GetColor4   ()const                      {return m_vColor;}
    inline coreVector3             GetColor3   ()const                      {return m_vColor.xyz();}
    inline const float&            GetAlpha    ()const                      {return m_vColor.a;}
    inline const coreVector2&      GetTexSize  ()const                      {return m_vTexSize;}
    inline const coreVector2&      GetTexOffset()const                      {return m_vTexOffset;}
    inline const int&              GetStatus   ()const                      {return m_iStatus;}
    //! @}
};


// ****************************************************************
// object manager
class coreObjectManager final : public coreResourceRelation
{
private:


private:
    coreObjectManager()noexcept;
    ~coreObjectManager();
    friend class Core;


public:


private:
    //! reset with the resource manager
    //! @{
    void __Reset(const coreResourceReset& bInit)override;
    //! @}
};


// ****************************************************************
// constructor
constexpr_weak coreObject::coreObject()noexcept
: m_vColor     (coreVector4(1.0f,1.0f,1.0f,1.0f))
, m_vTexSize   (coreVector2(1.0f,1.0f))
, m_vTexOffset (coreVector2(0.0f,0.0f))
, m_iUpdate    (CORE_OBJECT_UPDATE_ALL)
, m_iEnabled   (CORE_OBJECT_ENABLE_ALL)
, m_iStatus    (0)
{
}


#endif // _CORE_GUARD_OBJECT_H_