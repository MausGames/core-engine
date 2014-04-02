//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#define CORE_INPUT_PRESS(x)   {(x)[3] = false;  (x)[2] = !(x)[1]; (x)[1] = true;}
#define CORE_INPUT_RELEASE(x) {(x)[3] = (x)[1]; (x)[2] = false;   (x)[1] = false;}


// ****************************************************************
// constructor
CoreInput::coreKeyboard::coreKeyboard()noexcept
: iLast (SDL_SCANCODE_UNKNOWN)
, cChar ('\0')
{
    std::memset(aabButton, 0, sizeof(aabButton));
}


// ****************************************************************
// constructor
CoreInput::coreMouse::coreMouse()noexcept
: iLast     (0)
, vPosition (coreVector2(0.0f,0.0f))
, vRelative (coreVector3(0.0f,0.0f,0.0f))
{
    std::memset(aabButton, 0, sizeof(aabButton));
}


// ****************************************************************
// constructor
CoreInput::coreJoystick::coreJoystick()noexcept
: pHandle   (NULL)
, iLast     (0)
, vRelative (coreVector2(0.0f,0.0f))
{
    std::memset(aabButton, 0, sizeof(aabButton));
}


// ****************************************************************
// constructor
CoreInput::CoreInput()noexcept
: m_pCursor        (NULL)
, m_bCursorVisible (true)
{
    Core::Log->Header("Input Interface");

    // check for joystick devices
    const int iNumJoysticks = SDL_NumJoysticks();
    if(iNumJoysticks)
    {
        Core::Log->ListStart("Joysticks/Gamepads found");
        for(int i = 0; i < iNumJoysticks; ++i)
        {
            coreJoystick Joystick;

            // open joystick device
            Joystick.pHandle = SDL_JoystickOpen(i);
            m_aJoystick.push_back(Joystick);

            Core::Log->ListEntry("%s", this->GetJoystickName(i));
        }
        Core::Log->ListEnd();
    }
    else Core::Log->Info("No joysticks/gamepads found");

    // append empty joystick device
    m_aJoystick.push_back(coreJoystick());
}


// ****************************************************************
// destructor
CoreInput::~CoreInput()
{
    Core::Log->Info("Input Interface shut down");

    // close all joystick devices
    FOR_EACH(it, m_aJoystick)
    {
        if(it->pHandle)
            SDL_JoystickClose(it->pHandle);
    }
    m_aJoystick.clear();

    // free the hardware mouse cursor
    SDL_FreeCursor(m_pCursor);
}


// ****************************************************************
// set the cursor object
void CoreInput::DefineCursor(const char* pcPath)
{
    coreFile* pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // load texture from file
    SDL_Surface* pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pFile->GetPath()));
    if(pData)
    {
        if(m_pCursor) SDL_FreeCursor(m_pCursor);

        // create hardware mouse cursor
        m_pCursor = SDL_CreateColorCursor(pData, 0, 0);
        SDL_SetCursor(m_pCursor);

        // free the texture
        SDL_FreeSurface(pData);
    }
}


// ****************************************************************
// show or hide the mouse cursor
// TODO: handle unsupported RelativeMouseMode, SDL_HINT_MOUSE_RELATIVE_MODE_WARP
void CoreInput::ShowCursor(const bool& bStatus)
{
    if(m_bCursorVisible == bStatus) return;

    // toggle cursor visibility
    SDL_ShowCursor(bStatus ? 1 : 0);
    SDL_SetRelativeMouseMode(bStatus ? SDL_FALSE : SDL_TRUE);

    // save visibility status
    m_bCursorVisible = bStatus;
}


// ****************************************************************
// control mouse with keyboard
void CoreInput::UseMouseWithKeyboard(const SDL_Scancode& iLeft, const SDL_Scancode& iRight, const SDL_Scancode& iUp, const SDL_Scancode& iDown, const SDL_Scancode& iButton1, const SDL_Scancode& iButton2, const float& fSpeed)
{
    // TODO: implement function
}


// ****************************************************************
// control mouse with joystick
void CoreInput::UseMouseWithJoystick(const coreUint& iID, const int& iButton1, const int& iButton2, const float& fSpeed)
{
    if(iID >= m_aJoystick.size()) return;

    // move the mouse cursor
    const coreVector2 vAcc = m_aJoystick[iID].vRelative.Normalized();
    if(vAcc.x || vAcc.y)
    {
        const coreVector2 vPos = this->GetMousePosition()*coreVector2(0.5f,-0.5f);
        const coreVector2 vNew = (vAcc*coreVector2(1.0f,-Core::System->GetResolution().AspectRatio()) * Core::System->GetTime()*fSpeed + vPos) * Core::System->GetResolution();
        SDL_WarpMouseInWindow(Core::System->GetWindow(), int(vNew.x), int(vNew.y));
    }

    // press mouse buttons
    if(this->GetJoystickButton(iID, iButton1, CORE_INPUT_PRESS))   this->SetMouseButton(1, true);
    if(this->GetJoystickButton(iID, iButton1, CORE_INPUT_RELEASE)) this->SetMouseButton(1, false);
    if(this->GetJoystickButton(iID, iButton2, CORE_INPUT_PRESS))   this->SetMouseButton(2, true);
    if(this->GetJoystickButton(iID, iButton2, CORE_INPUT_RELEASE)) this->SetMouseButton(2, false);
}


// ****************************************************************
// process input events
bool CoreInput::ProcessEvent(const SDL_Event& Event)
{
    switch(Event.type)
    {
    // set text-input character
    case SDL_TEXTINPUT:
        Core::Input->SetKeyboardChar((char)Event.text.text[0]);
        break;

    // press keyboard button
    case SDL_KEYDOWN:
        this->SetKeyboardButton(Event.key.keysym.scancode, true);
             if(Event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE)   this->SetKeyboardChar(SDLK_BACKSPACE);
        else if(Event.key.keysym.scancode == SDL_SCANCODE_RETURN)      this->SetKeyboardChar(SDLK_RETURN);
        else if(Event.key.keysym.scancode == SDL_SCANCODE_KP_ENTER)    this->SetKeyboardChar(SDLK_RETURN);
        else if(Event.key.keysym.scancode == SDL_SCANCODE_PRINTSCREEN) return false;   
        break;

    // release keyboard button
    case SDL_KEYUP:
        this->SetKeyboardButton(Event.key.keysym.scancode, false);
        break;

    // press mouse button
    case SDL_MOUSEBUTTONDOWN:
        this->SetMouseButton(Event.button.button, true);
        break;

    // release mouse button
    case SDL_MOUSEBUTTONUP:
        this->SetMouseButton(Event.button.button, false);
        break;

    // move mouse position
    case SDL_MOUSEMOTION:
        if(Event.motion.x != int(0.5f*Core::System->GetResolution().x) || 
           Event.motion.y != int(0.5f*Core::System->GetResolution().y))
        {
            this->SetMousePosition(coreVector2(float(Event.motion.x),    -float(Event.motion.y))   /Core::System->GetResolution() + coreVector2(-0.5f, 0.5f));
            this->SetMouseRelative(coreVector2(float(Event.motion.xrel), -float(Event.motion.yrel))/Core::System->GetResolution() + this->GetMouseRelative().xy());
        }
        break;

    // move mouse wheel
    case SDL_MOUSEWHEEL:
        this->SetMouseWheel((float)Event.wheel.y);
        break;

    // press joystick button
    case SDL_JOYBUTTONDOWN:
        this->SetJoystickButton(Event.jbutton.which, Event.jbutton.button, true);
        break;

    // release joystick button
    case SDL_JOYBUTTONUP:
        this->SetJoystickButton(Event.jbutton.which, Event.jbutton.button, false);
        break;

    // move joystick axis
    case SDL_JOYAXISMOTION:
        if(ABS(Event.jaxis.value) > 8000) this->SetJoystickRelative(Event.jbutton.which, Event.jaxis.axis, float(coreMath::Sign(Event.jaxis.value)) * (Event.jaxis.axis ? -1.0f : 1.0f));
                                     else this->SetJoystickRelative(Event.jbutton.which, Event.jaxis.axis, 0.0f);
        break;
    }

    return true;
}


// ****************************************************************
// update the input button interface
void CoreInput::__UpdateButtons()
{
    // process keyboard inputs
    for(int i = 0; i < CORE_INPUT_BUTTONS_KEYBOARD; ++i)
    {
             if( m_Keyboard.aabButton[i][0]) CORE_INPUT_PRESS(m_Keyboard.aabButton[i])
        else if(!m_Keyboard.aabButton[i][0]) CORE_INPUT_RELEASE(m_Keyboard.aabButton[i])
    }

    // process mouse inputs
    for(int i = 0; i < CORE_INPUT_BUTTONS_MOUSE; ++i)
    {
             if( m_Mouse.aabButton[i][0]) CORE_INPUT_PRESS(m_Mouse.aabButton[i])
        else if(!m_Mouse.aabButton[i][0]) CORE_INPUT_RELEASE(m_Mouse.aabButton[i])
    }

    // process joystick inputs
    FOR_EACH(it, m_aJoystick)
    {
        for(int i = 0; i < CORE_INPUT_BUTTONS_JOYSTICK; ++i)
        {
                 if( it->aabButton[i][0]) CORE_INPUT_PRESS(it->aabButton[i])
            else if(!it->aabButton[i][0]) CORE_INPUT_RELEASE(it->aabButton[i])
        }
    }

#if defined(_CORE_LINUX_)

    if(!m_bCursorVisible)
    {
        // hold cursor in window center when not visible
        SDL_WarpMouseInWindow(Core::System->GetWindow(), int(0.5f*Core::System->GetResolution().x), 
                                                         int(0.5f*Core::System->GetResolution().y));
    }

#endif
}


// ****************************************************************
// clear the input button interface
void CoreInput::__ClearButtons()
{
    // clear all last pressed buttons
    m_Keyboard.iLast = SDL_SCANCODE_UNKNOWN;
    m_Mouse.iLast    = -1;
    FOR_EACH(it, m_aJoystick)
        it->iLast = -1;

    // clear current text-input character
    m_Keyboard.cChar = '\0';

    // reset relative movement of the mouse cursor
    m_Mouse.vRelative = coreVector3(0.0f,0.0f,0.0f);
}