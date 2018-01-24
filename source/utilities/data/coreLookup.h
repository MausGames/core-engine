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

// TODO: measure performance with high amount of entries (>200), both normal and bs interface
// TODO: radix-tree, crit-bit-tree, splay-tree ?
// TODO: check for detection of inconsistent vector-manipulation (changing value-ordering through iterator, without changing the key-ordering)
// TODO: TryGet function ?


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
    //! @}

    /*! access specific entry */
    //! @{
    T& operator [] (const I& tKey);
    T&          at (const I& tKey);
    const T&    at (const I& tKey)const;
    //! @}

    /*! check number of existing entries */
    //! @{
    inline coreBool  count   (const I& tKey)      {return this->_check(this->_retrieve   (tKey));}
    inline coreBool  count   (const I& tKey)const {return this->_check(this->_retrieve   (tKey));}
    inline coreBool  count_bs(const I& tKey)      {return this->_check(this->_retrieve_bs(tKey));}
    inline coreBool  count_bs(const I& tKey)const {return this->_check(this->_retrieve_bs(tKey));}
    inline coreUintW size    ()const              {return m_atValueList.size ();}
    inline coreBool  empty   ()const              {return m_atValueList.empty();}
    //! @}

    /*! control memory allocation */
    //! @{
    inline void      reserve(const coreUintW iReserve) {this->_cache_clear(); m_atValueList.reserve(iReserve); m_atKeyList.reserve(iReserve);}
    inline void      shrink_to_fit()                   {this->_cache_clear(); m_atValueList.shrink_to_fit();   m_atKeyList.shrink_to_fit();}
    inline coreUintW capacity()const                   {return m_atValueList.capacity();}
    //! @}

    /*! manage container ordering */
    //! @{
    inline void sort_asc  () {this->_sort([](const auto& a, const auto& b) {return (a.first < b.first);});}
    inline void sort_desc () {this->_sort([](const auto& a, const auto& b) {return (a.first > b.first);});}
    inline void reverse   () {this->_cache_clear(); std::reverse(m_atValueList.begin(), m_atValueList.end()); std::reverse(m_atKeyList.begin(), m_atKeyList.end());}
    inline void prepare_bs() {this->sort_asc();}
    //! @}

    /*! create new entry */
    //! @{
    template <typename... A> void emplace(const I& tKey, A&&... vArgs);
    //! @}

    /*! remove existing entry */
    //! @{
    coreBool                 erase    (const I& tKey);
    coreBool                 erase_bs (const I& tKey);
    inline coreValueIterator erase    (const coreValueIterator& it) {this->_cache_clear(); m_atKeyList.erase(this->get_key(it)); return m_atValueList.erase(it);}
    inline void              clear    ()                            {this->_cache_clear(); m_atValueList.clear();    m_atKeyList.clear();}
    inline void              pop_back ()                            {this->_cache_clear(); m_atValueList.pop_back(); m_atKeyList.pop_back();}
    inline void              pop_front()                            {this->_cache_clear(); this->erase(this->begin());}
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
    inline coreValueIterator      begin  ()                   {return m_atValueList.begin();}
    inline coreValueConstIterator begin  ()const              {return m_atValueList.begin();}
    inline coreValueIterator      end    ()                   {return m_atValueList.end  ();}
    inline coreValueConstIterator end    ()const              {return m_atValueList.end  ();}
    inline coreValueIterator      find   (const I& tKey)      {return this->get_value(this->_retrieve   (tKey));}
    inline coreValueConstIterator find   (const I& tKey)const {return this->get_value(this->_retrieve   (tKey));}
    inline coreValueIterator      find_bs(const I& tKey)      {return this->get_value(this->_retrieve_bs(tKey));}
    inline coreValueConstIterator find_bs(const I& tKey)const {return this->get_value(this->_retrieve_bs(tKey));}
    //! @}

    /*! operate between values and keys */
    //! @{
    inline coreValueIterator      get_value    (const coreKeyIterator&        it)      {return m_atValueList.begin() + (it-m_atKeyList  .begin());}
    inline coreValueConstIterator get_value    (const coreKeyConstIterator&   it)const {return m_atValueList.begin() + (it-m_atKeyList  .begin());}
    inline coreKeyIterator        get_key      (const coreValueIterator&      it)      {return m_atKeyList  .begin() + (it-m_atValueList.begin());}
    inline coreKeyConstIterator   get_key      (const coreValueConstIterator& it)const {return m_atKeyList  .begin() + (it-m_atValueList.begin());}
    inline const coreValueList&   get_valuelist()const                                 {return m_atValueList;}
    inline const coreKeyList&     get_keylist  ()const                                 {return m_atKeyList;}
    //! @}


protected:
    /*! check for successful entry lookup */
    //! @{
    inline coreBool _check(const coreKeyIterator&      it)      {return (it != m_atKeyList.end()) ? true : false;}
    inline coreBool _check(const coreKeyConstIterator& it)const {return (it != m_atKeyList.end()) ? true : false;}
    //! @}

    /*! cache last requested entry */
    //! @{
    inline void     _cache_set(T* ptValue, K* ptKey) {m_ptValueCache = ptValue; m_ptKeyCache =  ptKey;}
    inline void     _cache_clear()                   {m_ptValueCache = NULL;    m_ptKeyCache =  NULL;}
    inline coreBool _cache_try(const I& tKey)const   {return (m_ptKeyCache && (*m_ptKeyCache == tKey)) ? true : false;}
    //! @}

    /*! lookup entry by key */
    //! @{
    coreKeyIterator      _retrieve(const I& tKey);
    coreKeyConstIterator _retrieve(const I& tKey)const;
    //! @}

    /*! binary lookup entry by key */
    //! @{
    coreKeyIterator      _retrieve_bs(const I& tKey);
    coreKeyConstIterator _retrieve_bs(const I& tKey)const;
    //! @}

    /*! sort entries with comparison function */
    //! @{
    template <typename F> void _sort(F&& nCompareFunc);   //!< [](const auto& a, const auto& b) -> coreBool
    //! @}
};

// ****************************************************************
/* simplified generic lookup container type */
template <typename K, typename T> using coreLookup = coreLookupGen<K, K, T>;


// ****************************************************************
/* string lookup container class (with original strings) */
template <typename T> class coreLookupStrFull final : public coreLookupGen<coreUint32, coreHashString, T>
{
private:
    coreLookup<coreUint32, std::string> m_asStringList;   //!< list with original strings


public:
    coreLookupStrFull() = default;

    ENABLE_COPY(coreLookupStrFull)

    /*! access specific entry */
    //! @{
    inline T& operator [] (const coreHashString& sKey)   {this->__save_string(sKey); return this->coreLookupGen<coreUint32, coreHashString, T>::operator [] (sKey);}
    inline T& operator [] (const coreUintW       iIndex) {return this->m_atValueList[iIndex];}
    //! @}

    /*! create new entry */
    //! @{
    template <typename... A> inline void emplace(const coreHashString& sKey, A&&... vArgs) {this->__save_string(sKey); this->coreLookupGen<coreUint32, coreHashString, T>::emplace(sKey, std::forward<A>(vArgs)...);}
    //! @}

    /*! remove existing entry */
    //! @{
    using coreLookupGen<coreUint32, coreHashString, T>::erase;
    inline typename coreValueIterator erase(const coreUintW iIndex) {this->_cache_clear(); this->m_atKeyList.erase(this->m_atKeyList.begin()+iIndex); return this->m_atValueList.erase(this->m_atValueList.begin()+iIndex);}
    //! @}

    /*! return original string */
    //! @{
    inline const coreChar* get_string(const typename coreValueIterator&      it)      {return m_asStringList.at(*this->get_key(it)).c_str();}
    inline const coreChar* get_string(const typename coreValueConstIterator& it)const {return m_asStringList.at(*this->get_key(it)).c_str();}
    inline const coreChar* get_string(const typename coreKeyIterator&        it)      {return m_asStringList.at(*it).c_str();}
    inline const coreChar* get_string(const typename coreKeyConstIterator&   it)const {return m_asStringList.at(*it).c_str();}
    //! @}


private:
    /*! save original string */
    //! @{
    inline void __save_string(const coreHashString& sKey) {if(!m_asStringList.count(sKey)) m_asStringList.emplace(sKey, sKey.GetString()); else WARN_IF(sKey.GetString() != m_asStringList.at(sKey)) {}}
    //! @}
};


// ****************************************************************
/* string lookup container class (without original strings) */
template <typename T> class coreLookupStrSlim final : public coreLookupGen<coreUint32, coreHashString, T>
{
public:
    coreLookupStrSlim() = default;

    ENABLE_COPY(coreLookupStrSlim)

    /*! access specific entry */
    //! @{
    using coreLookupGen<coreUint32, coreHashString, T>::operator [];
    inline T& operator [] (const coreUintW iIndex) {return this->m_atValueList[iIndex];}
    //! @}

    /*! remove existing entry */
    //! @{
    using coreLookupGen<coreUint32, coreHashString, T>::erase;
    inline typename coreValueIterator erase(const coreUintW iIndex) {this->_cache_clear(); this->m_atKeyList.erase(this->m_atKeyList.begin()+iIndex); return this->m_atValueList.erase(this->m_atValueList.begin()+iIndex);}
    //! @}
};


// ****************************************************************
/* simplified string lookup container type */
#if defined(_CORE_DEBUG_)
    template <typename T> using coreLookupStr = coreLookupStrFull<T>;
#else
    template <typename T> using coreLookupStr = coreLookupStrSlim<T>;
#endif


// ****************************************************************
/* constructor */
template <typename K, typename I, typename T> coreLookupGen<K, I, T>::coreLookupGen()noexcept
: m_atValueList  {}
, m_atKeyList    {}
, m_ptValueCache (NULL)
, m_ptKeyCache   (NULL)
{
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
    std::swap(m_atValueList,  o.m_atValueList);
    std::swap(m_atKeyList,    o.m_atKeyList);
    std::swap(m_ptValueCache, o.m_ptValueCache);
    std::swap(m_ptKeyCache,   o.m_ptKeyCache);
    return *this;
}


// ****************************************************************
/* access specific entry and create it if necessary */
template <typename K, typename I, typename T> T& coreLookupGen<K, I, T>::operator [] (const I& tKey)
{
    // check for cached entry
    if(this->_cache_try(tKey)) return (*m_ptValueCache);

    // lookup entry by key
    auto it = this->_retrieve(tKey);
    if(!this->_check(it))
    {
        // create new entry
        m_atValueList.emplace_back();
        m_atKeyList  .push_back(tKey);

        // cache current entry
        this->_cache_set(&m_atValueList.back(), &m_atKeyList.back());
        return m_atValueList.back();
    }

    return (*this->get_value(it));
}


// ****************************************************************
/* access specific entry */
template <typename K, typename I, typename T> T& coreLookupGen<K, I, T>::at(const I& tKey)
{
    // check for cached entry
    if(this->_cache_try(tKey)) return (*m_ptValueCache);

    // lookup entry by key
    auto it = this->_retrieve(tKey);
    ASSERT(this->_check(it))

    return (*this->get_value(it));
}

template <typename K, typename I, typename T> const T& coreLookupGen<K, I, T>::at(const I& tKey)const
{
    // check for cached entry
    if(this->_cache_try(tKey)) return (*m_ptValueCache);

    // lookup entry by key
    auto it = this->_retrieve(tKey);
    ASSERT(this->_check(it))

    return (*this->get_value(it));
}


// ****************************************************************
/* create new entry */
template <typename K, typename I, typename T> template <typename... A> void coreLookupGen<K, I, T>::emplace(const I& tKey, A&&... vArgs)
{
    ASSERT(!this->count(tKey))

    // create new entry
    m_atValueList.emplace_back(std::forward<A>(vArgs)...);
    m_atKeyList  .push_back(tKey);

    // cache current entry
    this->_cache_set(&m_atValueList.back(), &m_atKeyList.back());
}


// ****************************************************************
/* remove existing entry */
template <typename K, typename I, typename T> coreBool coreLookupGen<K, I, T>::erase(const I& tKey)
{
    // lookup entry by key
    auto it = this->_retrieve(tKey);
    if(this->_check(it))
    {
        // reset cache
        this->_cache_clear();

        // remove existing entry
        m_atValueList.erase(this->get_value(it));
        m_atKeyList  .erase(it);
        return true;
    }

    return false;
}

template <typename K, typename I, typename T> coreBool coreLookupGen<K, I, T>::erase_bs(const I& tKey)
{
    // binary lookup entry by key
    auto it = this->_retrieve_bs(tKey);
    if(this->_check(it))
    {
        // reset cache
        this->_cache_clear();

        // remove existing entry
        m_atValueList.erase(this->get_value(it));
        m_atKeyList  .erase(it);
        return true;
    }

    return false;
}


// ****************************************************************
/* lookup entry by key */
template <typename K, typename I, typename T> typename coreLookupGen<K, I, T>::coreKeyIterator coreLookupGen<K, I, T>::_retrieve(const I& tKey)
{
    // loop through all entries
    FOR_EACH(it, m_atKeyList)
    {
        // compare key
        if((*it) == tKey)
        {
            // cache current entry
            this->_cache_set(&(*this->get_value(it)), &(*it));
            return it;
        }
    }

    return m_atKeyList.end();
}

template <typename K, typename I, typename T> typename coreLookupGen<K, I, T>::coreKeyConstIterator coreLookupGen<K, I, T>::_retrieve(const I& tKey)const
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


// ****************************************************************
/* binary lookup entry by key */
template <typename K, typename I, typename T> typename coreLookupGen<K, I, T>::coreKeyIterator coreLookupGen<K, I, T>::_retrieve_bs(const I& tKey)
{
    // find entry with binary search
    ASSERT(std::is_sorted(m_atKeyList.begin(), m_atKeyList.end()))
    auto it = std::lower_bound(m_atKeyList.begin(), m_atKeyList.end(), tKey);

    // check result
    if((it != m_atKeyList.end()) && ((*it) == tKey))
    {
        // cache current entry
        this->_cache_set(&(*this->get_value(it)), &(*it));
        return it;
    }

    return m_atKeyList.end();
}

template <typename K, typename I, typename T> typename coreLookupGen<K, I, T>::coreKeyConstIterator coreLookupGen<K, I, T>::_retrieve_bs(const I& tKey)const
{
    // find entry with binary search
    ASSERT(std::is_sorted(m_atKeyList.begin(), m_atKeyList.end()))
    auto it = std::lower_bound(m_atKeyList.begin(), m_atKeyList.end(), tKey);

    // check result
    if((it != m_atKeyList.end()) && ((*it) == tKey))
        return it;

    return m_atKeyList.end();
}


// ****************************************************************
/* sort entries with comparison function */
template <typename K, typename I, typename T> template <typename F> void coreLookupGen<K, I, T>::_sort(F&& nCompareFunc)
{
    std::vector<std::pair<K, T>> aPairList;

    // merge values and keys into single container
    aPairList.reserve(m_atKeyList.size());
    FOR_EACH(it, m_atKeyList)
    {
        aPairList.emplace_back(std::move(*it), std::move(*this->get_value(it)));
    }

    // sort the container
    std::sort(aPairList.begin(), aPairList.end(), nCompareFunc);

    // move entries back into separate lists
    this->clear();
    FOR_EACH(it, aPairList)
    {
        m_atKeyList  .push_back(std::move(it->first));
        m_atValueList.push_back(std::move(it->second));
    }
}


#endif /* _CORE_GUARD_LOOKUP_H_ */