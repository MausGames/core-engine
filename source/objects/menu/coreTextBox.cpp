///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

coreInt8 coreTextBox::s_iActiveCounter = 0;


// ****************************************************************
/* constructor */
coreTextBox::coreTextBox()noexcept
: coreButton  ()
, m_sText     ("")
, m_sPrevious ("")
, m_iLength   (0u)
, m_cCursor   ('|')
, m_cReplace  ('\0')
, m_bInput    (false)
, m_bDisplay  (false)
, m_bReturned (false)
{
}

coreTextBox::coreTextBox(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline, const coreUint8 iLength)noexcept
: coreTextBox ()
{
    // construct on creation
    this->Construct(sIdle, sBusy, sFont, iHeight, iOutline, iLength);
}


// ****************************************************************
/* construct the text-box */
void coreTextBox::Construct(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline, const coreUint8 iLength)
{
    ASSERT(iLength)

    // save properties
    m_iLength = iLength;

    // construct the button
    this->coreButton::Construct(sIdle, sBusy, sFont, iHeight, iOutline);

    // reserve memory for text
    m_sText    .reserve(iLength);
    m_sPrevious.reserve(iLength);
}


// ****************************************************************
/* move the text-box */
void coreTextBox::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // move the button
    this->coreButton::Move();

    // check for interaction
    if(this->IsClicked()) this->SetInput(true);
    if(m_bInput)
    {
        if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(ESCAPE), CORE_INPUT_PRESS))
        {
            // cancel and reset text-input
            m_sText = m_sPrevious;
            this->SetInput(false);
        }
        else if(Core::Input->GetMouseButton(CORE_INPUT_LEFT, CORE_INPUT_PRESS) && !this->IsFocused())
        {
            // cancel text-input
            this->SetInput(false);
        }
        else if(this->__Write())
        {
            // finish text-input
            m_bReturned = true;
            this->SetInput(false);
        }
    }
    else m_bReturned = false;

    // update the caption
    if(m_bDisplay)
    {
        if(m_cReplace)
        {
            // hide text with replacement character
            const coreString sPassword(m_sText.length(), m_cReplace);
            m_pCaption->SetText(m_bInput ? PRINT("%s%c", sPassword.c_str(), m_cCursor) : sPassword.c_str());
        }
        else m_pCaption->SetText(m_bInput ? PRINT("%s%c", m_sText.c_str(), m_cCursor) : m_sText.c_str());

        m_bDisplay = false;
    }
}


// ****************************************************************
/* set text-input status */
void coreTextBox::SetInput(const coreBool bInput)
{
    if(m_bInput == bInput) return;

    // save new text-input status
    m_bInput   = bInput;
    m_bDisplay = true;

    if(m_bInput)
    {
        // start text-input
        if(++s_iActiveCounter == 1) SDL_StartTextInput();
        m_sPrevious = m_sText;
    }
    else
    {
        // stop text-input
        if(--s_iActiveCounter == 0) SDL_StopTextInput();
    }
    ASSERT(s_iActiveCounter >= 0)
}


// ****************************************************************
/* process new text-input characters */
coreBool coreTextBox::__Write()
{
    ASSERT(m_iLength)

    // get new text-input character
    const coreInputChar& iChar = Core::Input->GetKeyboardChar();
    if(iChar)
    {
        if(iChar == CORE_INPUT_CHAR(RETURN))
        {
            // finish the text-input
            return true;
        }
        else if(iChar == CORE_INPUT_CHAR(BACKSPACE))
        {
            // remove last character
            if(!m_sText.empty()) m_sText.pop_back();
        }
        else if(iChar == CORE_INPUT_CHAR(PASTE))
        {
            // paste text from clipboard
            coreChar* pcPaste = SDL_GetClipboardText();
            if(pcPaste)
            {
                const coreUintW iLen = MIN(std::strlen(pcPaste), m_iLength - m_sText.length());

                // append and clamp to remaining string space
                m_sText.append(pcPaste, iLen);
                SDL_free(pcPaste);
            }
        }
        else if(m_sText.length() < m_iLength)
        {
            // append new character
            m_sText.append(1u, coreChar(iChar));
        }

        m_bDisplay = true;
    }

    return false;
}