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
coreViewBox::coreViewBox()noexcept
: coreObject2D ()
, m_apObject   {}
, m_vOffset    (coreVector2(0.0f,0.0f))
, m_vOffsetOld (coreVector2(0.0f,0.0f))
, m_bScissor   (true)
{
    // enable interaction handling
    m_bFocusable = true;
}


// ****************************************************************
/* render the view-box */
void coreViewBox::Render()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return;

    if(m_bScissor)
    {
        const coreVector2 vPosition   = this->GetScreenPosition();
        const coreVector2 vSize       = this->GetScreenSize90() * 0.5f;
        const coreVector4 vResolution = Core::Graphics->GetViewResolution();
        const coreVector2 vLowerLeft  = (vPosition - vSize) * vResolution.zw();
        const coreVector2 vUpperRight = (vPosition + vSize) * vResolution.zw();

        // start scissor testing
        Core::Graphics->StartScissorTest(vLowerLeft, vUpperRight);

        // render view-objects
        FOR_EACH(it, m_apObject)
        {
            const coreVector2 vObjPosition = (*it)->GetScreenPosition();
            const coreVector2 vObjSize     = (*it)->GetScreenSize() * 0.5f;

            // cull outside of view-box
            if((ABS(vObjPosition.x - vPosition.x) <= vObjSize.x + vSize.x) &&
               (ABS(vObjPosition.y - vPosition.y) <= vObjSize.y + vSize.y))
            {
                (*it)->Render();
            }
        }

        // end scissor testing
        Core::Graphics->EndScissorTest();
    }
    else
    {
        // render view-objects (without culling)
        FOR_EACH(it, m_apObject) (*it)->Render();
    }
}


// ****************************************************************
/* move the view-box */
void coreViewBox::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // check for position offset changes
    const coreVector2 vMove = m_vOffset - m_vOffsetOld;
    if(!vMove.IsNull())
    {
        // update relative view-object transformations
        FOR_EACH(it, m_apObject)
        {
            coreObject2D* pObject = (*it);

            pObject->SetPosition(pObject->GetPosition() + vMove);
        }
    }

    coreBool bFocused = !this->IsFocused();

    // find current object with input focus
    FOR_EACH_REV(it, m_apObject)
    {
        coreObject2D* pObject = (*it);

        if(!bFocused)
        {
            pObject->Interact();
            if(pObject->IsFocused()) bFocused = true;
        }
        else
        {
            pObject->SetFocused(false);
        }
    }

    // move view-objects
    FOR_EACH(it, m_apObject)
    {
        coreObject2D* pObject = (*it);

        pObject->SetAlpha(this->GetAlpha());
        pObject->Move();
    }

    // move the 2d-object
    this->coreObject2D::Move();

    // store position offset
    m_vOffsetOld = m_vOffset;
}


// ****************************************************************
/* bind view-object */
void coreViewBox::BindObject(coreObject2D* pObject)
{
    ASSERT(pObject)

    // add view-object to list
    pObject->SetAlpha(0.0f);
    m_apObject.insert(pObject);
}


// ****************************************************************
/* unbind view-object */
void coreViewBox::UnbindObject(coreObject2D* pObject)
{
    ASSERT(pObject)

    // remove view-object from list
    m_apObject.index(m_apObject.erase(pObject));
}