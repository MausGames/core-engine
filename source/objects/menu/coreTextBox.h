///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TEXTBOX_H_
#define _CORE_GUARD_TEXTBOX_H_

// TODO 3: enable text selection and correct clipboard cut/copy/paste
// TODO 3: enable text-cursor positioning (mouse + arrow keys)
// TODO 5: <old comment style>
// TODO 3: only allow characters which exist in the font (though this may need to be handled by the label) (m_pCaption->GetFont()->IsGlyphProvided(pcText))
// TODO 3: how to handle length with UTF8 support? there is a difference between storage requirements and characters to display
// TODO 5: implement partial paste with UTF-8 support ?
// TODO 3: make use of SDL_SetTextInputArea


// ****************************************************************
/* menu text-box class */
class coreTextBox : public coreButton
{
private:
    coreString m_sText;                 // current text
    coreString m_sPrevious;             // previous text

    coreUint8 m_iLength;                // max number of characters
    coreChar  m_cCursor;                // cursor character
    coreChar  m_cReplace;               // replacement character for hidden text

    coreBool m_bInput;                  // text-input status
    coreBool m_bDisplay;                // caption update status (dirty flag)

    coreBool m_bReturned;               // text-input finished with return key

    static coreInt8 s_iActiveCounter;   // number of currently active text-boxes


public:
    coreTextBox()noexcept;
    coreTextBox(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline, const coreUint8 iLength)noexcept;
    virtual ~coreTextBox()override = default;

    DISABLE_COPY(coreTextBox)

    /* construct the text-box */
    void Construct(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline, const coreUint8 iLength);

    /* move the text-box */
    virtual void Move()override;

    /* check for finished text-input */
    inline const coreBool& IsReturned()const {return m_bReturned;}

    /* set object properties */
    inline void SetText   (const coreChar* pcText)   {if(m_sText    != pcText)   {m_sText    = pcText;   m_bDisplay = true; ASSERT(m_sText.length() <= m_iLength)}}
    inline void SetCursor (const coreChar  cCursor)  {if(m_cCursor  != cCursor)  {m_cCursor  = cCursor;  m_bDisplay = true;}}
    inline void SetReplace(const coreChar  cReplace) {if(m_cReplace != cReplace) {m_cReplace = cReplace; m_bDisplay = true;}}
    void SetInput(const coreBool bInput);

    /* get object properties */
    inline const coreChar* GetText   ()const {return m_sText.c_str();}
    inline       coreUintW GetTextLen()const {return m_sText.length();}
    inline const coreChar& GetCursor ()const {return m_cCursor;}
    inline const coreChar& GetReplace()const {return m_cReplace;}
    inline const coreBool& GetInput  ()const {return m_bInput;}


private:
    /* process new text-input characters */
    coreBool __Write();
};


#endif /* _CORE_GUARD_TEXTBOX_H_ */