///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
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

        const coreVector2 vLabelPosition = m_pCaption->GetScreenPosition();
        const coreVector2 vLabelBound    = m_pCaption->GetScreenBound90()      * 0.5f;
        const coreVector4 vResHalf       = Core::Graphics->GetViewResolution() * 0.5f;

        // create rectangle around label
        SDL_Rect oRect;
        oRect.x = F_TO_SI(vResHalf.x + vLabelPosition.x);
        oRect.y = F_TO_SI(vResHalf.y - vLabelPosition.y);
        oRect.w = F_TO_SI(vLabelBound.x);
        oRect.h = F_TO_SI(vLabelBound.y);

        // set text input area
        SDL_SetTextInputArea(Core::System->GetWindow(), &oRect, F_TO_SI(vLabelBound.x));
    }
    else m_bReturned = false;

    // update the caption
    if(m_bDisplay)
    {
        if(m_cReplace)
        {
            // hide text with replacement character
            const coreString sPassword(SDL_utf8strlen(m_sText.c_str()), m_cReplace);
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
        if(++s_iActiveCounter == 1)
        {
            // set function properties
            coreProperties oProps;
            SDL_SetNumberProperty (oProps, SDL_PROP_TEXTINPUT_TYPE_NUMBER,           m_cReplace ? SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN : SDL_TEXTINPUT_TYPE_TEXT);
            SDL_SetNumberProperty (oProps, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_NONE);
            SDL_SetBooleanProperty(oProps, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN,   false);
            SDL_SetBooleanProperty(oProps, SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN,     false);

            // start text-input
            SDL_StartTextInputWithProperties(Core::System->GetWindow(), oProps);
        }
        m_sPrevious = m_sText;
    }
    else
    {
        if(--s_iActiveCounter == 0)
        {
            // stop text-input
            SDL_StopTextInput(Core::System->GetWindow());
        }
    }
    ASSERT(s_iActiveCounter >= 0)
}


// ****************************************************************
/* process new text-input characters */
coreBool coreTextBox::__Write()
{
    ASSERT(m_iLength)

    // get new text-input character
    const coreInputChar iChar = Core::Input->GetKeyboardChar();
    if(iChar)
    {
        if((iChar == CORE_INPUT_CHAR(RETURN)) || (iChar == CORE_INPUT_CHAR(KP_ENTER)))
        {
            // finish text-input
            return true;
        }
        else if(iChar == CORE_INPUT_CHAR(BACKSPACE))
        {
            // remove last character
            if(!m_sText.empty())
            {
                if(HAS_FLAG(m_sText.back(), 0x80u))
                {
                    // handle UTF-8 encoding
                    while(!HAS_FLAG(m_sText.back(), 0xC0u)) m_sText.pop_back();
                }
                m_sText.pop_back();
            }
        }
        else if(iChar == CORE_INPUT_CHAR(CUT))
        {
            // move text to clipboard
            if(!m_sText.empty()) SDL_SetClipboardText(m_sText.c_str());
            m_sText.clear();
        }
        else if(iChar == CORE_INPUT_CHAR(COPY))
        {
            // copy text to clipboard
            if(!m_sText.empty()) SDL_SetClipboardText(m_sText.c_str());
        }
        else if(iChar == CORE_INPUT_CHAR(PASTE))
        {
            // paste text from clipboard
            coreChar* pcPaste = SDL_GetClipboardText();
            if(pcPaste)
            {
                const coreUintW iLen = std::strlen(pcPaste);

                // append text
                if(iLen <= m_iLength - m_sText.length()) m_sText.append(pcPaste, iLen);
                SDL_free(pcPaste);
            }
        }
        else
        {
            const coreChar* pcText = Core::Input->GetKeyboardCharText();
            const coreUintW iLen   = std::strlen(pcText);

            // append new characters
            if(iLen <= m_iLength - m_sText.length()) m_sText.append(pcText, iLen);
        }

        m_bDisplay = true;
    }

    return false;
}