///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MAP_H_
#define _CORE_GUARD_MAP_H_

// TODO 3: check for detection of inconsistent list-manipulation (changing value-ordering through iterator, without changing the key-ordering)
// TODO 5: TryGet function ?
// TODO 3: implement swap-erase ?
// TODO 3: erase and clear strings in coreMapStrFull ?
// TODO 3: reserve and shrink_to_fit strings in coreMapStrFull ?
// TODO 3: "deducing this" in C++23, to get rid of the const and non-const variants for at and at_bs (maybe somewhere else too)
// TODO 3: change m_asStringList to regular list


// ****************************************************************
/* map container definitions */
#define CORE_MAP_INVALID (coreUintW(-1))   // invalid cache index


// ****************************************************************
/* generic map container class */
template <typename K, typename I, typename T> class coreMapGen
{
protected:
    /* internal types */
    using coreValueList             = coreList<T>;
    using coreKeyList               = coreList<K>;
    using coreValueIterator         = typename coreValueList::iterator;
    using coreValueConstIterator    = typename coreValueList::const_iterator;
    using coreValueRevIterator      = typename coreValueList::reverse_iterator;
    using coreValueConstRevIterator = typename coreValueList::const_reverse_iterator;
    using coreKeyIterator           = typename coreKeyList  ::iterator;
    using coreKeyConstIterator      = typename coreKeyList  ::const_iterator;


protected:
    coreValueList m_atValueList;   // list with values
    coreKeyList   m_atKeyList;     // list with keys

    coreUintW m_iCacheIndex;       // index to the last requested value


public:
    coreMapGen()noexcept;
    coreMapGen(const coreMapGen& c)noexcept;
    coreMapGen(coreMapGen&&      m)noexcept;

    /* assignment operations */
    coreMapGen& operator = (coreMapGen o)noexcept;

    /* access specific entry */
    T& operator []   (const I& tKey);
    T&          bs   (const I& tKey);
    T&          at   (const I& tKey);
    const T&    at   (const I& tKey)const;
    T&          at_bs(const I& tKey);
    const T&    at_bs(const I& tKey)const;

    /* check number of existing entries */
    inline coreBool  count   (const I& tKey)      {return this->_check(this->_retrieve   (tKey));}
    inline coreBool  count   (const I& tKey)const {return this->_check(this->_retrieve   (tKey));}
    inline coreBool  count_bs(const I& tKey)      {return this->_check(this->_retrieve_bs(tKey));}
    inline coreBool  count_bs(const I& tKey)const {return this->_check(this->_retrieve_bs(tKey));}
    inline coreUintW size    ()const              {return m_atValueList.size ();}
    inline coreBool  empty   ()const              {return m_atValueList.empty();}

    /* get internal index */
    inline coreUintW index   (const I& tKey)const                    {if(this->_cache_try(tKey)) return m_iCacheIndex; return m_atKeyList.index(this->_retrieve   (tKey));}
    inline coreUintW index_bs(const I& tKey)const                    {if(this->_cache_try(tKey)) return m_iCacheIndex; return m_atKeyList.index(this->_retrieve_bs(tKey));}
    inline coreUintW index   (const coreValueIterator&      it)const {return m_atValueList.index(it);}
    inline coreUintW index   (const coreValueConstIterator& it)const {return m_atValueList.index(it);}

    /* control memory allocation */
    inline void      reserve      (const coreUintW iCapacity) {m_atValueList.reserve(iCapacity); m_atKeyList.reserve(iCapacity);}
    inline void      shrink_to_fit()                          {m_atValueList.shrink_to_fit();    m_atKeyList.shrink_to_fit();}
    inline coreUintW capacity     ()const                     {return m_atValueList.capacity();}

    /* manage container ordering */
    inline void sort_asc  () {this->_cache_clear(); if(!this->empty()) this->_sort([](const K& a, const K& b) {return (a < b);});}
    inline void sort_desc () {this->_cache_clear(); if(!this->empty()) this->_sort([](const K& a, const K& b) {return (a > b);});}
    inline void reverse   () {this->_cache_clear(); std::reverse(m_atValueList.begin(), m_atValueList.end()); std::reverse(m_atKeyList.begin(), m_atKeyList.end());}
    inline void prepare_bs() {this->sort_asc();}

    /* create new entry */
    template <typename... A> void emplace   (const I& tKey,                              A&&... vArgs);
    template <typename... A> void emplace   (I&&      tKey,                              A&&... vArgs);
    template <typename... A> void emplace_bs(const I& tKey,                              A&&... vArgs);
    template <typename... A> void emplace_bs(I&&      tKey,                              A&&... vArgs);
    template <typename... A> void emplace   (const coreValueIterator& it, const I& tKey, A&&... vArgs);
    template <typename... A> void emplace   (const coreValueIterator& it, I&&      tKey, A&&... vArgs);

    /* remove existing entry */
    coreBool                 erase    (const I& tKey);
    coreBool                 erase_bs (const I& tKey);
    inline coreValueIterator erase    (const coreValueIterator& it) {this->_cache_clear(); m_atKeyList.erase(this->get_key(it)); return m_atValueList.erase(it);}
    inline void              clear    ()                            {this->_cache_clear(); m_atValueList.clear();    m_atKeyList.clear();}
    inline void              pop_back ()                            {this->_cache_clear(); m_atValueList.pop_back(); m_atKeyList.pop_back();}
    inline void              pop_front()                            {this->erase(this->begin());}

    /* return first and last entry */
    inline T&       front()      {return m_atValueList.front();}
    inline const T& front()const {return m_atValueList.front();}
    inline T&       back ()      {return m_atValueList.back ();}
    inline const T& back ()const {return m_atValueList.back ();}

    /* return internal iterator */
    inline coreValueIterator         begin  ()                   {return m_atValueList.begin ();}
    inline coreValueConstIterator    begin  ()const              {return m_atValueList.begin ();}
    inline coreValueIterator         end    ()                   {return m_atValueList.end   ();}
    inline coreValueConstIterator    end    ()const              {return m_atValueList.end   ();}
    inline coreValueRevIterator      rbegin ()                   {return m_atValueList.rbegin();}
    inline coreValueConstRevIterator rbegin ()const              {return m_atValueList.rbegin();}
    inline coreValueRevIterator      rend   ()                   {return m_atValueList.rend  ();}
    inline coreValueConstRevIterator rend   ()const              {return m_atValueList.rend  ();}
    inline coreValueIterator         find   (const I& tKey)      {return this->get_value(this->_retrieve   (tKey));}
    inline coreValueConstIterator    find   (const I& tKey)const {return this->get_value(this->_retrieve   (tKey));}
    inline coreValueIterator         find_bs(const I& tKey)      {return this->get_value(this->_retrieve_bs(tKey));}
    inline coreValueConstIterator    find_bs(const I& tKey)const {return this->get_value(this->_retrieve_bs(tKey));}

    /* operate between values and keys */
    inline coreValueIterator      get_value    (const coreKeyIterator&        it)      {return m_atValueList.begin() + m_atKeyList  .index(it);}
    inline coreValueConstIterator get_value    (const coreKeyConstIterator&   it)const {return m_atValueList.begin() + m_atKeyList  .index(it);}
    inline coreKeyIterator        get_key      (const coreValueIterator&      it)      {return m_atKeyList  .begin() + m_atValueList.index(it);}
    inline coreKeyConstIterator   get_key      (const coreValueConstIterator& it)const {return m_atKeyList  .begin() + m_atValueList.index(it);}
    inline const coreValueList&   get_valuelist()const                                 {return m_atValueList;}
    inline const coreKeyList&     get_keylist  ()const                                 {return m_atKeyList;}


protected:
    /* check for successful entry lookup */
    inline coreBool _check(const coreKeyIterator&      it)      {return (it != m_atKeyList.end());}
    inline coreBool _check(const coreKeyConstIterator& it)const {return (it != m_atKeyList.end());}

    /* cache last requested entry */
    inline void     _cache_set  (const coreUintW iIndex) {m_iCacheIndex = iIndex;}
    inline void     _cache_clear()                       {m_iCacheIndex = CORE_MAP_INVALID;}
    inline coreBool _cache_try  (const I& tKey)const     {return ((m_iCacheIndex != CORE_MAP_INVALID) && (m_atKeyList[m_iCacheIndex] == tKey));}

    /* lookup entry by key */
    coreKeyIterator      _retrieve(const I& tKey);
    coreKeyConstIterator _retrieve(const I& tKey)const;

    /* binary lookup entry by key */
    coreKeyIterator      _retrieve_bs(const I& tKey);
    coreKeyConstIterator _retrieve_bs(const I& tKey)const;

    /* sort entries with comparison function */
    template <typename F> void _sort(F&& nCompareFunc);   // [](const K& a, const K& b) -> coreBool
    template <typename F> void _sort(F&& nCompareFunc, const coreUintW a, const coreUintW b);
};


// ****************************************************************
/* simplified generic map container type */
template <typename K, typename T> using coreMap = coreMapGen<K, K, T>;


// ****************************************************************
/* base string map container type */
template <typename T> using coreMapStrBase = coreMapGen<coreUint32, coreHashString, T>;


// ****************************************************************
/* string map container class (with original strings) */
template <typename T> class coreMapStrFull final : public coreMapStrBase<T>
{
private:
    coreMap<coreUint32, coreString> m_asStringList;   // list with original strings


public:
    coreMapStrFull() = default;

    ENABLE_COPY(coreMapStrFull)

    /* access specific entry */
    inline       T& operator [] (const coreUintW       iIndex)      {return this->m_atValueList[iIndex];}
    inline const T& operator [] (const coreUintW       iIndex)const {return this->m_atValueList[iIndex];}
    inline       T& operator [] (const coreHashString& sKey)        {this->__save_string(sKey); return this->coreMapStrBase<T>::operator [] (sKey);}
    inline       T&          bs (const coreHashString& sKey)        {this->__save_string(sKey); return this->coreMapStrBase<T>::         bs (sKey);}

    /* control memory allocation */
    inline void reserve      (const coreUintW iCapacity) {m_asStringList.reserve(iCapacity); this->coreMapStrBase<T>::reserve(iCapacity);}
    inline void shrink_to_fit()                          {m_asStringList.shrink_to_fit();    this->coreMapStrBase<T>::shrink_to_fit();}

    /* create new entry */
    template <typename... A> inline void emplace   (const coreHashString& sKey,                                                          A&&... vArgs) {this->__save_string(sKey); this->coreMapStrBase<T>::emplace   (sKey,     std::forward<A>(vArgs)...);}
    template <typename... A> inline void emplace_bs(const coreHashString& sKey,                                                          A&&... vArgs) {this->__save_string(sKey); this->coreMapStrBase<T>::emplace_bs(sKey,     std::forward<A>(vArgs)...);}
    template <typename... A> inline void emplace   (const typename coreMapStrBase<T>::coreValueIterator& it, const coreHashString& sKey, A&&... vArgs) {this->__save_string(sKey); this->coreMapStrBase<T>::emplace   (it, sKey, std::forward<A>(vArgs)...);}

    /* remove existing entry */
    using coreMapStrBase<T>::erase;
    inline typename coreMapStrBase<T>::coreValueIterator erase(const coreUintW iIndex) {this->_cache_clear(); this->m_atKeyList.erase(this->m_atKeyList.begin() + iIndex); return this->m_atValueList.erase(this->m_atValueList.begin() + iIndex);}
    inline void                                          clear()                       {m_asStringList.clear(); this->coreMapStrBase<T>::clear();}

    /* return original string */
    inline const coreChar* get_string(const typename coreMapStrBase<T>::coreValueIterator&      it)      {return m_asStringList.at_bs(*this->get_key(it)).c_str();}
    inline const coreChar* get_string(const typename coreMapStrBase<T>::coreValueConstIterator& it)const {return m_asStringList.at_bs(*this->get_key(it)).c_str();}


private:
    /* save original string */
    inline void __save_string(const coreHashString& sKey) {if(!m_asStringList.count_bs(sKey)) m_asStringList.emplace_bs(sKey, sKey.GetString()); ASSERT(m_asStringList.at_bs(sKey) == sKey.GetString())}
};


// ****************************************************************
/* string map container class (without original strings) */
template <typename T> class coreMapStrSlim final : public coreMapStrBase<T>
{
public:
    coreMapStrSlim() = default;

    ENABLE_COPY(coreMapStrSlim)

    /* access specific entry */
    using coreMapStrBase<T>::operator [];
    inline       T& operator [] (const coreUintW iIndex)      {return this->m_atValueList[iIndex];}
    inline const T& operator [] (const coreUintW iIndex)const {return this->m_atValueList[iIndex];}

    /* remove existing entry */
    using coreMapStrBase<T>::erase;
    inline typename coreMapStrBase<T>::coreValueIterator erase(const coreUintW iIndex) {this->_cache_clear(); this->m_atKeyList.erase(this->m_atKeyList.begin() + iIndex); return this->m_atValueList.erase(this->m_atValueList.begin() + iIndex);}
};


// ****************************************************************
/* simplified string map container type */
#if defined(_CORE_DEBUG_)
    template <typename T> using coreMapStr = coreMapStrFull<T>;
#else
    template <typename T> using coreMapStr = coreMapStrSlim<T>;
#endif


// ****************************************************************
/* constructor */
template <typename K, typename I, typename T> coreMapGen<K, I, T>::coreMapGen()noexcept
: m_atValueList {}
, m_atKeyList   {}
, m_iCacheIndex (CORE_MAP_INVALID)
{
}

template <typename K, typename I, typename T> coreMapGen<K, I, T>::coreMapGen(const coreMapGen& c)noexcept
: m_atValueList (c.m_atValueList)
, m_atKeyList   (c.m_atKeyList)
, m_iCacheIndex (c.m_iCacheIndex)
{
}

template <typename K, typename I, typename T> coreMapGen<K, I, T>::coreMapGen(coreMapGen&& m)noexcept
: m_atValueList (std::move(m.m_atValueList))
, m_atKeyList   (std::move(m.m_atKeyList))
, m_iCacheIndex (m.m_iCacheIndex)
{
}


// ****************************************************************
/* assignment operations */
template <typename K, typename I, typename T> coreMapGen<K, I, T>& coreMapGen<K, I, T>::operator = (coreMapGen o)noexcept
{
    // swap properties
    std::swap(m_atValueList, o.m_atValueList);
    std::swap(m_atKeyList,   o.m_atKeyList);
    std::swap(m_iCacheIndex, o.m_iCacheIndex);

    return *this;
}


// ****************************************************************
/* access specific entry and create it if necessary */
template <typename K, typename I, typename T> T& coreMapGen<K, I, T>::operator [] (const I& tKey)
{
    // check for cached entry
    if(this->_cache_try(tKey)) return m_atValueList[m_iCacheIndex];

    // lookup entry by key
    const auto it = this->_retrieve(tKey);
    if(!this->_check(it))
    {
        // create new entry
        this->emplace(tKey);
        return m_atValueList.back();
    }

    return (*this->get_value(it));
}

template <typename K, typename I, typename T> T& coreMapGen<K, I, T>::bs(const I& tKey)
{
    // check for cached entry
    if(this->_cache_try(tKey)) return m_atValueList[m_iCacheIndex];

    // binary lookup entry by key
    const auto it = this->_retrieve_bs(tKey);
    if(!this->_check(it))
    {
        // create new entry
        this->emplace_bs(tKey);
        return m_atValueList[m_iCacheIndex];
    }

    return (*this->get_value(it));
}


// ****************************************************************
/* access specific entry */
template <typename K, typename I, typename T> T& coreMapGen<K, I, T>::at(const I& tKey)
{
    // check for cached entry
    if(this->_cache_try(tKey)) return m_atValueList[m_iCacheIndex];

    // lookup entry by key
    const auto it = this->_retrieve(tKey);
    ASSERT(this->_check(it))

    return (*this->get_value(it));
}

template <typename K, typename I, typename T> const T& coreMapGen<K, I, T>::at(const I& tKey)const
{
    // check for cached entry
    if(this->_cache_try(tKey)) return m_atValueList[m_iCacheIndex];

    // lookup entry by key
    const auto it = this->_retrieve(tKey);
    ASSERT(this->_check(it))

    return (*this->get_value(it));
}

template <typename K, typename I, typename T> T& coreMapGen<K, I, T>::at_bs(const I& tKey)
{
    // check for cached entry
    if(this->_cache_try(tKey)) return m_atValueList[m_iCacheIndex];

    // binary lookup entry by key
    const auto it = this->_retrieve_bs(tKey);
    ASSERT(this->_check(it))

    return (*this->get_value(it));
}

template <typename K, typename I, typename T> const T& coreMapGen<K, I, T>::at_bs(const I& tKey)const
{
    // check for cached entry
    if(this->_cache_try(tKey)) return m_atValueList[m_iCacheIndex];

    // binary lookup entry by key
    const auto it = this->_retrieve_bs(tKey);
    ASSERT(this->_check(it))

    return (*this->get_value(it));
}


// ****************************************************************
/* create new entry */
template <typename K, typename I, typename T> template <typename... A> void coreMapGen<K, I, T>::emplace(const I& tKey, A&&... vArgs)
{
    // copy and move key
    this->emplace(std::move(I(tKey)), std::forward<A>(vArgs)...);
}

template <typename K, typename I, typename T> template <typename... A> void coreMapGen<K, I, T>::emplace(I&& tKey, A&&... vArgs)
{
    ASSERT(!this->count(tKey))

    // create new entry
    m_atValueList.emplace_back(std::forward<A>(vArgs)...);
    m_atKeyList  .push_back(std::move(tKey));

    // cache current entry
    this->_cache_set(m_atValueList.size() - 1u);
}

template <typename K, typename I, typename T> template <typename... A> void coreMapGen<K, I, T>::emplace_bs(const I& tKey, A&&... vArgs)
{
    // copy and move key
    this->emplace_bs(std::move(I(tKey)), std::forward<A>(vArgs)...);
}

template <typename K, typename I, typename T> template <typename... A> void coreMapGen<K, I, T>::emplace_bs(I&& tKey, A&&... vArgs)
{
    ASSERT(!this->count_bs(tKey))

    // find target location with binary search
    auto it = std::lower_bound(m_atKeyList.begin(), m_atKeyList.end(), tKey);

    // create new entry
    m_atValueList.emplace(this->get_value(it), std::forward<A>(vArgs)...);
    it = m_atKeyList.insert(it, std::move(tKey));

    // cache current entry
    this->_cache_set(m_atKeyList.index(it));
}

template <typename K, typename I, typename T> template <typename... A> void coreMapGen<K, I, T>::emplace(const coreValueIterator& it, const I& tKey, A&&... vArgs)
{
    // copy and move key
    return this->emplace(it, std::move(I(tKey)), std::forward<A>(vArgs)...);
}

template <typename K, typename I, typename T> template <typename... A> void coreMapGen<K, I, T>::emplace(const coreValueIterator& it, I&& tKey, A&&... vArgs)
{
    ASSERT(!this->count(tKey))

    // create new entry
    const auto et = m_atKeyList.insert(this->get_key(it), std::move(tKey));
    m_atValueList.emplace(it, std::forward<A>(vArgs)...);

    // cache current entry
    this->_cache_set(m_atKeyList.index(et));
}


// ****************************************************************
/* remove existing entry */
template <typename K, typename I, typename T> coreBool coreMapGen<K, I, T>::erase(const I& tKey)
{
    // lookup entry by key
    const auto it = this->_retrieve(tKey);
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

template <typename K, typename I, typename T> coreBool coreMapGen<K, I, T>::erase_bs(const I& tKey)
{
    // binary lookup entry by key
    const auto it = this->_retrieve_bs(tKey);
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
template <typename K, typename I, typename T> typename coreMapGen<K, I, T>::coreKeyIterator coreMapGen<K, I, T>::_retrieve(const I& tKey)
{
    // loop through all entries
    FOR_EACH(it, m_atKeyList)
    {
        // compare key
        if((*it) == tKey)
        {
            // cache current entry
            this->_cache_set(m_atKeyList.index(it));
            return it;
        }
    }

    return m_atKeyList.end();
}

template <typename K, typename I, typename T> typename coreMapGen<K, I, T>::coreKeyConstIterator coreMapGen<K, I, T>::_retrieve(const I& tKey)const
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
template <typename K, typename I, typename T> typename coreMapGen<K, I, T>::coreKeyIterator coreMapGen<K, I, T>::_retrieve_bs(const I& tKey)
{
    // find entry with binary search
    ASSERT(std::is_sorted(m_atKeyList.begin(), m_atKeyList.end()))
    const auto it = std::lower_bound(m_atKeyList.begin(), m_atKeyList.end(), tKey);

    // check result
    if((it != m_atKeyList.end()) && ((*it) == tKey))
    {
        // cache current entry
        this->_cache_set(m_atKeyList.index(it));
        return it;
    }

    return m_atKeyList.end();
}

template <typename K, typename I, typename T> typename coreMapGen<K, I, T>::coreKeyConstIterator coreMapGen<K, I, T>::_retrieve_bs(const I& tKey)const
{
    // find entry with binary search
    ASSERT(std::is_sorted(m_atKeyList.begin(), m_atKeyList.end()))
    const auto it = std::lower_bound(m_atKeyList.begin(), m_atKeyList.end(), tKey);

    // check result
    if((it != m_atKeyList.end()) && ((*it) == tKey))
        return it;

    return m_atKeyList.end();
}


// ****************************************************************
/* sort entries with comparison function */
template <typename K, typename I, typename T> template <typename F> void coreMapGen<K, I, T>::_sort(F&& nCompareFunc)
{
    ASSERT(!this->empty())
    this->_sort(std::forward<F>(nCompareFunc), 0u, m_atKeyList.size() - 1u);
}

template <typename K, typename I, typename T> template <typename F> void coreMapGen<K, I, T>::_sort(F&& nCompareFunc, const coreUintW a, const coreUintW b)
{
    // perform quicksort with Hoare partition scheme
    if(a < b)
    {
        const K tPivot = m_atKeyList[(a + b) / 2u];

        coreUintW i = a - 1u;
        coreUintW j = b + 1u;

        while(true)
        {
            do
            {
                i += 1u;
            }
            while(nCompareFunc(m_atKeyList[i], tPivot));

            do
            {
                j -= 1u;
            }
            while(nCompareFunc(tPivot, m_atKeyList[j]));

            if(i >= j) break;

            std::swap(m_atValueList[i], m_atValueList[j]);
            std::swap(m_atKeyList  [i], m_atKeyList  [j]);
        }

        this->_sort(nCompareFunc, a,      j);
        this->_sort(nCompareFunc, j + 1u, b);
    }
}


#endif /* _CORE_GUARD_MAP_H_ */