///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreMenu::coreMenu(const coreUint8 iNumSurfaces, const coreUint8 iStartSurface)noexcept
: coreObject2D   ()
, m_papObject    (NULL)
, m_pCurObject   (NULL)
, m_iNumSurfaces (iNumSurfaces)
, m_iCurSurface  (iStartSurface)
, m_iOldSurface  (iStartSurface)
, m_Transition   (coreTimer(1.0f, 1.0f, 1u))
, m_aapRender    {}
{
    // create surfaces
    ASSERT(m_iNumSurfaces)
    m_papObject = new coreSet<coreObject2D*>[m_iNumSurfaces];
}


// ****************************************************************
/* destructor */
coreMenu::~coreMenu()
{
    // remove all menu objects
    for(coreUintW i = 0u; i < m_iNumSurfaces;          ++i) m_papObject[i].clear();
    for(coreUintW i = 0u; i < ARRAY_SIZE(m_aapRender); ++i) m_aapRender[i].clear();

    // delete surfaces
    SAFE_DELETE_ARRAY(m_papObject)
}


// ****************************************************************
/* render the menu */
void coreMenu::Render()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return;

    if(m_Transition.GetStatus())
    {
        // render transition between surfaces
        for(coreUintW i = 0u; i < ARRAY_SIZE(m_aapRender); ++i)
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
/* move the menu */
void coreMenu::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // reset current object
    m_pCurObject = NULL;

    if(m_Transition.GetStatus())
    {
        // update transition timer
        if(m_Transition.Update(1.0f))
        {
            // remove focus from old surface
            FOR_EACH(it, m_aapRender[1]) (*it)->SetFocused(false);
        }

        // set alpha value for each render-list
        const coreFloat afAlpha[3] = {this->GetAlpha(),
                                      this->GetAlpha() * m_Transition.GetValue(CORE_TIMER_GET_REVERSED),
                                      this->GetAlpha() * m_Transition.GetValue(CORE_TIMER_GET_NORMAL)};

        // move transition between surfaces
        for(coreUintW i = 0u; i < ARRAY_SIZE(m_aapRender); ++i)
        {
            FOR_EACH(it, m_aapRender[i])
            {
                coreObject2D* pObject = (*it);

                pObject->SetAlpha(afAlpha[i]);
                pObject->Move();
            }
        }
    }
    else
    {
        // find current object with input focus
        FOR_EACH_REV(it, m_papObject[m_iCurSurface])
        {
            coreObject2D* pObject = (*it);

            if(!m_pCurObject)
            {
                pObject->Interact();
                if(pObject->IsFocused()) m_pCurObject = pObject;
            }
            else
            {
                pObject->SetFocused(false);
            }
        }

        // move current surface
        FOR_EACH(it, m_papObject[m_iCurSurface])
        {
            coreObject2D* pObject = (*it);

            pObject->SetAlpha(this->GetAlpha());
            pObject->Move();
        }
    }
}


// ****************************************************************
/* bind menu object */
void coreMenu::BindObject(const coreUintW iSurface, coreObject2D* pObject)
{
    ASSERT((iSurface < m_iNumSurfaces) && pObject)

    // add menu object
    pObject->SetAlpha(0.0f);
    m_papObject[iSurface].insert(pObject);
}


// ****************************************************************
/* unbind menu object */
void coreMenu::UnbindObject(const coreUintW iSurface, coreObject2D* pObject)
{
    ASSERT((iSurface < m_iNumSurfaces) && pObject)

    // remove the requested object
    m_papObject[iSurface].erase(pObject);
}


// ****************************************************************
/* change current surface */
coreBool coreMenu::ChangeSurface(const coreUint8 iNewSurface, const coreFloat fSpeed)
{
         if(iNewSurface == m_iCurSurface)  return false;
    WARN_IF(iNewSurface >= m_iNumSurfaces) return false;

    if(fSpeed <= 0.0f)
    {
        // change surface without transition
        FOR_EACH(it, m_papObject[m_iCurSurface]) {(*it)->SetAlpha(0.0f); (*it)->SetFocused(false); (*it)->Move();}   // propagate alpha
        FOR_EACH(it, m_papObject[iNewSurface])   {(*it)->SetAlpha(this->GetAlpha());               (*it)->Move();}
    }
    else
    {
        // hide old surface on a fast switch
        if(m_Transition.GetStatus())
        {
            FOR_EACH(it, m_aapRender[1]) {(*it)->SetAlpha(0.0f); (*it)->SetFocused(false); (*it)->Move();}
        }

        // clear and refill all render-lists
        for(coreUintW i = 0u; i < ARRAY_SIZE(m_aapRender); ++i) m_aapRender[i].clear();
        FOR_EACH(it, m_papObject[m_iCurSurface]) m_aapRender[1].push_back(*it);
        FOR_EACH(it, m_papObject[iNewSurface])   m_aapRender[2].push_back(*it);

        // find objects on both surfaces
        for(coreUintW i = 0u; i < m_aapRender[1].size(); ++i)
        {
            for(coreUintW j = 0u; j < m_aapRender[2].size(); ++j)
            {
                if(m_aapRender[1][i] == m_aapRender[2][j])
                {
                    // move object to own render-list
                    m_aapRender[0].push_back(m_aapRender[1][i]);
                    m_aapRender[1].erase_index(i);
                    m_aapRender[2].erase_index(j);

                    --i;
                    break;
                }
            }
        }

        // set and start the transition
        m_Transition.SetSpeed(fSpeed);
        m_Transition.Play(CORE_TIMER_PLAY_RESET);
    }

    // save new surface numbers
    m_iOldSurface = m_iCurSurface;
    m_iCurSurface = iNewSurface;

    return true;
}