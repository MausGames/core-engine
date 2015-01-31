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


// ****************************************************************
// menu label class
class coreLabel final : public coreObject2D, public coreResourceRelation, public coreTranslate
{
private:
    coreFontPtr m_pFont;         //!< font object
    int         m_iHeight;       //!< specific height for the font

    coreVector2 m_vResolution;   //!< resolution of the generated texture
    coreUint    m_iLength;       //!< max number of characters (0 = dynamic)

    std::string m_sText;         //!< current text
    float       m_fScale;        //!< scale factor

    coreByte m_iGenerate;        //!< generation status (0 = do nothing | 1 = update only size | 3 = update texture and size)


public:
    coreLabel()noexcept;
    coreLabel(const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept;
    ~coreLabel();

    DISABLE_COPY(coreLabel)

    //! construct the label
    //! @{
    void Construct(const char* pcFont, const int& iHeight, const coreUint& iLength);
    //! @}

    //! render and move the label
    //! @{
    void Render()override;
    void Move  ()override;
    //! @}

    //! set object properties
    //! @{
    bool        SetText        (const char*  pcText);
    bool        SetText        (const char*  pcText, const coreUint& iNum);
    inline void SetTextLanguage(const char*  pcKey)  {this->_BindString(&m_sText, pcKey);}
    inline void SetScale       (const float& fScale) {if(m_fScale != fScale) {m_iGenerate |= 1; m_fScale = fScale;}}
    //! @}

    //! get object properties
    //! @{
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    inline const coreUint&    GetLength    ()const {return m_iLength;}
    inline const char*        GetText      ()const {return m_sText.c_str();}
    inline const float&       GetScale     ()const {return m_fScale;}
    //! @}


private:
    //! reset with the resource manager
    //! @{
    void __Reset(const coreResourceReset& bInit)override;
    //! @}

    //! update object after modification
    //! @{
    inline void __Update()override {m_iGenerate = 3;}
    //! @}

    //! generate the texture
    //! @{
    void __Generate(const char* pcText, const bool& bSub);
    //! @}
};


#endif // _CORE_GUARD_LABEL_H_