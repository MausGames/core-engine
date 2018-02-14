//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SET_H_
#define _CORE_GUARD_SET_H_


// ****************************************************************
/* extended set class */
template <typename T> class coreSet final : public std::vector<T>
{
public:
    coreSet() = default;

    ENABLE_COPY(coreSet)

    /*! insert new unique item */
    //! @{
    using std::vector<T>::insert;
    inline void insert(const T& tItem) {ASSERT(!this->count(tItem)) this->push_back(tItem);}
    //! @}

    /*! remove existing item */
    //! @{
    using std::vector<T>::erase;
    inline typename std::vector<T>::iterator erase(const T& tItem) {FOR_EACH(it, *this) if((*it) == tItem) return this->erase(it); return this->end();}
    //! @}

    /*! check for existing item */
    //! @{
    inline coreBool count(const T& tItem)const {FOR_EACH(it, *this) if((*it) == tItem) return true; return false;}
    //! @}
};


#endif /* _CORE_GUARD_SET_H_ */