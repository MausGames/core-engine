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


// ****************************************************************
// base-object extension
// TODO: re-implement relative behavior (additional classes?)
class coreObject
{
protected:
    coreTexturePtr m_apTexture[CORE_TEXTURE_UNITS];   //!< multiple texture objects
    coreProgramShr m_pProgram;                        //!< shader-program object

    coreMatrix4 m_mRotation;                          //!< separate rotation matrix
    coreMatrix4 m_mTransform;                         //!< transformation matrix
    coreByte m_iUpdate;                               //!< update status (0 = do nothing | 1 = update only transformation | 3 = update rotation and transformation)

    coreVector4 m_vColor;                             //!< RGBA color-value
    coreVector2 m_vTexSize;                           //!< size-factor of the texture
    coreVector2 m_vTexOffset;                         //!< offset of the texture

    int m_iStatus;                                    //!< numeric status-value for individual use


protected:
    constexpr_obj coreObject()noexcept;
    ~coreObject() {}


public:
    //! define the visual appearance
    //! @{
    const coreTexturePtr& DefineTextureFile(const coreByte& iUnit, const char* pcPath);
    const coreTexturePtr& DefineTextureLink(const coreByte& iUnit, const char* pcName);
    const coreProgramShr& DefineProgramShare(const char* pcName);
    //! @}

    //! set object attributes
    //! @{
    inline void SetColor4(const coreVector4& vColor)        {m_vColor     = vColor;}
    inline void SetColor3(const coreVector3& vColor)        {m_vColor.xyz(vColor);}
    inline void SetAlpha(const float& fAlpha)               {m_vColor.a   = fAlpha;}
    inline void SetTexSize(const coreVector2& vTexSize)     {m_vTexSize   = vTexSize;}
    inline void SetTexOffset(const coreVector2& vTexOffset) {m_vTexOffset = vTexOffset;}
    inline void SetStatus(const int& iStatus)               {m_iStatus    = iStatus;}
    //! @}

    //! get object attributes
    //! @{
    inline const coreTexturePtr& GetTexture(const coreByte& iUnit)const {SDL_assert(iUnit < CORE_TEXTURE_UNITS); return m_apTexture[iUnit];}
    inline const coreProgramShr& GetProgram()const                      {return m_pProgram;}
    inline const coreMatrix4& GetRotation()const                        {return m_mRotation;}
    inline const coreMatrix4& GetTransform()const                       {return m_mTransform;}
    inline const coreVector4& GetColor4()const                          {return m_vColor;}
    inline const coreVector3& GetColor3()const                          {return m_vColor.xyz();}
    inline const float& GetAlpha()const                                 {return m_vColor.a;}
    inline const coreVector2& GetTexSize()const                         {return m_vTexSize;}
    inline const coreVector2& GetTexOffset()const                       {return m_vTexOffset;}
    inline const int& GetStatus()const                                  {return m_iStatus;}
    //! @}
};


// ****************************************************************
// object manager
class coreObjectManager final
{
private:


private:
    coreObjectManager()noexcept;
    ~coreObjectManager();
    friend class Core;


public:

};


// ****************************************************************
// constructor
constexpr_obj coreObject::coreObject()noexcept
: m_mRotation  (coreMatrix4::Identity())
, m_mTransform (coreMatrix4::Identity())
, m_iUpdate    (3)
, m_vColor     (coreVector4(1.0f,1.0f,1.0f,1.0f))
, m_vTexSize   (coreVector2(1.0f,1.0f))
, m_vTexOffset (coreVector2(0.0f,0.0f))
, m_iStatus    (0)
{
}


#endif // _CORE_GUARD_OBJECT_H_