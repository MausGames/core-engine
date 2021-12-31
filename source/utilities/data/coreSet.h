///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SET_H_
#define _CORE_GUARD_SET_H_


// ****************************************************************
/* set container class */
template <typename T> class coreSet final : public coreList<T>
{
private:
    /* internal types */
    using coreIterator      = typename coreList<T>::iterator;
    using coreConstIterator = typename coreList<T>::const_iterator;


public:
    coreSet() = default;

    ENABLE_COPY(coreSet)

    /* insert new unique item */
    using coreList<T>::insert;
    inline void insert   (const T& tItem) {ASSERT(!this->count   (tItem)) this->push_back(tItem);}
    inline void insert_bs(const T& tItem) {ASSERT(!this->count_bs(tItem)) this->insert(this->__retrieve_bs(tItem), tItem);}

    /* remove existing item */
    using coreList<T>::erase;
    inline coreIterator erase   (const T& tItem) {const auto it = this->__retrieve   (tItem); if(this->__check   (it))        return this->erase(it); return this->end();}
    inline coreIterator erase_bs(const T& tItem) {const auto it = this->__retrieve_bs(tItem); if(this->__check_bs(it, tItem)) return this->erase(it); return this->end();}

    /* check for existing item */
    inline coreBool count   (const T& tItem)const {return this->__check   (this->__retrieve   (tItem));}
    inline coreBool count_bs(const T& tItem)const {return this->__check_bs(this->__retrieve_bs(tItem), tItem);}

    /* get internal index */
    using coreList<T>::index;
    inline coreUintW index   (const T& tItem)const {return this->index(this->__retrieve   (tItem));}
    inline coreUintW index_bs(const T& tItem)const {return this->index(this->__retrieve_bs(tItem));}


private:
    /* check for successful item lookup */
    inline coreBool __check   (const coreConstIterator& it)const                 {return (it != this->end());}
    inline coreBool __check_bs(const coreConstIterator& it, const T& tItem)const {return (it != this->end()) && ((*it) == tItem);}

    /* lookup item */
    inline coreConstIterator __retrieve   (const T& tItem)const {return std::find(this->begin(), this->end(), tItem);}
    inline coreConstIterator __retrieve_bs(const T& tItem)const {ASSERT(std::is_sorted(this->begin(), this->end())) return std::lower_bound(this->begin(), this->end(), tItem);}
};


#endif /* _CORE_GUARD_SET_H_ */