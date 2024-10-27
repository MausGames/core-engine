///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SCOPE_H_
#define _CORE_GUARD_SCOPE_H_

// NOTE: assigning the same object to multiple scopes is not supported and not detected


// ****************************************************************
/* scope definitions */
#define CORE_SCOPE_CLASS(c,t,f,g)                                    \
    class c final : public coreScope<t>                              \
    {                                                                \
    public:                                                          \
        constexpr c(t* pObject)noexcept : coreScope<t> (pObject) {f} \
        ~c() {g}                                                     \
                                                                     \
        ENABLE_COPY(c)                                               \
    };


// ****************************************************************
/* scope interface */
template <typename T> class INTERFACE coreScope
{
protected:
    T* m_ptObject;   // associated object


protected:
    constexpr coreScope(T* ptObject)noexcept;
    coreScope(coreScope&& m)noexcept;
    ~coreScope() = default;


public:
    DISABLE_HEAP

    /* assignment operations */
    coreScope& operator = (coreScope&& m)noexcept;

    /* access associated object */
    constexpr T*                      Get()const {ASSERT(m_ptObject) return m_ptObject;}
    inline             operator       T* ()      {return m_ptObject;}
    constexpr          operator const T* ()const {return m_ptObject;}
    constexpr explicit operator coreBool ()const {return m_ptObject != NULL;}
    constexpr T*       operator ->       ()const {return  this->Get();}
    constexpr T&       operator *        ()const {return *this->Get();}
};


// ****************************************************************
/* constructor */
template <typename T> constexpr coreScope<T>::coreScope(T* ptObject)noexcept
: m_ptObject (ptObject)
{
}

template <typename T> coreScope<T>::coreScope(coreScope&& m)noexcept
: m_ptObject (m.m_ptObject)
{
    m.m_ptObject = NULL;
}


// ****************************************************************
/* assignment operations */
template <typename T> coreScope<T>& coreScope<T>::operator = (coreScope&& m)noexcept
{
    // swap properties
    std::swap(m_ptObject, m.m_ptObject);

    // handle re-assignment
    if(m_ptObject == m.m_ptObject) m.m_ptObject = NULL;

    return *this;
}


// ****************************************************************
/* default scope types */
CORE_SCOPE_CLASS(coreFileScope,    coreFile,    {if(m_ptObject) m_ptObject->Acquire();}, {if(m_ptObject) m_ptObject->Release();})
CORE_SCOPE_CLASS(corePathScope,    coreChar,    {},                                      {if(m_ptObject) SDL_free       (m_ptObject);})
CORE_SCOPE_CLASS(coreSurfaceScope, SDL_Surface, {},                                      {if(m_ptObject) SDL_FreeSurface(m_ptObject);})

template <typename T> CORE_SCOPE_CLASS(coreDataScope, T, {}, {SAFE_DELETE_ARRAY(this->m_ptObject)})


#endif /* _CORE_GUARD_SCOPE_H_ */