///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LIST_H_
#define _CORE_GUARD_LIST_H_


// ****************************************************************
/* list container class */
template <typename T> class coreList : public std::vector<T>
{
private:
    /* internal types */
    using coreIterator      = typename std::vector<T>::iterator;
    using coreConstIterator = typename std::vector<T>::const_iterator;


public:
    coreList() = default;

    ENABLE_COPY(coreList)

    /* remove existing item */
    coreIterator erase_swap(const coreIterator& it);
};


// ****************************************************************
/* remove existing item without compaction */
template <typename T> typename coreList<T>::coreIterator coreList<T>::erase_swap(const coreIterator& it)
{
    ASSERT(!this->empty())

    // remember current index
    const coreUintW iIndex = it - this->begin();

    // swap and delete target item (but do not preserve ordering)
    std::swap(it, this->end() - 1u);
    this->pop_back();

    return this->begin() + iIndex;
}


#endif /* _CORE_GUARD_LIST_H_ */