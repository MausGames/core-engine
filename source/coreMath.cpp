//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// calculate square root
float coreMath::Sqrt(float fInput)
{
    // check input
    if(fInput == 0.0f) return fInput;
    if(fInput  < 0.0f) fInput = -fInput;

    // optimized
    if(Core::System->SupportSSE2())
    {
        _mm_store_ss(&fInput, _mm_sqrt_ss(_mm_load_ss(&fInput)));
        return fInput;
    }
    
    // normal
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    return sqrtf(fInput);
#else
    const float fValue     = fInput;
    const float fHalfValue = fInput*0.5f;
    coreUint* piPointer    = (coreUint*)&fInput;
    *piPointer             = (0xbe6f0000-*piPointer)>>1;

    fInput *= 1.5f - fInput*fInput*fHalfValue;
    fInput *= 1.5f - fInput*fInput*fHalfValue;

    return fValue*fInput;
#endif
}


// ****************************************************************
// calculate sinus value
float coreMath::Sin(const float& fInput)
{
    // TODO: add SSE-support
    return sinf(fInput);
}


// ****************************************************************
// calculate cosinus value
float coreMath::Cos(const float& fInput)
{
    // TODO: add SSE-support
    return cosf(fInput);
}


// ****************************************************************
// check if inside field-of-view
bool coreMath::CheckFOV(const coreVector3& vPosition, const float& fFOV, const coreVector3& vCamPosition, const coreVector3& vCamDirection)
{
    // calculate relative position and angle
    const coreVector3 vRelative = (vPosition - vCamPosition).Normalize();
    const float fDot = coreVector3::Dot(vRelative, vCamDirection);

    // check result
    if(fDot < 0.0f) return false;
    return (fDot > coreMath::Cos(fFOV));
}


// ****************************************************************
// convert HSV-color to RGB-color
coreVector3 coreMath::HSVtoRGB(const coreVector3& vHSV)
{
    const float& H = vHSV.x;
    const float& S = vHSV.y;
    const float& V = vHSV.z;

    if(S == 0.0f) return coreVector3(V, V, V);

    const int   h = (int)floorf(H * 6.0f);
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
coreVector3 coreMath::RGBtoHSV(const coreVector3& vRGB)
{
    // TODO: implement function
    return coreVector3(0.0f,0.0f,0.0f);
}