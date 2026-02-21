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

// TODO 3: look into SDL_PROP_NAME_STRING (in constructor ?)

// NOTE: assigning the same object to multiple scopes is not supported and not detected


// ****************************************************************
/* scope definitions */
#define CORE_SCOPE_CLASS(c,t,f,g)                                 \
    class c final : public coreScope<t>                           \
    {                                                             \
    public:                                                       \
        c(t* pObject = NULL)noexcept : coreScope<t> (pObject) {f} \
        ~c()                                                  {g} \
                                                                  \
        ENABLE_COPY(c)                                            \
    };

#define DEFER(x) const coreDefer CONCAT(__d, __LINE__)([&]() {x});


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
    CUSTOM_COPY(coreScope)
    DISABLE_HEAP

    /* assignment operations */
    coreScope& operator = (coreScope&& m)noexcept;

    /* access associated object */
    constexpr T*                      Get()const {ASSERT(m_ptObject) return m_ptObject;}
    constexpr          operator       T* ()      {return m_ptObject;}
    constexpr          operator const T* ()const {return m_ptObject;}
    constexpr explicit operator coreBool ()const {return m_ptObject != NULL;}
    constexpr T*       operator ->       ()const {return  this->Get();}
    constexpr T&       operator *        ()const {return *this->Get();}
};


// ****************************************************************
/* deferred execution helper */
template <typename F> class coreDefer final : public F
{
public:
    constexpr coreDefer(F&& nFunc)noexcept;
    ~coreDefer();

    DISABLE_COPY(coreDefer)
    DISABLE_HEAP
};


// ****************************************************************
/* property group wrapper */
class coreProperties final
{
private:
    SDL_PropertiesID m_iPropertiesID;   // property group identifier


public:
    coreProperties()noexcept;
    ~coreProperties();

    DISABLE_COPY(coreProperties)
    DISABLE_HEAP

    /* access property group identifier */
    inline operator SDL_PropertiesID ()const {return m_iPropertiesID;}
};


// ****************************************************************
/* constructor */
template <typename T> constexpr coreScope<T>::coreScope(T* ptObject)noexcept
: m_ptObject (ptObject)
{
}

template <typename T> coreScope<T>::coreScope(coreScope&& m)noexcept
: m_ptObject (std::exchange(m.m_ptObject, NULL))
{
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
/* constructor */
template <typename F> constexpr coreDefer<F>::coreDefer(F&& nFunc)noexcept
: F (std::forward<F>(nFunc))
{
}


// ****************************************************************
/* destructor */
template <typename F> coreDefer<F>::~coreDefer()
{
    F::operator()();
}


// ****************************************************************
/* constructor */
inline coreProperties::coreProperties()noexcept
: m_iPropertiesID (0u)
{
    m_iPropertiesID = SDL_CreateProperties();
    ASSERT(m_iPropertiesID)
}


// ****************************************************************
/* destructor */
inline coreProperties::~coreProperties()
{
    SDL_DestroyProperties(m_iPropertiesID);
}


// ****************************************************************
/* default scope types */
CORE_SCOPE_CLASS(coreFileScope,      coreFile,      {if(m_ptObject) m_ptObject->Acquire();}, {if(m_ptObject) m_ptObject->Release();})
CORE_SCOPE_CLASS(coreSurfaceScope,   SDL_Surface,   {},                                      {if(m_ptObject) SDL_DestroySurface(m_ptObject);})
CORE_SCOPE_CLASS(coreAnimationScope, IMG_Animation, {},                                      {if(m_ptObject) IMG_FreeAnimation (m_ptObject);})

template <typename T> CORE_SCOPE_CLASS(coreAllocScope, T, {}, {if(this->m_ptObject) SDL_free(this->m_ptObject);})
template <typename T> CORE_SCOPE_CLASS(coreDataScope,  T, {}, {SAFE_DELETE_ARRAY(this->m_ptObject)})


#endif /* _CORE_GUARD_SCOPE_H_ */