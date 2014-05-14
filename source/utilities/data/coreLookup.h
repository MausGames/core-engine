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


// ****************************************************************
// string lookup container class
// NOTE: much faster than map and u_map with only few items
// TODO: implement quicksort and binary search
// TODO: measure performance with high amount of entries (>200)
// TODO: radix-tree, crit-bit-tree, splay-tree ?
template <typename T> class coreLookup final
{
public:
    //! internal types
    typedef std::pair<std::string, T>         coreEntry;
    typedef std::vector<coreEntry>            coreList;
    typedef typename coreList::iterator       coreIterator;
    typedef typename coreList::const_iterator coreConstIterator;


private:
    coreList m_aList;           //!< vector-list with pair-values

    coreEntry* m_pCache;        //!< last requested entry
    const char* m_pcCacheRef;   //!< string pointer to the last requested entry


public:
    coreLookup()noexcept;
    coreLookup(const coreLookup<T>& c)noexcept;
    coreLookup(coreLookup<T>&& m)noexcept;
    ~coreLookup();

    //! assignment operator
    //! @{
    coreLookup<T>& operator = (coreLookup<T> o)noexcept;
    template <typename S> friend void swap(coreLookup<S>& a, coreLookup<S>& b)noexcept;
    //! @}

    //! access specific entry
    //! @{
    const T& at(const char* pcKey)const noexcept;
    T& operator [] (const char* pcKey)noexcept;
    inline T& operator [] (const coreUint& iIndex)noexcept {return m_aList[iIndex].second;}
    //! @}

    //! check number of existing entries
    //! @{
    inline coreUint count(const char* pcKey)noexcept       {return this->__check(this->__retrieve(pcKey)) ? 1 : 0;}
    inline coreUint count(const char* pcKey)const noexcept {return this->__check(this->__retrieve(pcKey)) ? 1 : 0;} 
    inline coreUint size()const noexcept                   {return m_aList.size();}
    inline bool empty()const noexcept                      {return m_aList.empty();}
    //! @}

    //! control memory consumption
    //! @{
    inline void reserve(const coreUint& iReserve)noexcept {m_aList.reserve(iReserve);}
    inline coreUint capacity()const noexcept              {return m_aList.capacity();}
    //! @}

    //! remove existing entries
    //! @{
    bool erase(const T& Entry)noexcept;
    bool erase(const char* pcKey)noexcept;
    inline coreIterator erase(const coreUint& iIndex)noexcept            {this->__clearcache(); SDL_assert(iIndex < m_aList.size()); return m_aList.erase(m_aList.begin()+iIndex);}
    inline coreIterator erase(const coreConstIterator& Iterator)noexcept {this->__clearcache(); return m_aList.erase(Iterator);}
    inline void clear()noexcept                                          {this->__clearcache(); m_aList.clear();}
    //! @}

    //! retrieve internal iterator
    //! @{
    inline coreIterator begin()noexcept            {return m_aList.begin();}
    inline coreConstIterator begin()const noexcept {return m_aList.begin();}
    inline coreIterator end()noexcept              {return m_aList.end();}
    inline coreConstIterator end()const noexcept   {return m_aList.end();}
    //! @}


private:
    //! retrieve iterator
    //! @{
    coreIterator __retrieve(const T& Entry)noexcept hot_func;
    coreConstIterator __retrieve(const T& Entry)const noexcept hot_func;
    coreIterator __retrieve(const char* pcKey)noexcept hot_func;
    coreConstIterator __retrieve(const char* pcKey)const noexcept hot_func;
    //! @}

    //! check for valid iterator
    //! @{
    inline bool __check(const coreIterator& it)const noexcept      {return (it != m_aList.end()) ? true : false;}
    inline bool __check(const coreConstIterator& it)const noexcept {return (it != m_aList.end()) ? true : false;}
    //! @}

    //! check for cached entry
    //! @{
    inline bool __cache(const char* pcKey)const noexcept {return (m_pcCacheRef == pcKey) ? true : false;}
    inline void __clearcache()noexcept                   {m_pCache = NULL; m_pcCacheRef = NULL;}
    //! @}
};


// ****************************************************************
// generic lookup container class
template <typename K, typename T> class coreLookupGen final
{
public:
    //! internal types
    typedef std::pair<K, T>                   coreEntry;
    typedef std::vector<coreEntry>            coreList;
    typedef typename coreList::iterator       coreIterator;
    typedef typename coreList::const_iterator coreConstIterator;


private:
    coreList m_aList;      //!< vector-list with pair-values

    coreEntry* m_pCache;   //!< last requested entry
    K m_CacheRef;          //!< key to the last requested entry


public:
    coreLookupGen()noexcept;
    coreLookupGen(const coreLookupGen<K, T>& c)noexcept;
    coreLookupGen(coreLookupGen<K, T>&& m)noexcept;
    ~coreLookupGen();

    //! assignment operator
    //! @{
    coreLookupGen<K, T>& operator = (coreLookupGen<K, T> o)noexcept;
    template <typename R, typename S> friend void swap(coreLookupGen<R, S>& a, coreLookupGen<R, S>& b)noexcept;
    //! @}

    //! access specific entry
    //! @{
    const T& at(const K& Key)const noexcept;
    T& operator [] (const K& Key)noexcept;
    //! @}

    //! check number of existing entries
    //! @{
    inline coreUint count(const K& Key)noexcept       {return this->__check(this->__retrieve(Key)) ? 1 : 0;}
    inline coreUint count(const K& Key)const noexcept {return this->__check(this->__retrieve(Key)) ? 1 : 0;}
    inline coreUint size()const noexcept              {return m_aList.size();}
    inline bool empty()const noexcept                 {return m_aList.empty();}
    //! @}

    //! control memory consumption
    //! @{
    inline void reserve(const coreUint& iReserve)noexcept {m_aList.reserve(iReserve);}
    inline coreUint capacity()const noexcept              {return m_aList.capacity();}
    //! @}

    //! remove existing entries
    //! @{
    bool erase(const K& Key)noexcept;
    inline coreIterator erase(const coreConstIterator& Iterator)noexcept {this->__clearcache(); return m_aList.erase(Iterator);}
    inline void clear()noexcept                                          {this->__clearcache(); m_aList.clear();}
    //! @}

    //! retrieve internal iterator
    //! @{
    inline coreIterator begin()noexcept            {return m_aList.begin();}
    inline coreConstIterator begin()const noexcept {return m_aList.begin();}
    inline coreIterator end()noexcept              {return m_aList.end();}
    inline coreConstIterator end()const noexcept   {return m_aList.end();}
    //! @}


private:
    //! retrieve iterator
    //! @{
    coreIterator __retrieve(const K& Key)noexcept hot_func;
    coreConstIterator __retrieve(const K& Key)const noexcept hot_func;
    //! @}

    //! check for valid iterator
    //! @{
    inline bool __check(const coreIterator& it)const noexcept      {return (it != m_aList.end()) ? true : false;}
    inline bool __check(const coreConstIterator& it)const noexcept {return (it != m_aList.end()) ? true : false;}
    //! @}

    //! check for cached entry
    //! @{
    inline bool __cache(const K& Key)const noexcept {return (m_CacheRef == Key) ? true : false;}
    inline void __clearcache()noexcept              {m_pCache = NULL; m_CacheRef = (K)NULL;}
    //! @}
};


// ****************************************************************
// constructor
template <typename T> coreLookup<T>::coreLookup()noexcept
: m_pCache     (NULL)
, m_pcCacheRef (NULL)
{
    // reserve variable sized memory
    constexpr_var coreUint iSize = 1 + 64/sizeof(T);
    m_aList.reserve(iSize);
}

template <typename T> coreLookup<T>::coreLookup(const coreLookup<T>& c)noexcept
: m_aList      (c.m_aList)
, m_pCache     (NULL)
, m_pcCacheRef (NULL)
{
}

template <typename T> coreLookup<T>::coreLookup(coreLookup<T>&& m)noexcept
: m_aList      (std::move(m.m_aList))
, m_pCache     (NULL)
, m_pcCacheRef (NULL)
{
}


// ****************************************************************
// destructor
template <typename T> coreLookup<T>::~coreLookup()
{
    m_aList.clear();
}


// ****************************************************************
// assignment operator
template <typename T> coreLookup<T>& coreLookup<T>::operator = (coreLookup<T> o)noexcept
{
    swap(*this, o);
    return *this;
}

template <typename S> void swap(coreLookup<S>& a, coreLookup<S>& b)noexcept
{
    using std::swap;
    swap(a.m_aList, b.m_aList);
}


// ****************************************************************
// access specific entry
template <typename T> const T& coreLookup<T>::at(const char* pcKey)const noexcept
{
    // check for cached entry
    if(this->__cache(pcKey)) return m_pCache->second;

    // retrieve and check iterator by specific key
    auto it = this->__retrieve(pcKey);
    SDL_assert(this->__check(it));

    return it->second;
}


// ****************************************************************
// access specific entry and create it if necessary
template <typename T> T& coreLookup<T>::operator [] (const char* pcKey)noexcept
{
    // check for cached entry
    if(this->__cache(pcKey)) return m_pCache->second;

    // retrieve and check iterator by specific key
    auto it = this->__retrieve(pcKey);
    if(!this->__check(it))
    {
        // create new entry
        m_aList.push_back(coreEntry(pcKey, T()));
        it = m_aList.end()-1;

        // cache current entry
        m_pCache = &(*it);
    }

    return it->second;
}


// ****************************************************************
// remove existing entry
template <typename T> bool coreLookup<T>::erase(const T& Entry)noexcept
{
    // reset cache
    this->__clearcache();

    // retrieve and check iterator by specific value
    auto it = this->__retrieve(Entry);
    if(this->__check(it))
    {
        // remove existing entry
        m_aList.erase(it);
        return true;
    }

    return false;
}

template <typename T> bool coreLookup<T>::erase(const char* pcKey)noexcept
{
    // reset cache
    this->__clearcache();

    // retrieve and check iterator by specific key
    auto it = this->__retrieve(pcKey);
    if(this->__check(it))
    {
        // remove existing entry
        m_aList.erase(it);
        return true;
    }

    return false;
}


// ****************************************************************
// retrieve iterator by specific value
template <typename T> typename coreLookup<T>::coreIterator coreLookup<T>::__retrieve(const T& Entry)noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare values
        if(it->second == Entry)
            return it;
    }
    return m_aList.end();
}

template <typename T> typename coreLookup<T>::coreConstIterator coreLookup<T>::__retrieve(const T& Entry)const noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare values
        if(it->second == Entry)
            return it;
    }
    return m_aList.end();
}


// ****************************************************************
// retrieve iterator by specific key
template <typename T> typename coreLookup<T>::coreIterator coreLookup<T>::__retrieve(const char* pcKey)noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare string-keys
        if(!std::strcmp(it->first.c_str(), pcKey))
        {
            // cache current entry
            m_pCache     = &(*it);
            m_pcCacheRef = pcKey;
            return it;
        }
    }
    return m_aList.end();
}

template <typename T> typename coreLookup<T>::coreConstIterator coreLookup<T>::__retrieve(const char* pcKey)const noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare string-keys
        if(!std::strcmp(it->first.c_str(), pcKey))
            return it;
    }
    return m_aList.end();
}


// ****************************************************************
// constructor
template <typename K, typename T> coreLookupGen<K, T>::coreLookupGen()noexcept
: m_pCache   (NULL)
, m_CacheRef ((K)NULL)
{
    // reserve variable sized memory
    constexpr_var coreUint iSize = 1 + 64/sizeof(T);
    m_aList.reserve(iSize);
}

template <typename K, typename T> coreLookupGen<K, T>::coreLookupGen(const coreLookupGen<K, T>& c)noexcept
: m_aList    (c.m_aList)
, m_pCache   (NULL)
, m_CacheRef ((K)NULL)
{
}

template <typename K, typename T> coreLookupGen<K, T>::coreLookupGen(coreLookupGen<K, T>&& m)noexcept
: m_aList    (std::move(m.m_aList))
, m_pCache   (NULL)
, m_CacheRef ((K)NULL)
{
}


// ****************************************************************
// destructor
template <typename K, typename T> coreLookupGen<K, T>::~coreLookupGen()
{
    m_aList.clear();
}


// ****************************************************************
// assignment operator
template <typename K, typename T> coreLookupGen<K, T>& coreLookupGen<K, T>::operator = (coreLookupGen<K, T> o)noexcept
{
    swap(*this, o);
    return *this;
}

template <typename R, typename S> void swap(coreLookupGen<R, S>& a, coreLookupGen<R, S>& b)noexcept
{
    using std::swap;
    swap(a.m_aList, b.m_aList);
}


// ****************************************************************
// access specific entry
template <typename K, typename T> const T& coreLookupGen<K, T>::at(const K& Key)const noexcept
{
    // check for cached entry
    if(this->__cache(Key)) return m_pCache->second;

    // retrieve and check iterator by specific key
    auto it = this->__retrieve(Key);
    SDL_assert(this->__check(it));

    return it->second;
}


// ****************************************************************
// access specific entry and create it if necessary
template <typename K, typename T> T& coreLookupGen<K, T>::operator [] (const K& Key)noexcept
{
    // check for cached entry
    if(this->__cache(Key)) return m_pCache->second;

    // retrieve and check iterator by specific key
    auto it = this->__retrieve(Key);
    if(!this->__check(it))
    {
        // create new entry
        m_aList.push_back(coreEntry(Key, T()));
        it = m_aList.end()-1;

        // cache current entry
        m_pCache = &(*it);
    }

    return it->second;
}


// ****************************************************************
// remove existing entry
template <typename K, typename T> bool coreLookupGen<K, T>::erase(const K& Key)noexcept
{
    // reset cache
    this->__clearcache();

    // retrieve and check iterator by specific key
    auto it = this->__retrieve(Key);
    if(this->__check(it))
    {
        // remove existing entry
        m_aList.erase(it);
        return true;
    }

    return false;
}


// ****************************************************************
// retrieve iterator by specific key
template <typename K, typename T> typename coreLookupGen<K, T>::coreIterator coreLookupGen<K, T>::__retrieve(const K& Key)noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare string-keys
        if(it->first == Key)
        {
            // cache current entry
            m_pCache   = &(*it);
            m_CacheRef = Key;
            return it;
        }
    }
    return m_aList.end();
}

template <typename K, typename T> typename coreLookupGen<K, T>::coreConstIterator coreLookupGen<K, T>::__retrieve(const K& Key)const noexcept
{
    // loop through all entries
    FOR_EACH(it, m_aList)
    {
        // compare string-keys
        if(it->first == Key)
            return it;
    }
    return m_aList.end();
}



#endif // _CORE_GUARD_LOOKUP_H_