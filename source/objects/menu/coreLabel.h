//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LABEL_H_
#define _CORE_GUARD_LABEL_H_

// TODO: 3d text with link or own class ?
// TODO: implement multi-line text with automatic newline if row is too long (snippet in p1) (single texture with line height) or TTF_RenderText_Blended_Wrapped
// TODO: transformation matrix is not always immediately updated after a Move(), because re-generation must be in Render(), with Move() afterwards
// TODO: in __Generate, use font-size calculation interface to check for font-size and pre-allocations
// TODO: change text-generation to per-glyph interface or gather all text into big textures -> enable instancing


// ****************************************************************
// menu label definitions
#define CORE_LABEL_DETAIL        (Core::System->GetResolution().y * 0.96f)
#define CORE_LABEL_SIZE_FACTOR   (RCP(CORE_LABEL_DETAIL))       //!< map texture resolution on current window resolution
#define CORE_LABEL_HEIGHT_FACTOR (CORE_LABEL_DETAIL / 800.0f)   //!< set real font height relative to current window resolution

#define CORE_LABEL_HEIGHT_RELATIVE(x) (F_TO_UI(I_TO_F(x) * CORE_LABEL_HEIGHT_FACTOR))

enum coreLabelUpdate : coreUint8
{
    CORE_LABEL_UPDATE_NOTHING = 0x00u,   //!< update nothing
    CORE_LABEL_UPDATE_SIZE    = 0x01u,   //!< update object size
    CORE_LABEL_UPDATE_TEXTURE = 0x02u,   //!< update and generate texture
    CORE_LABEL_UPDATE_ALL     = 0x03u    //!< update everything
};
ENABLE_BITWISE(coreLabelUpdate)


// ****************************************************************
// menu label class
class coreLabel final : public coreObject2D, public coreTranslate, public coreResourceRelation
{
private:
    coreFontPtr m_pFont;         //!< font object
    coreUint8   m_iHeight;       //!< specific height for the font
    coreUint8   m_iOutline;      //!< create very sharp outlined text

    coreUint8   m_iLength;       //!< max number of characters (0 = dynamic)
    coreVector2 m_vResolution;   //!< resolution of the generated texture

    std::string m_sText;         //!< current text
    coreFloat   m_fScale;        //!< scale factor

    coreLabelUpdate m_iUpdate;   //!< update status (dirty flag)


public:
    coreLabel()noexcept;
    coreLabel(const coreHashString& sFont, const coreUint8 iHeight, const coreUint8 iOutline, const coreUint8 iLength)noexcept;
    ~coreLabel();

    DISABLE_COPY(coreLabel)

    //! construct the label
    //! @{
    void Construct(const coreHashString& sFont, const coreUint8 iHeight, const coreUint8 iOutline, const coreUint8 iLength);
    //! @}

    //! render and move the label
    //! @{
    void Render()override;
    void Move  ()override;
    //! @}

    //! set object properties
    //! @{
    coreBool    SetText        (const coreChar*       pcText);
    coreBool    SetText        (const coreChar*       pcText, const coreUint8 iNum);
    inline void SetTextLanguage(const coreHashString& sKey)   {this->_BindString(&m_sText, sKey);}
    inline void SetScale       (const coreFloat       fScale) {if(m_fScale != fScale) {ADD_VALUE(m_iUpdate, CORE_LABEL_UPDATE_SIZE) m_fScale = fScale;}}
    //! @}

    //! get object properties
    //! @{
    inline const coreFontPtr& GetFont      ()const {return m_pFont;}
    inline const coreUint8&   GetHeight    ()const {return m_iHeight;}
    inline const coreUint8&   GetOutline   ()const {return m_iOutline;}
    inline const coreUint8&   GetLength    ()const {return m_iLength;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    inline const coreChar*    GetText      ()const {return m_sText.c_str();}
    inline const coreFloat&   GetScale     ()const {return m_fScale;}
    //! @}


private:
    //! reset with the resource manager
    //! @{
    void __Reset(const coreResourceReset bInit)override;
    //! @}

    //! update object after modification
    //! @{
    inline void __Update()override {ADD_VALUE(m_iUpdate, CORE_LABEL_UPDATE_ALL)}
    //! @}

    //! generate the texture
    //! @{
    void __Generate(const coreChar* pcText, const coreBool bSub);
    //! @}
};


#endif // _CORE_GUARD_LABEL_H_