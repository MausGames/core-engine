//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TEXTBOX_H_
#define _CORE_GUARD_TEXTBOX_H_


// ****************************************************************
// menu text-box class
// TODO: enable text selection and clipboard copy/cut
// TODO: enable clipboard paste
class coreTextBox final : public coreButton
{
private:
    std::string m_sText;       //!< current text
    std::string m_sPrevious;   //!< previous text
    char m_cReplace;           //!< replacement character for hidden text

    bool m_bInput;             //!< text-input status
    bool m_bDisplay;           //!< caption update status

    bool m_bReturned;          //!< text-input finished with return key


public:
    coreTextBox(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept;

    //! move the text-box
    //! @{
    void Move()override;
    //! @}

    //! check for finished text-input
    //! @{
    inline const bool& IsReturned()const {return m_bReturned;}
    //! @}

    //! set object attributes
    //! @{
    inline void SetText(const char* pcText)      {if(m_sText    != pcText)   {m_sText    = pcText;   m_bDisplay = true;}}
    inline void SetReplace(const char& cReplace) {if(m_cReplace != cReplace) {m_cReplace = cReplace; m_bDisplay = true;}}
    void SetInput(const bool& bInput);
    //! @}

    //! get object attributes
    //! @{
    inline const char* GetText()const    {return m_sText.c_str();}
    inline const char& GetReplace()const {return m_cReplace;}
    inline const bool& GetInput()const   {return m_bInput;}
    //! @}


private:
    //! process new text-input characters
    //! @{
    bool __Write();
    //! @}
};


#endif // _CORE_GUARD_TEXTBOX_H_