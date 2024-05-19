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

#define CORE_SCOPE_TEMPLATE(c,t,f,g) template <typename t> CORE_SCOPE_CLASS(c, t, f, g)


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
    inline T*                      Get()const {ASSERT(m_ptObject) return m_ptObject;}
    inline          operator       T* ()      {return m_ptObject;}
    constexpr       operator const T* ()const {return m_ptObject;}
    inline explicit operator coreBool ()const {return m_ptObject != NULL;}
    inline T*       operator ->       ()const {return  this->Get();}
    inline T&       operator *        ()const {return *this->Get();}
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

    // handle re-assignment
    if(m_ptObject == m.m_ptObject) m.m_ptObject = NULL;

    return *this;
}


// ****************************************************************
/* default scope types */
CORE_SCOPE_CLASS   (coreFileScope,    coreFile,    {if(m_ptObject) m_ptObject->Acquire();}, {if(m_ptObject) m_ptObject->Release();})
CORE_SCOPE_CLASS   (coreSurfaceScope, SDL_Surface, {},                                      {if(m_ptObject) SDL_FreeSurface(m_ptObject);})
CORE_SCOPE_TEMPLATE(coreDataScope,    T,           {},                                      {SAFE_DELETE_ARRAY(this->m_ptObject)})
CORE_SCOPE_TEMPLATE(coreSDLScope,     T,           {},                                      {SDL_free(this->m_ptObject);})


#endif /* _CORE_GUARD_SCOPE_H_ */