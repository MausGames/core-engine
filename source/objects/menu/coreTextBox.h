//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TEXTBOX_H_
#define _CORE_GUARD_TEXTBOX_H_

// TODO: enable text selection and clipboard copy/cut
// TODO: enable clipboard paste


// ****************************************************************
// menu text-box class
class coreTextBox final : public coreButton
{
private:
    std::string m_sText;                //!< current text
    std::string m_sPrevious;            //!< previous text

    coreChar m_cCursor;                 //!< cursor character
    coreChar m_cReplace;                //!< replacement character for hidden text

    coreBool m_bInput;                  //!< text-input status
    coreBool m_bDisplay;                //!< caption update status (dirty flag)

    coreBool m_bReturned;               //!< text-input finished with return key

    static coreInt8 s_iActiveCounter;   //!< number of currently active text-boxes


public:
    coreTextBox()noexcept;
    coreTextBox(const coreChar* pcIdle, const coreChar* pcBusy, const coreChar* pcFont, const coreUint8& iHeight, const coreBool& bOutlined, const coreUint8& iLength)noexcept;

    DISABLE_COPY(coreTextBox)

    //! construct the text-box
    //! @{
    void Construct(const coreChar* pcIdle, const coreChar* pcBusy, const coreChar* pcFont, const coreUint8& iHeight, const coreBool& bOutlined, const coreUint8& iLength);
    //! @}

    //! move the text-box
    //! @{
    void Move()override;
    //! @}

    //! check for finished text-input
    //! @{
    inline const coreBool& IsReturned()const {return m_bReturned;}
    //! @}

    //! set object properties
    //! @{
    inline void SetText   (const coreChar* pcText)   {if(m_sText    != pcText)   {m_sText    = pcText;   m_bDisplay = true;}}
    inline void SetCursor (const coreChar& cCursor)  {if(m_cCursor  != cCursor)  {m_cCursor  = cCursor;  m_bDisplay = true;}}
    inline void SetReplace(const coreChar& cReplace) {if(m_cReplace != cReplace) {m_cReplace = cReplace; m_bDisplay = true;}}
    void SetInput(const coreBool& bInput);
    //! @}

    //! get object properties
    //! @{
    inline const coreChar* GetText   ()const {return m_sText.c_str();}
    inline const coreChar& GetCursor ()const {return m_cCursor;}
    inline const coreChar& GetReplace()const {return m_cReplace;}
    inline const coreBool& GetInput  ()const {return m_bInput;}
    //! @}


private:
    //! process new text-input characters
    //! @{
    coreBool __Write();
    //! @}
};


#endif // _CORE_GUARD_TEXTBOX_H_