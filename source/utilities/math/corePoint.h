///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_POINT_H_
#define _CORE_GUARD_POINT_H_

// TODO 3: resolutions (monitor, textures, framebuffers, etc.) (default res should stay vector, for calculations ?)
// TODO 3: colors (? + conversion functions)


// ****************************************************************
/* point class */
template <typename T, coreUintW iSize> class corePoint final
{
private:
    T m_atValue[iSize];   // point elements


public:
    corePoint() = default;
    template <typename... A> constexpr explicit corePoint(const T& tValue, A&&... vArgs)noexcept : m_atValue {tValue, std::forward<A>(vArgs)...} {STATIC_ASSERT(1u + sizeof...(A) == iSize)}

    ENABLE_COPY(corePoint)
    ENABLE_COMPARISON(corePoint)

    /* access specific element */
    inline          T& operator [] (const coreUintW iIndex)      {ASSERT(iIndex < iSize) return m_atValue[iIndex];}
    constexpr const T& operator [] (const coreUintW iIndex)const {ASSERT(iIndex < iSize) return m_atValue[iIndex];}

    /* operate on all elements */
    constexpr corePoint  operator +  (const corePoint p)const {corePoint P; for(coreUintW i = 0u; i < iSize; ++i) P.m_atValue[i] = m_atValue[i] + p.m_atValue[i]; return P;}
    constexpr corePoint  operator -  (const corePoint p)const {corePoint P; for(coreUintW i = 0u; i < iSize; ++i) P.m_atValue[i] = m_atValue[i] - p.m_atValue[i]; return P;}
    constexpr corePoint  operator *  (const corePoint p)const {corePoint P; for(coreUintW i = 0u; i < iSize; ++i) P.m_atValue[i] = m_atValue[i] * p.m_atValue[i]; return P;}
    constexpr corePoint  operator /  (const corePoint p)const {corePoint P; for(coreUintW i = 0u; i < iSize; ++i) P.m_atValue[i] = m_atValue[i] / p.m_atValue[i]; return P;}
    inline    corePoint& operator += (const corePoint p)      {return (*this = *this + p);}
    inline    corePoint& operator -= (const corePoint p)      {return (*this = *this - p);}
    inline    corePoint& operator *= (const corePoint p)      {return (*this = *this * p);}
    inline    corePoint& operator /= (const corePoint p)      {return (*this = *this / p);}
};


// ****************************************************************
/* default point types */
using corePoint2U8  = corePoint<coreUint8,  2u>;
using corePoint2U16 = corePoint<coreUint16, 2u>;
using corePoint3U8  = corePoint<coreUint8,  3u>;
using corePoint3U16 = corePoint<coreUint16, 3u>;
using coreUint128   = corePoint<coreUint64, 2u>;


// ****************************************************************
/* additional checks */
STATIC_ASSERT(std::is_trivial_v<corePoint2U8>)


#endif /* _CORE_GUARD_POINT_H_ */