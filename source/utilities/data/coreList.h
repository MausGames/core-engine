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

// TODO 3: constexpr if libc++ supports it ? (what about other containers ?)


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

    /* get internal index */
    inline coreUintW index      (const coreIterator&      it)const {return it - this->begin();}
    inline coreUintW index      (const coreConstIterator& it)const {return it - this->begin();}
    inline coreUintW first_index(const T& tItem)const              {return this->index(std::find(this->begin(),  this->end(),  tItem));}
    inline coreUintW last_index (const T& tItem)const              {return this->index(std::find(this->rbegin(), this->rend(), tItem));}

    /* change front item */
    inline void push_front(const T& tItem) {this->insert(this->begin(), tItem);}
    inline void pop_front ()               {ASSERT(!this->empty()) this->erase(this->begin());}

    /* remove existing item */
    coreIterator erase_swap(const coreIterator& it);
};


// ****************************************************************
/* remove existing item without compaction */
template <typename T> typename coreList<T>::coreIterator coreList<T>::erase_swap(const coreIterator& it)
{
    ASSERT(!this->empty())

    // remember current index
    const coreUintW iIndex = this->index(it);

    // swap and delete target item (but do not preserve ordering)
    std::swap(it, this->end() - 1u);
    this->pop_back();

    return this->begin() + iIndex;
}


#endif /* _CORE_GUARD_LIST_H_ */