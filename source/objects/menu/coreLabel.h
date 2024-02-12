///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LABEL_H_
#define _CORE_GUARD_LABEL_H_

// TODO 5: <old comment style>
// TODO 3: coreResourceRelation in coreLabel is spamming the resource-manager
// TODO 2: right and bottom side bleeds on non-aligned text, maybe add dummy lines/row, or clear instead of invalidate on non-aligned


// ****************************************************************
/* menu label definitions */
#define CORE_LABEL_DETAIL        (Core::System->GetResolution().y * 1.0f)
#define CORE_LABEL_SIZE_FACTOR   (RCP(CORE_LABEL_DETAIL))       // map texture resolution on current window resolution
#define CORE_LABEL_HEIGHT_FACTOR (CORE_LABEL_DETAIL / 800.0f)   // set real font height relative to current window resolution
#define CORE_LABEL_TEXTURE       (1u)                           // default texture unit (other than 0, to reduce texture switches)

#define CORE_LABEL_HEIGHT_RELATIVE(x)  (F_TO_UI(I_TO_F(x) * CORE_LABEL_HEIGHT_FACTOR))
#define CORE_LABEL_OUTLINE_RELATIVE(x) ((x) ? MAX(CORE_LABEL_HEIGHT_RELATIVE(x), 1u) : 0u)

enum coreLabelRefresh : coreUint8
{
    CORE_LABEL_REFRESH_NOTHING = 0x00u,   // refresh nothing
    CORE_LABEL_REFRESH_SIZE    = 0x01u,   // refresh object size
    CORE_LABEL_REFRESH_TEXTURE = 0x02u,   // refresh and generate texture
    CORE_LABEL_REFRESH_ALL     = 0x03u    // refresh everything
};
ENABLE_BITWISE(coreLabelRefresh)


// ****************************************************************
/* menu label class */
class coreLabel : public coreObject2D, public coreTranslate, public coreResourceRelation
{
private:
    coreFontPtr m_pFont;           // font object
    coreUint16  m_iHeight;         // specific height for the font
    coreUint8   m_iOutline;        // create very sharp outlined text

    coreVector2 m_vResolution;     // resolution of the generated texture

    coreString  m_sText;           // current text
    coreVector2 m_vScale;          // scale factor
    coreUint8   m_iRectify;        // align texture with screen pixels (X, Y)

    coreInt8 m_iShift;             // vertical shift for handling ascenders and descenders

    coreLabelRefresh m_eRefresh;   // refresh status (dirty flag)


public:
    coreLabel()noexcept;
    coreLabel(const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)noexcept;
    virtual ~coreLabel()override;

    DISABLE_COPY(coreLabel)

    /* construct the label */
    void Construct(const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline);

    /* render and move the label */
    virtual void Render()override;
    virtual void Move  ()override;

    /* retrieve desired size without rendering */
    template <typename F> void RetrieveDesiredSize(F&& nRetrieveFunc)const;   // [](const coreVector2 vSize) -> void

    /* invoke texture generation */
    inline void RegenerateTexture() {ADD_FLAG(m_eRefresh, CORE_LABEL_REFRESH_ALL)}

    /* set object properties */
    coreBool    SetText        (const coreChar*       pcText);
    coreBool    SetText        (const coreChar*       pcText, const coreUint16 iNum);
    inline void SetTextLanguage(const coreHashString& sKey)     {this->_BindString(&m_sText, sKey);}
    inline void SetScale       (const coreVector2     vScale)   {if(m_vScale != vScale)                      {ADD_FLAG(m_eRefresh, CORE_LABEL_REFRESH_SIZE)      m_vScale = vScale;}}
    inline void SetRectifyX    (const coreBool        bRectify) {if(HAS_FLAG(m_iRectify, 0x01u) != bRectify) {ADD_FLAG(m_eUpdate,  CORE_OBJECT_UPDATE_TRANSFORM) SET_FLAG(m_iRectify, 0x01u, bRectify)}}
    inline void SetRectifyY    (const coreBool        bRectify) {if(HAS_FLAG(m_iRectify, 0x02u) != bRectify) {ADD_FLAG(m_eUpdate,  CORE_OBJECT_UPDATE_TRANSFORM) SET_FLAG(m_iRectify, 0x02u, bRectify)}}
    inline void SetRectify     (const coreBool        bRectify) {if(HAS_FLAG(m_iRectify, 0x03u) != bRectify) {ADD_FLAG(m_eUpdate,  CORE_OBJECT_UPDATE_TRANSFORM) SET_FLAG(m_iRectify, 0x03u, bRectify)}}

    /* get object properties */
    inline const coreFontPtr& GetFont      ()const {return m_pFont;}
    inline const coreUint16&  GetHeight    ()const {return m_iHeight;}
    inline const coreUint8&   GetOutline   ()const {return m_iOutline;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    inline const coreChar*    GetText      ()const {return m_sText.c_str();}
    inline const coreVector2& GetScale     ()const {return m_vScale;}
    inline const coreUint8&   GetRectify   ()const {return m_iRectify;}


private:
    /* reset with the resource manager */
    void __Reset(const coreResourceReset eInit)final;

    /* reshape with the resource manager */
    void __Reshape()final;

    /* update object after modification */
    inline void __Update()final {ADD_FLAG(m_eRefresh, CORE_LABEL_REFRESH_ALL)}

    /* generate the texture */
    void __GenerateTexture(const coreChar* pcText);

    /* move and adjust the label */
    void __RefreshSize();
    void __MoveRectified();
};


// ****************************************************************
/* retrieve desired size without rendering */
template <typename F> void coreLabel::RetrieveDesiredSize(F&& nRetrieveFunc)const
{
    if(HAS_FLAG(m_eRefresh, CORE_LABEL_REFRESH_SIZE))
    {
        // check if requested font is loaded
        m_pFont.OnUsableOnce([=, this]()
        {
            // get relative font height and outline
            const coreUint16 iRelHeight  = CORE_LABEL_HEIGHT_RELATIVE (m_iHeight);
            const coreUint8  iRelOutline = CORE_LABEL_OUTLINE_RELATIVE(m_iOutline);

            // return the dimensions of the current text
            const coreVector2 vDimensions = m_pFont->RetrieveTextDimensions(m_sText.c_str(), iRelHeight, iRelOutline);
            nRetrieveFunc(vDimensions * m_vScale * CORE_LABEL_SIZE_FACTOR);
        });
    }
    else
    {
        // return actual size
        nRetrieveFunc(this->GetSize());
    }
}


#endif /* _CORE_GUARD_LABEL_H_ */