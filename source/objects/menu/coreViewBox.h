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


// ****************************************************************
/* menu view-box class */
class coreViewBox : public coreObject2D
{
private:
    coreSet<coreObject2D*> m_apObject;   // pointers to view-objects

    coreVector2 m_vOffset;               // position offset for the whole content
    coreVector2 m_vOffsetOld;            // old position offset (to handle movement)
    coreBool    m_bScissor;              // use scissor testing


public:
    coreViewBox()noexcept;
    virtual ~coreViewBox()override = default;

    DISABLE_COPY(coreViewBox)

    /* render and move the view-box */
    virtual void Render()override;
    virtual void Move  ()override;

    /* bind and unbind view-objects */
    void BindObject  (coreObject2D* pObject);
    void UnbindObject(coreObject2D* pObject);

    /* check if view-object belongs to the view-box */
    inline coreBool ContainsObject(coreObject2D* pObject)const {return m_apObject.count(pObject);}

    /* access view-object list directly */
    inline const coreSet<coreObject2D*>* List()const {return &m_apObject;}

    /* set object properties */
    void SetOffset (const coreVector2 vOffset)  {m_vOffset  = vOffset;}
    void SetScissor(const coreBool    bScissor) {m_bScissor = bScissor;}

    /* get object properties */
    inline const coreVector2& GetOffset ()const {return m_vOffset;}
    inline const coreBool&    GetScissor()const {return m_bScissor;}
};


#endif /* _CORE_GUARD_VIEWBOX_H_ */