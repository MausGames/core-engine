//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// check if inside field-of-view
bool coreMath::CheckFOV(const coreVector3& vPosition, const float& fFOV, const coreVector3& vCamPosition, const coreVector3& vCamDirection)noexcept
{
    // calculate relative position and angle
    const coreVector3 vRelative = (vPosition - vCamPosition).Normalize();
    const float fDot = coreVector3::Dot(vRelative, vCamDirection);

    // check result
    return (fDot < 0.0f) ? false : (fDot > coreMath::Cos(fFOV));
}


// ****************************************************************
// convert HSV-color to RGB-color
coreVector3 coreMath::HSVtoRGB(const coreVector3& vHSV)noexcept
{
    const float& H = vHSV.x;
    const float& S = vHSV.y;
    const float& V = vHSV.z;

    if(S == 0.0f) return coreVector3(V, V, V);

    const int   h = (int)std::floor(H * 6.0f);
    const float f = (H * 6.0f) - h;

    const float p = V * (1 - S);
    const float q = V * (1 - S*f);
    const float t = V * (1 - S*(1-f));

    switch(h)
    {
    case 0:
    case 6: return coreVector3(V, t, p);
    case 1: return coreVector3(q, V, p);
    case 2: return coreVector3(p, V, t);
    case 3: return coreVector3(p, q, V);
    case 4: return coreVector3(t, p, V);
    case 5: return coreVector3(V, p, q);
    }

    return coreVector3(0.0f,0.0f,0.0f);
}


// ****************************************************************
// convert RGB-color to HSV-color
coreVector3 coreMath::RGBtoHSV(const coreVector3& vRGB)noexcept
{
    // TODO: implement function
    return coreVector3(0.0f,0.0f,0.0f);
}