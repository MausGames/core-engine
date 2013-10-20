//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TREE_H_
#define _CORE_GUARD_TREE_H_


// ****************************************************************
// tree extension
template <typename T> class coreTree
{
private:
    T* m_pParent;             //!< superior object
    std::set<T*> m_apChild;   //!< list with subordinate objects


protected:
    coreTree()noexcept : m_pParent (NULL) {}
    ~coreTree()                           {this->SetParent(NULL); this->ClearChildren();}


public:
    //! set parent
    //! @{
    bool SetParent(T* pObject);
    //! @}

    //! manage children
    //! @{
    bool AddChild(T* pObject);
    bool RemoveChild(T* pObject);
    void ClearChildren();
    //! @}

    //! access parent and children
    //! @{
    inline T* GetParent()const                     {return m_pParent;}
    inline const std::set<T*>& GetChildList()const {return m_apChild;}
    //! @}
};



// ****************************************************************
// set parent
template <typename T> bool coreTree<T>::SetParent(T* pObject)
{
    if(m_pParent == pObject) return false;

    // change the current parent
    if(m_pParent) m_pParent->RemoveChild(this);
    if(pObject) pObject->AddChild(this);

    return true;
}


// ****************************************************************
// add child
template <typename T> bool coreTree<T>::AddChild(T* pObject)
{
    SDL_assert(!m_apChild.count(pObject));
    if(m_apChild.count(pObject)) return false;

    // disconnect child from old parent
    if(pObject->m_pParent)
        pObject->m_pParent->m_apChild.erase(pObject);

    // connect child and parent
    m_apChild.insert(pObject);
    pObject->m_pParent = this;

    return true;
}


// ****************************************************************
// remove child
template <typename T> bool coreTree<T>::RemoveChild(T* pObject)
{
    SDL_assert(m_apChild.count(pObject));
    if(!m_apChild.count(pObject)) return false;

    // disconnect child and parent
    m_apChild.erase(pObject);
    pObject->m_pParent = NULL;

    return true;
}


// ****************************************************************
// remove all children
template <typename T> void coreTree<T>::ClearChildren()
{
    // remove parents from children
    for(auto it = m_apChild.begin(); it != m_apChild.end(); ++it)
        (*it)->m_pParent = NULL;

    // clear memory
    m_apChild.clear();
}


#endif // _CORE_GUARD_TREE_H_