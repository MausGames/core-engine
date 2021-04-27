///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SCOPE_H_
#define _CORE_GUARD_SCOPE_H_


// ****************************************************************
/* scope definitions */
#define CORE_SCOPE_CLASS(c,t,f)                                     \
    class c final : public coreScope<t>                             \
    {                                                               \
    public:                                                         \
        constexpr c(t* pObject)noexcept : coreScope<t> (pObject) {} \
        ~c() {f}                                                    \
                                                                    \
        ENABLE_COPY(c)                                              \
    };


// ****************************************************************
/* scope interface */
template <typename T> class INTERFACE coreScope
{
protected:
    T* m_ptObject;   // associated object


protected:
    constexpr coreScope(T* ptObject)noexcept;
    coreScope(coreScope<T>&& m)noexcept;
    ~coreScope() = default;


public:
    DISABLE_HEAP

    /* assignment operations */
    coreScope<T>& operator = (coreScope<T>&& m)noexcept;

    /* access associated object */
    inline          operator       T* ()      {return m_ptObject;}
    constexpr       operator const T* ()const {return m_ptObject;}
    inline explicit operator coreBool ()const {return m_ptObject != NULL;}
    inline T*       operator ->       ()const {ASSERT(m_ptObject) return  m_ptObject;}
    inline T&       operator *        ()const {ASSERT(m_ptObject) return *m_ptObject;}
};


// ****************************************************************
/* constructor */
template <typename T> constexpr coreScope<T>::coreScope(T* ptObject)noexcept
: m_ptObject (ptObject)
{
}

template <typename T> coreScope<T>::coreScope(coreScope<T>&& m)noexcept
: m_ptObject (m.m_ptObject)
{
    m.m_ptObject = NULL;
}


// ****************************************************************
/* assignment operations */
template <typename T> coreScope<T>& coreScope<T>::operator = (coreScope<T>&& m)noexcept
{
    // swap properties
    std::swap(m_ptObject, m.m_ptObject);

    return *this;
}


// ****************************************************************
/* default scope types */
CORE_SCOPE_CLASS(coreFileScope,    coreFile,    {if(m_ptObject) m_ptObject->UnloadData();})
CORE_SCOPE_CLASS(coreSurfaceScope, SDL_Surface, {if(m_ptObject) SDL_FreeSurface(m_ptObject);})


#endif /* _CORE_GUARD_SCOPE_H_ */