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

// TODO 3: prevent accidental duplicates (operator[], insert(it), emplace(it), etc.)
// TODO 3: insert_once_bs makes two lookups


// ****************************************************************
/* set container class */
template <typename T> class coreSet final : public coreList<T>
{
private:
    /* internal types */
    using coreIterator      = coreList<T>::iterator;
    using coreConstIterator = coreList<T>::const_iterator;

    /* hide undesired functions */
    using coreList<T>::erase_first;
    using coreList<T>::erase_last;
    using coreList<T>::count_first;
    using coreList<T>::count_last;
    using coreList<T>::count_num;
    using coreList<T>::index_first;
    using coreList<T>::index_last;
    using coreList<T>::push_back;
    using coreList<T>::push_front;


public:
    coreSet() = default;

    ENABLE_COPY(coreSet)

    /* insert new unique item */
    using coreList<T>::insert;
    constexpr void     insert        (const T& tItem) {ASSERT(!this->count   (tItem)) {this->push_back(tItem);}}
    constexpr void     insert        (T&&      tItem) {ASSERT(!this->count   (tItem)) {this->push_back(std::move(tItem));}}
    constexpr void     insert_bs     (const T& tItem) {ASSERT(!this->count_bs(tItem)) {const auto it = this->__retrieve_bs(tItem); this->insert(it, tItem);}}
    constexpr void     insert_bs     (T&&      tItem) {ASSERT(!this->count_bs(tItem)) {const auto it = this->__retrieve_bs(tItem); this->insert(it, std::move(tItem));}}
    constexpr coreBool insert_once   (const T& tItem) {    if(!this->count   (tItem)) {this->push_back(tItem);                                                         return true;} return false;}
    constexpr coreBool insert_once   (T&&      tItem) {    if(!this->count   (tItem)) {this->push_back(std::move(tItem));                                              return true;} return false;}
    constexpr coreBool insert_once_bs(const T& tItem) {    if(!this->count_bs(tItem)) {const auto it = this->__retrieve_bs(tItem); this->insert(it, tItem);            return true;} return false;}
    constexpr coreBool insert_once_bs(T&&      tItem) {    if(!this->count_bs(tItem)) {const auto it = this->__retrieve_bs(tItem); this->insert(it, std::move(tItem)); return true;} return false;}

    /* remove existing item */
    using coreList<T>::erase;
    constexpr coreIterator erase   (const T& tItem) {const auto it = this->__retrieve   (tItem); if(this->__check   (it))        return this->erase(it); return this->end();}
    constexpr coreIterator erase_bs(const T& tItem) {const auto it = this->__retrieve_bs(tItem); if(this->__check_bs(it, tItem)) return this->erase(it); return this->end();}

    /* check for existing item */
    constexpr coreBool count   (const T& tItem)const {return this->__check   (this->__retrieve   (tItem));}
    constexpr coreBool count_bs(const T& tItem)const {return this->__check_bs(this->__retrieve_bs(tItem), tItem);}

    /* get internal index */
    using coreList<T>::index;
    constexpr coreUintW index   (const T& tItem)const {return this->index(this->__retrieve   (tItem));}
    constexpr coreUintW index_bs(const T& tItem)const {return this->index(this->__retrieve_bs(tItem));}


private:
    /* check for successful item lookup */
    constexpr coreBool __check   (const coreConstIterator& it)const                 {return (it != this->end());}
    constexpr coreBool __check_bs(const coreConstIterator& it, const T& tItem)const {return (it != this->end()) && ((*it) == tItem);}

    /* lookup item */
    constexpr coreConstIterator __retrieve   (const T& tItem)const {return std::find(this->begin(), this->end(), tItem);}
    constexpr coreConstIterator __retrieve_bs(const T& tItem)const {ASSERT(std::is_sorted(this->begin(), this->end())) return std::lower_bound(this->begin(), this->end(), tItem);}
};


#endif /* _CORE_GUARD_SET_H_ */