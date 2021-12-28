///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreViewBox::coreViewBox()noexcept
: coreObject2D ()
, m_apObject   {}
, m_aData      {}
, m_vOffset    (coreVector2(0.0f,0.0f))
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

    // start scissor testing
    if(m_bScissor)
    {
        const coreVector2 vResolution = Core::Graphics->GetViewResolution().xy();
        const coreVector2 vPosition   = coreVector2(m_mTransform._31, m_mTransform._32);
        const coreVector2 vSize       = coreVector2(coreVector2(m_mTransform._11, m_mTransform._21).Length(), coreVector2(m_mTransform._12, m_mTransform._22).Length());
        const coreVector2 vLowerLeft  = (vPosition - vSize * 0.5f) / vResolution;
        const coreVector2 vUpperRight = (vPosition + vSize * 0.5f) / vResolution;

        Core::Graphics->StartScissorTest(vLowerLeft, vUpperRight);
    }

    // render view-objects
    FOR_EACH(it, m_apObject) (*it)->Render();

    // end scissor testing
    if(m_bScissor)
    {
        Core::Graphics->EndScissorTest();
    }
}


// ****************************************************************
/* move the view-box */
void coreViewBox::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    if(HAS_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
    {
        // update relative view-object transformations
        FOR_EACH(it, m_apObject)
        {
            coreObject2D* pObject = (*it);

            const coreViewData& oData = m_aData[m_apObject.index(it)];
            pObject->SetPosition(this->GetPosition() + oData.vPosition + m_vOffset);
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
}


// ****************************************************************
/* bind view-object */
void coreViewBox::BindObject(coreObject2D* pObject)
{
    ASSERT(pObject)

    // create new view-data
    coreViewData oData = {};
    oData.vPosition = pObject->GetPosition() - this->GetPosition();

    // add view-object to list
    m_apObject.insert(pObject);

    // add view-data to list
    m_aData.push_back(oData);
}


// ****************************************************************
/* unbind view-object */
void coreViewBox::UnbindObject(coreObject2D* pObject)
{
    ASSERT(pObject)

    // remove view-object from list
    const coreUintW iIndex = m_apObject.index(m_apObject.erase(pObject));

    // remove view-data from list
    if(iIndex < m_aData.size()) m_aData.erase(m_aData.begin() + iIndex);
}