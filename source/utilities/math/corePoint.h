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

// NOTE: greater-than and less-than use three-way subobject-wise comparison


// ****************************************************************
/* point class */
template <typename T, coreUintW iSize> class corePoint final
{
private:
    T m_atValue[iSize];   // point elements


public:
    corePoint() = default;
    template <typename... A> constexpr explicit corePoint(const T tValue, A&&... vArgs)noexcept : m_atValue {tValue, T(std::forward<A>(vArgs))...} {STATIC_ASSERT(1u + sizeof...(A) == iSize)}

    ENABLE_COPY(corePoint)
    ENABLE_COMPARISON(corePoint)

    /* access specific element */
    constexpr explicit operator coreBool ()const            {for(coreUintW i = 0u; i < iSize; ++i) if(m_atValue[i]) return true; return false;}
    inline          T& operator [] (const coreUintW i)      {ASSERT(i < iSize) return m_atValue[i];}
    constexpr const T& operator [] (const coreUintW i)const {ASSERT(i < iSize) return m_atValue[i];}
    constexpr const T* ptr()const                           {return m_atValue;}

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
using corePoint2I32 = corePoint<coreInt32,  2u>;
using corePoint3U8  = corePoint<coreUint8,  3u>;
using corePoint3U16 = corePoint<coreUint16, 3u>;
using corePoint3I32 = corePoint<coreInt32,  3u>;
using corePoint4U8  = corePoint<coreUint8,  4u>;
using corePoint4U16 = corePoint<coreUint16, 4u>;
using corePoint4I32 = corePoint<coreInt32,  4u>;
using coreUint128   = corePoint<coreUint64, 2u>;


// ****************************************************************
/* additional checks */
STATIC_ASSERT(IS_TRIVIAL(corePoint2U8))


#endif /* _CORE_GUARD_POINT_H_ */