//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LABEL_H_
#define _CORE_GUARD_LABEL_H_


// ****************************************************************
// menu label class
class coreLabel final : public coreObject2D, public coreReset
{
private:
    coreFontPtr m_pFont;         //!< font object
    int m_iHeight;               //!< specific height for the font

    coreVector2 m_vResolution;   //!< resolution of the generated texture
    int m_iLength;               //!< max number of characters (0 = dynamic)
                           
    std::string m_sText;         //!< current text
    float m_fScale;              //!< scale factor

    coreByte m_iGenerate;        //!< generate status (0 = do nothing | 1 = update only size | 2 = update only texture | 3 = update texture and size)


public:
    coreLabel(const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept;
    ~coreLabel();

    //! render and move the label
    //! @{
    void Render()override;
    void Move()override;
    //! @}

    //! set object attributes
    //! @{
    bool SetText(const char* pcText, int iNum = -1);
    inline void SetScale(const float& fScale) {if(m_fScale != fScale) {m_iGenerate |= 1; m_fScale = fScale;}}
    //! @}
    
    //! get object attributes
    //! @{
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    inline const int& GetLength()const             {return m_iLength;}
    inline const char* GetText()const              {return m_sText.c_str();}
    inline const float& GetScale()const            {return m_fScale;}
    //! @}


private:
    //! reset with the resource manager
    //! @{
    void __Reset(const bool& bInit)override {SDL_assert(false);}
    //! @}

    //! update the texture of the label 
    //! @{
    void __Generate();
    //! @}
};


#endif // _CORE_GUARD_LABEL_H_