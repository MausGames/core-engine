///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LIST_H_
#define _CORE_GUARD_LIST_H_

// TODO 3: for list, set, map, string (though is used in copyable_function), remove implicit copy operator and create explicit copy function, std::copy(this->begin(), this->end(), A.begin());


// ****************************************************************
/* base list container type */
template <typename T> using coreListBase = std::vector<T>;


// ****************************************************************
/* list container class */
template <typename T> class coreList : public coreListBase<T>
{
private:
    /* internal types */
    using coreIterator      = coreListBase<T>::iterator;
    using coreConstIterator = coreListBase<T>::const_iterator;

    /* hide undesired functions */
    using coreListBase<T>::at;


public:
    coreList() = default;

    ENABLE_COPY(coreList)

    /* get internal index */
    constexpr coreUintW index      (const coreIterator&      it)const {return it - this->begin();}
    constexpr coreUintW index      (const coreConstIterator& it)const {return it - this->begin();}
    constexpr coreUintW first_index(const T& tItem)const              {return this->index(std::find(this->begin(),  this->end(),  tItem));}
    constexpr coreUintW last_index (const T& tItem)const              {return this->index(std::find(this->rbegin(), this->rend(), tItem));}

    /* change front item */
    constexpr void push_front(const T& tItem) {this->insert(this->begin(), tItem);}
    constexpr void pop_front ()               {ASSERT(!this->empty()) this->erase(this->begin());}

    /* remove existing item */
    constexpr coreIterator erase_swap(const coreIterator& it);
};


// ****************************************************************
/* remove existing item without compaction */
template <typename T> constexpr coreList<T>::coreIterator coreList<T>::erase_swap(const coreIterator& it)
{
    ASSERT(!this->empty())

    // remember current index
    const coreUintW iIndex = this->index(it);
    ASSERT(iIndex < this->size())

    // overwrite target item with last item (and do not preserve ordering)
    if(iIndex < this->size() - 1u) (*it) = std::move(this->back());
    this->pop_back();

    return this->begin() + iIndex;
}


#endif /* _CORE_GUARD_LIST_H_ */