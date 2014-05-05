//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreMenu::coreMenu(const coreByte& iNumSurfaces, const coreByte& iCurSurface)
: m_pCurObject   (NULL)                                   
, m_iNumSurface  (iNumSurfaces)
, m_iCurSurface  (iCurSurface)
, m_iOldSurface  (iCurSurface)
, m_Transition   (coreTimer(1.0f, 1.0f, 1))
, m_pFrameBuffer (NULL)
{
    // create surfaces
    m_papObject = new std::vector<coreObject2D*>[m_iNumSurface];

    // reserve memory for objects
    for(int i = 0; i < m_iNumSurface; ++i) m_papObject[i].reserve(32);
    for(int i = 0; i < 3;             ++i) m_aapRender[i].reserve(32);
}


// ****************************************************************
// destructor
coreMenu::~coreMenu()
{
    // remove all menu objects
    this->ClearObjects();

    // delete surfaces and frame buffer
    SAFE_DELETE_ARRAY(m_papObject)
    SAFE_DELETE(m_pFrameBuffer)
}


// ****************************************************************    
// render the menu
void coreMenu::Render()
{
    if(m_Transition.GetStatus())
    {
        // render transition between surfaces
        for(int i = 0; i < 3; ++i)
        {
            FOR_EACH(it, m_aapRender[i])
                (*it)->Render();
        }
    }
    else
    {
        // render current surface
        FOR_EACH(it, m_papObject[m_iCurSurface])
            (*it)->Render();
    }
}


// ****************************************************************
// move the menu
void coreMenu::Move()
{
    // reset current object
    m_pCurObject = NULL;

    if(m_Transition.GetStatus())
    {
        // update transition timer
        if(m_Transition.Update(1.0f))
        {
            // remove focus from old surface
            FOR_EACH(it, m_aapRender[1]) (*it)->SetFocus(false);
        }

        // set alpha value for each render-list
        const float afAlpha[3] = {this->GetAlpha(), 
                                  this->GetAlpha()*m_Transition.GetCurrent(true),
                                  this->GetAlpha()*m_Transition.GetCurrent(false)};

        // move transition between surfaces
        for(int i = 0; i < 3; ++i)
        {
            FOR_EACH(it, m_aapRender[i])
            {
                coreObject2D* pObject = (*it);

                // update object
                pObject->SetAlpha(afAlpha[i]);
                pObject->Move();
            }
        }    
    }
    else
    {
        // move current surface
        FOR_EACH_REV(it, m_papObject[m_iCurSurface])
        {
            coreObject2D* pObject = (*it);

            // find current object with input focus 
            if(!m_pCurObject)
            {
                // interact and check status
                pObject->Interact();
                if(pObject->IsFocused()) m_pCurObject = pObject;
            }
            else pObject->SetFocus(false);

            // update object
            pObject->SetAlpha(this->GetAlpha());
            pObject->Move();
        }
    }
}


// ****************************************************************
// add menu object
void coreMenu::AddObject(const coreByte& iSurface, coreObject2D* pObject)
{
#if defined(_CORE_DEBUG_)

    // check for duplicate menu objects
    FOR_EACH(it, m_papObject[iSurface])
        SDL_assert((*it) != pObject);

#endif

    // add menu object
    pObject->SetAlpha(0.0f);
    m_papObject[iSurface].push_back(pObject);
}


// ****************************************************************
// remove menu object
void coreMenu::RemoveObject(const coreByte& iSurface, coreObject2D* pObject)
{
    // loop through all menu objects
    FOR_EACH(it, m_papObject[iSurface])
    {
        if((*it) == pObject)
        {
            // remove the requested object
            m_papObject[iSurface].erase(it);
            return;
        }
    }
}


// ****************************************************************
// remove all menu objects
void coreMenu::ClearObjects()
{
    for(int i = 0; i < m_iNumSurface; ++i) m_papObject[i].clear();
    for(int i = 0; i < 3;             ++i) m_aapRender[i].clear();
}


// ****************************************************************
// change current surface
bool coreMenu::ChangeSurface(const coreByte& iNewSurface, const float& fSpeed)
{
           if(iNewSurface == m_iCurSurface) return false;
    ASSERT_IF(iNewSurface >= m_iNumSurface) return false;

    if(!fSpeed)
    {
        // change surface without transition
        FOR_EACH(it, m_papObject[m_iCurSurface]) {(*it)->SetAlpha(0.0f); (*it)->SetFocus(false);}
        FOR_EACH(it, m_papObject[iNewSurface])   {(*it)->SetAlpha(this->GetAlpha()); (*it)->Move();}
    }
    else
    {
        // hide old surface on a fast switch
        if(m_Transition.GetStatus())
        {
            FOR_EACH(it, m_aapRender[1]) {(*it)->SetAlpha(0.0f); (*it)->SetFocus(false);}
        }

        // clear and refill all render-lists
        for(int i = 0; i < 3; ++i) m_aapRender[i].clear();
        FOR_EACH(it, m_papObject[m_iCurSurface]) m_aapRender[1].push_back(*it);
        FOR_EACH(it, m_papObject[iNewSurface])   m_aapRender[2].push_back(*it);

        // find objects on both surfaces
        for(coreUint i = 0; i < m_aapRender[1].size(); ++i)
        {
            for(coreUint j = 0; j < m_aapRender[2].size(); ++j)
            {
                if(m_aapRender[1][i] == m_aapRender[2][j])
                {
                    // move object to own render-list
                    m_aapRender[0].push_back(m_aapRender[1][i]);
                    m_aapRender[1].erase(m_aapRender[1].begin()+i);
                    m_aapRender[2].erase(m_aapRender[2].begin()+j);

                    --i;
                    break;
                }
            }
        }

        // set and start the transition
        m_Transition.SetSpeed(fSpeed);
        m_Transition.Play(true);
    }

    // save new surface numbers
    m_iOldSurface = m_iCurSurface;
    m_iCurSurface = iNewSurface;
    
    return true;
}