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
// TODO 4: rename count to contains/exists/other, for list, set and map
// TODO 3: check on T& and co. parameters for list, set, map, switch-box
// TODO 4: cbegin, cend, crbegin, crend
// TODO 4: the coreIterator<>coreConstIterator split for certain functions seems unnecessary and just the const version would be enough (also in other containers)


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

    /* insert new item */
    template <typename... A> constexpr T&           emplace_back_unsafe(A&&... vArgs);
    template <typename... A> constexpr coreIterator emplace_unsafe     (const coreConstIterator& it, A&&... vArgs);
    constexpr void                                  push_back_unsafe   (const T& tItem)                              {this->emplace_back_unsafe(tItem);}
    constexpr void                                  push_back_unsafe   (T&&      tItem)                              {this->emplace_back_unsafe(std::move(tItem));}
    constexpr coreIterator                          insert_unsafe      (const coreConstIterator& it, const T& tItem) {return this->emplace_unsafe(it, tItem);}
    constexpr coreIterator                          insert_unsafe      (const coreConstIterator& it, T&&      tItem) {return this->emplace_unsafe(it, std::move(tItem));}

    /* remove existing item */
    constexpr coreIterator erase_first(const T& tItem)         {return this->erase(this->__retrieve_first(tItem));}
    constexpr coreIterator erase_last (const T& tItem)         {return this->erase(this->__retrieve_last (tItem));}
    constexpr coreIterator erase_index(const coreUintW iIndex) {ASSERT(iIndex < this->size()) return this->erase(this->begin() + iIndex);}
    constexpr coreIterator erase_swap (const coreIterator& it);

    /* check for existing item */
    constexpr coreBool  count_first(const T& tItem)const {return (this->__retrieve_first(tItem) != this->end());}
    constexpr coreBool  count_last (const T& tItem)const {return (this->__retrieve_last (tItem) != this->end());}
    constexpr coreUintW count_num  (const T& tItem)const {return std::count(this->begin(), this->end(), tItem);}

    /* get internal index */
    constexpr coreUintW index      (const coreIterator&      it)const {return (it - this->begin());}
    constexpr coreUintW index      (const coreConstIterator& it)const {return (it - this->begin());}
    constexpr coreUintW index_first(const T& tItem)const              {return this->index(this->__retrieve_first(tItem));}
    constexpr coreUintW index_last (const T& tItem)const              {return this->index(this->__retrieve_last (tItem));}

    /* change front item */
    constexpr void push_front(const T& tItem) {this->insert(this->begin(), tItem);}
    constexpr void push_front(T&&      tItem) {this->insert(this->begin(), std::move(tItem));}
    constexpr void pop_front ()               {ASSERT(!this->empty()) this->erase(this->begin());}


private:
    /* lookup item */
    constexpr coreConstIterator __retrieve_first(const T& tItem)const {return std::find(this->begin(),  this->end(),  tItem);}
    constexpr coreConstIterator __retrieve_last (const T& tItem)const {return std::find(this->rbegin(), this->rend(), tItem).base();}
};


// ****************************************************************
/* insert new item (without reallocation check) */
template <typename T> template <typename... A> constexpr T& coreList<T>::emplace_back_unsafe(A&&... vArgs)
{
    ASSERT(this->size() < this->capacity())

    // access raw container pointers
    T** pptRaw = r_cast<T**>(this);
    ASSERT(pptRaw[0] == std::to_address(this->begin()))
    ASSERT(pptRaw[1] == std::to_address(this->end  ()))

    // manually construct object in place
    CALL_CONSTRUCTOR(pptRaw[1], std::forward<A>(vArgs)...);
    pptRaw[1] += 1u;

    return this->back();
}

template <typename T> template <typename... A> constexpr coreList<T>::coreIterator coreList<T>::emplace_unsafe(const coreConstIterator& it, A&&... vArgs)
{
    ASSERT(this->size() < this->capacity())

    // access raw container pointers
    T** pptRaw = r_cast<T**>(this);
    ASSERT(pptRaw[0] == std::to_address(this->begin()))
    ASSERT(pptRaw[1] == std::to_address(this->end  ()))

    // get target index
    const coreUintW iIndex = this->index(it);
    ASSERT(iIndex < this->size() + 1u)

    if(it == this->end())
    {
        // manually construct object in place
        CALL_CONSTRUCTOR(pptRaw[1], std::forward<A>(vArgs)...);
        pptRaw[1] += 1u;
    }
    else
    {
        // handle possible aliasing of arguments with container elements
        T tTemp(std::forward<A>(vArgs)...);

        // move old elements back
        CALL_CONSTRUCTOR(pptRaw[1], std::move(*(pptRaw[1] - 1u)));
        std::move_backward(pptRaw[0] + iIndex, pptRaw[1] - 1u, pptRaw[1]);

        // insert new element
        (*(pptRaw[0] + iIndex)) = std::move(tTemp);
        pptRaw[1] += 1u;
    }

    return (this->begin() + iIndex);
}


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

    return (this->begin() + iIndex);
}


#endif /* _CORE_GUARD_LIST_H_ */