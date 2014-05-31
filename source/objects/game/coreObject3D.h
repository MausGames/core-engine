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


// ****************************************************************
// 3d-object class
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
    int m_iType;               //!< identification of the object type (0 = undefined)


public:
    constexpr_obj coreObject3D()noexcept;
    virtual ~coreObject3D() {}
    friend class coreObjectManager;

    //! define the visual appearance
    //! @{
    inline void DefineModel(const coreModelPtr& pModel) {m_pModel = pModel;}
    const coreModelPtr& DefineModelFile(const char* pcPath);
    const coreModelPtr& DefineModelLink(const char* pcName);
    void Undefine();
    //! @}

    //! separately enable all resources for rendering
    //! @{
    inline bool Enable() {return coreObject3D::Enable(m_pProgram);}
    bool Enable(const coreProgramShr& pProgram) hot_func;
    //! @}

    //! render and move the 3d-object
    //! @{
    inline virtual void Render() {coreObject3D::Render(m_pProgram);}
    virtual void Render(const coreProgramShr& pProgram) hot_func;
    virtual void Move() hot_func;
    //! @}

    //! handle collision between different structures
    //! @{
    static bool Collision(const coreObject3D& Object1, const coreObject3D& Object2);
    static float Collision(const coreObject3D& Object, const coreVector3& vLinePos, const coreVector3& vLineDir);
    //! @}

    //! set object properties
    //! @{
    inline void SetPosition(const coreVector3& vPosition)       {if(m_vPosition != vPosition) {m_iUpdate |= 1; m_vPosition = vPosition;}}
    inline void SetSize(const coreVector3& vSize)               {if(m_vSize     != vSize)     {m_iUpdate |= 1; m_vSize     = vSize;}}
    inline void SetDirection(const coreVector3& vDirection)     {const coreVector3 vDirNorm = vDirection.Normalized();   if(m_vDirection   != vDirNorm) {m_iUpdate |= 3; m_vDirection   = vDirNorm;}}
    inline void SetOrientation(const coreVector3& vOrientation) {const coreVector3 vOriNorm = vOrientation.Normalized(); if(m_vOrientation != vOriNorm) {m_iUpdate |= 3; m_vOrientation = vOriNorm;}}
    inline void SetCollisionRange(const float& fCollisionRange) {m_fCollisionRange = fCollisionRange;}
    inline void SetType(const int& iType)                       {m_iType           = iType;}
    //! @}

    //! get object properties
    //! @{
    inline const coreModelPtr& GetModel()const      {return m_pModel;}
    inline const coreVector3& GetPosition()const    {return m_vPosition;}
    inline const coreVector3& GetSize()const        {return m_vSize;}
    inline const coreVector3& GetDirection()const   {return m_vDirection;}
    inline const coreVector3& GetOrientation()const {return m_vOrientation;}
    inline const float& GetCollisionRange()const    {return m_fCollisionRange;}
    inline const int& GetType()const                {return m_iType;}
    //! @}
};


// ****************************************************************
// constructor
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


#endif // _CORE_GUARD_OBJECT3D_H_