//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LOOKUP_H_
#define _CORE_GUARD_LOOKUP_H_

// TODO: implement quicksort and binary search (std::lower_bound)
// TODO: measure performance with high amount of entries (>200), but should not be as good as (unordered_)map
// TODO: radix-tree, crit-bit-tree, splay-tree ?


// ****************************************************************
/* generic lookup container class */
template <typename K, typename I, typename T> class coreLookupGen
{
protected:
    /*! internal types */
    using coreValueList          = std::vector<T>;
    using coreKeyList            = std::vector<K>;
    using coreValueIterator      = typename coreValueList::iterator;
    using coreValueConstIterator = typename coreValueList::const_iterator;
    using coreKeyIterator        = typename coreKeyList  ::iterator;
    using coreKeyConstIterator   = typename coreKeyList  ::const_iterator;


protected:
    coreValueList m_atValueList;   //!< list with values
    coreKeyList   m_atKeyList;     //!< list with keys

    T* m_ptValueCache;             //!< last requested value
    K* m_ptKeyCache;               //!< key to the last requested value


public:
    coreLookupGen()noexcept;
    coreLookupGen(const coreLookupGen<K, I, T>& c)noexcept;
    coreLookupGen(coreLookupGen<K, I, T>&&      m)noexcept;

    /*! assignment operations */
    //! @{
    coreLookupGen<K, I, T>& operator = (coreLookupGen<K, I, T> o)noexcept;
    template <typename R, typename O, typename S> friend void swap(coreLookupGen<R, O, S>& a, coreLookupGen<R, O, S>& b)noexcept;
    //! @}

    /*! access specific entry */
    //! @{
    const T&    at (const I& tKey)const;
    T& operator [] (const I& tKey);
    //! @}

    /*! check number of existing entries */
    //! @{
    inline coreBool  count(const I& tKey)      {return this->__check(this->__retrieve(tKey));}
    inline coreBool  count(const I& tKey)const {return this->__check(this->__retrieve(tKey));}
    inline coreUintW size ()const              {return m_atValueList.size ();}
    inline coreBool  empty()const              {return m_atValueList.empty();}
    //! @}

    /*! control memory allocation */
    //! @{
    inline void      reserve(const coreUintW& iReserve) {this->__cache_clear(); m_atValueList.reserve(iReserve); m_atKeyList.reserve(iReserve);}
    inline void      shrink_to_fit()                    {this->__cache_clear(); m_atValueList.shrink_to_fit();   m_atKeyList.shrink_to_fit();}
    inline coreUintW capacity()const                    {return m_atValueList.capacity();}
    //! @}

    /*! remove existing entries */
    //! @{
    coreBool                 erase(const I& tKey);
    inline coreValueIterator erase(const coreValueConstIterator& it) {this->__cache_clear(); m_atKeyList.erase(this->get_key(it)); return m_atValueList.erase(it);}
    inline void              clear()                                 {this->__cache_clear(); m_atValueList.clear(); m_atKeyList.clear();}
    //! @}

    /*! return first and last entry */
    //! @{
    inline T&       front()      {return m_atValueList.front();}
    inline const T& front()const {return m_atValueList.front();}
    inline T&       back ()      {return m_atValueList.back ();}
    inline const T& back ()const {return m_atValueList.back ();}
    //! @}

    /*! return internal iterator */
    //! @{
    inline coreValueIterator      begin()      {return m_atValueList.begin();}
    inline coreValueConstIterator begin()const {return m_atValueList.begin();}
    inline coreValueIterator      end  ()      {return m_atValueList.end  ();}
    inline coreValueConstIterator end  ()const {return m_atValueList.end  ();}
    //! @}

    /*! operate between values and keys */
    //! @{
    inline coreValueIterator      get_value  (const coreKeyIterator&        it)      {return m_atValueList.begin() + (it-m_atKeyList  .begin());}
    inline coreValueConstIterator get_value  (const coreKeyConstIterator&   it)const {return m_atValueList.begin() + (it-m_atKeyList  .begin());}
    inline coreKeyIterator        get_key    (const coreValueIterator&      it)      {return m_atKeyList  .begin() + (it-m_atValueList.begin());}
    inline coreKeyConstIterator   get_key    (const coreValueConstIterator& it)const {return m_atKeyList  .begin() + (it-m_atValueList.begin());}
    inline const coreKeyList&     get_keylist()const                                 {return m_atKeyList;}
    //! @}


protected:
    /*! check for successful entry lookup */
    //! @{
    inline coreBool __check(const coreKeyIterator&      it)      {return (it != m_atKeyList.end()) ? true : false;}
    inline coreBool __check(const coreKeyConstIterator& it)const {return (it != m_atKeyList.end()) ? true : false;}
    //! @}

    /*! cache last requested entry */
    //! @{
    inline void     __cache_set(T* ptValue, K* ptKey) {m_ptValueCache = ptValue; m_ptKeyCache =  ptKey;}
    inline void     __cache_clear()                   {m_ptValueCache = NULL;    m_ptKeyCache =  NULL;}
    inline coreBool __cache_try(const I& tKey)const   {return (m_ptKeyCache && (*m_ptKeyCache == tKey)) ? true : false;}
    //! @}

    /*! lookup entry by key */
    //! @{
    coreKeyIterator      __retrieve(const I& tKey);
    coreKeyConstIterator __retrieve(const I& tKey)const;
    //! @}
};


// ****************************************************************
/* simplified lookup container type */
template <typename K, typename T> using coreLookup = coreLookupGen<K, K, T>;


// ****************************************************************
/* string lookup container class */
template <typename T> class coreLookupStr final : public coreLookupGen<std::string, const coreChar*, T>
{
public:
    coreLookupStr()noexcept {}

    /*! access specific entry */
    //! @{
    using coreLookupGen<std::string, const coreChar*, T>::operator [];
    inline T& operator [] (const coreUintW& iIndex) {return this->m_atValueList[iIndex];}
    //! @}

    /*! remove existing entries */
    //! @{
    using coreLookupGen<std::string, const coreChar*, T>::erase;
    inline typename coreLookupStr<T>::coreValueIterator erase(const coreUintW& iIndex) {this->__cache_clear(); this->m_atKeyList.erase(this->m_atKeyList.begin()+iIndex); return this->m_atValueList.erase(this->m_atValueList.begin()+iIndex);}
    //! @}
};


// ****************************************************************
/* constructor */
template <typename K, typename I, typename T> coreLookupGen<K, I, T>::coreLookupGen()noexcept
: m_ptValueCache (NULL)
, m_ptKeyCache   (NULL)
{
    constexpr_var coreUintW iSize = MAX(64u / sizeof(T), 2u);

    // pre-allocate some memory
    m_atValueList.reserve(iSize);
    m_atKeyList  .reserve(iSize);
}

template <typename K, typename I, typename T> coreLookupGen<K, I, T>::coreLookupGen(const coreLookupGen<K, I, T>& c)noexcept
: m_atValueList  (c.m_atValueList)
, m_atKeyList    (c.m_atKeyList)
, m_ptValueCache (NULL)
, m_ptKeyCache   (NULL)
{
}

template <typename K, typename I, typename T> coreLookupGen<K, I, T>::coreLookupGen(coreLookupGen<K, I, T>&& m)noexcept
: m_atValueList  (std::move(m.m_atValueList))
, m_atKeyList    (std::move(m.m_atKeyList))
, m_ptValueCache (NULL)
, m_ptKeyCache   (NULL)
{
}


// ****************************************************************
/* assignment operations */
template <typename K, typename I, typename T> coreLookupGen<K, I, T>& coreLookupGen<K, I, T>::operator = (coreLookupGen<K, I, T> o)noexcept
{
    std::swap(*this, o);
    return *this;
}

template <typename R, typename O, typename S> void swap(coreLookupGen<R, O, S>& a, coreLookupGen<R, O, S>& b)noexcept
{
    std::swap(a.m_atValueList,  b.m_atValueList);
    std::swap(a.m_atKeyList,    b.m_atKeyList);
    std::swap(a.m_ptValueCache, b.m_ptValueCache);
    std::swap(a.m_ptKeyCache,   b.m_ptKeyCache);
}


// ****************************************************************
/* swap specialization */
namespace std {template<typename R, typename O, typename S> inline void swap(coreLookupGen<R, O, S>& a, coreLookupGen<R, O, S>& b) {::swap(a, b);}}


// ****************************************************************
/* access specific entry */
template <typename K, typename I, typename T> const T& coreLookupGen<K, I, T>::at(const I& tKey)const
{
    // check for cached entry
    if(this->__cache_try(tKey)) return *m_ptValueCache;

    // lookup entry by key
    auto it = this->__retrieve(tKey);
    ASSERT(this->__check(it))

    return *this->get_value(it);
}


// ****************************************************************
/* access specific entry and create it if necessary */
template <typename K, typename I, typename T> T& coreLookupGen<K, I, T>::operator [] (const I& tKey)
{
    // check for cached entry
    if(this->__cache_try(tKey)) return *m_ptValueCache;

    // lookup entry by key
    auto it = this->__retrieve(tKey);
    if(!this->__check(it))
    {
        // create new entry
        m_atValueList.push_back(T());
        m_atKeyList  .push_back(tKey);

        // cache current entry
        this->__cache_set(&m_atValueList.back(), &m_atKeyList.back());
        return m_atValueList.back();
    }

    return *this->get_value(it);
}


// ****************************************************************
/* remove existing entry */
template <typename K, typename I, typename T> coreBool coreLookupGen<K, I, T>::erase(const I& tKey)
{
    // lookup entry by key
    auto it = this->__retrieve(tKey);
    if(this->__check(it))
    {
        // reset cache
        this->__cache_clear();

        // remove existing entry
        m_atValueList.erase(this->get_value(it));
        m_atKeyList  .erase(it);
        return true;
    }

    return false;
}


// ****************************************************************
/* lookup entry by key */
template <typename K, typename I, typename T> typename coreLookupGen<K, I, T>::coreKeyIterator coreLookupGen<K, I, T>::__retrieve(const I& tKey)
{
    // loop through all entries
    FOR_EACH(it, m_atKeyList)
    {
        // compare key
        if((*it) == tKey)
        {
            // cache current entry
            this->__cache_set(&(*this->get_value(it)), &(*it));
            return it;
        }
    }

    return m_atKeyList.end();
}

template <typename K, typename I, typename T> typename coreLookupGen<K, I, T>::coreKeyConstIterator coreLookupGen<K, I, T>::__retrieve(const I& tKey)const
{
    // loop through all entries
    FOR_EACH(it, m_atKeyList)
    {
        // compare key
        if((*it) == tKey)
            return it;
    }

    return m_atKeyList.end();
}


#endif /* _CORE_GUARD_LOOKUP_H_ */