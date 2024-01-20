///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_VIEWBOX_H_
#define _CORE_GUARD_VIEWBOX_H_

// TODO 3: propagate direction
// TODO 5: propagate size
// TODO 3: propagate color and alpha (relative)
// TODO 3: implement way to change view-data
// TODO 3: CORE_OBJECT_UPDATE_TRANSFORM when changing offset also updates the view-box object, even though it did not change


// ****************************************************************
/* menu view-box class */
class coreViewBox : public coreObject2D
{
private:
    /* view-object data */
    struct coreViewData final
    {
        coreVector2 vPosition;   // relative position
    };


private:
    coreSet<coreObject2D*> m_apObject;   // pointers to view-objects
    coreList<coreViewData> m_aData;      // additional data for each view-object

    coreVector2 m_vOffset;               // position offset for the whole content
    coreBool    m_bScissor;              // use scissor testing


public:
    coreViewBox()noexcept;
    virtual ~coreViewBox()override = default;

    DISABLE_COPY(coreViewBox)

    /* move the view-box */
    virtual void Render()override;
    virtual void Move  ()override;

    /* bind and unbind view-objects */
    void BindObject  (coreObject2D* pObject);
    void UnbindObject(coreObject2D* pObject);

    /* set object properties */
    void SetOffset (const coreVector2 vOffset)  {if(m_vOffset != vOffset) {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vOffset = vOffset;}}
    void SetScissor(const coreBool    bScissor) {m_bScissor = bScissor;}

    /* get object properties */
    inline const coreVector2& GetOffset ()const {return m_vOffset;}
    inline const coreBool&    GetScissor()const {return m_bScissor;}
};


#endif /* _CORE_GUARD_VIEWBOX_H_ */