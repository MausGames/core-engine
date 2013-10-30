//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
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
    coreModelPtr m_pModel;   //!< model object


public:
    constexpr_obj coreObject3D()noexcept;
    virtual ~coreObject3D() {}

    //! define the visual appearance
    //! @{
    const coreModelPtr& DefineModelFile(const char* pcPath);
    const coreModelPtr& DefineModelLink(const char* pcName);
    void Undefine();
    //! @}

    //! render and move the 3d-object
    //! @{
    virtual void Render() {this->coreObject3D::Render(m_pProgram, true);}
    virtual void Render(const coreProgramShr& pProgram, const bool& bTextured);
    virtual void Move();
    //! @}

    //! set object attributes
    //! @{
    inline void SetPosition(const coreVector3& vPosition)       {if(m_vPosition != vPosition) {m_iUpdate |= 1; m_vPosition = vPosition;}}
    inline void SetSize(const coreVector3& vSize)               {if(m_vSize     != vSize)     {m_iUpdate |= 1; m_vSize     = vSize;}}
    inline void SetDirection(const coreVector3& vDirection)     {const coreVector3 vDirNorm = vDirection.Normalized();   if(m_vDirection   != vDirNorm) {m_iUpdate |= 3; m_vDirection   = vDirNorm;}}
    inline void SetOrientation(const coreVector3& vOrientation) {const coreVector3 vOriNorm = vOrientation.Normalized(); if(m_vOrientation != vOriNorm) {m_iUpdate |= 3; m_vOrientation = vOriNorm;}}
    //! @}

    //! get object attributes
    //! @{
    inline const coreModelPtr& GetModel()const      {return m_pModel;}
    inline const coreVector3& GetPosition()const    {return m_vPosition;}
    inline const coreVector3& GetSize()const        {return m_vSize;}
    inline const coreVector3& GetDirection()const   {return m_vDirection;}
    inline const coreVector3& GetOrientation()const {return m_vOrientation;}
    //! @}
};


// ****************************************************************
// constructor
constexpr_obj coreObject3D::coreObject3D()noexcept
: m_vPosition    (coreVector3(0.0f,0.0f,0.0f))
, m_vSize        (coreVector3(1.0f,1.0f,1.0f))
, m_vDirection   (coreVector3(0.0f,1.0f,0.0f))
, m_vOrientation (coreVector3(0.0f,0.0f,1.0f))
{
}


#endif // _CORE_GUARD_OBJECT3D_H_