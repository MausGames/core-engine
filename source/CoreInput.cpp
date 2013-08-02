#include "Core.h"

// ****************************************************************    
// constructor
CoreInput::CoreInput()
: m_pCursor        (NULL)
, m_bCursorVisible (true)
, m_bActive        (true)
{
    Core::Log->Header("Input Interface");

    // reset memory
    ZeroMemory(&m_Keyboard, sizeof(coreKeyboard));
    ZeroMemory(&m_Mouse, sizeof(coreMouse));

    // open joystick objects
    const int iNumJoysticks = SDL_NumJoysticks();
    if(iNumJoysticks)
    {
        Core::Log->ListStart("Joysticks found");
        for(int i = 0; i < iNumJoysticks; ++i)
        {
            coreJoystick Joystick;

            // open joystick device
            ZeroMemory(&Joystick, sizeof(coreJoystick));
            Joystick.pHandle = SDL_JoystickOpen(i);
            m_aJoystick.push_back(Joystick);

            Core::Log->ListEntry(coreUtils::Print("%s", this->GetJoystickName(i)));
        }
        Core::Log->ListEnd();
    }
    else Core::Log->Info("No joysticks found");
}


// ****************************************************************
// destructor
CoreInput::~CoreInput()
{
    Core::Log->Info("Input Interface Shut Down");

    // close joystick devices
    for(coreUint i = 0; i < m_aJoystick.size(); ++i)
        SDL_JoystickClose(m_aJoystick[i].pHandle);
    m_aJoystick.clear();

    // delete graphical mouse cursor
    // TODO: implement with coreObject2D
    //SAFE_DELETE(m_pCursor)
}


// ****************************************************************
// update the input interface
void CoreInput::__UpdateInput()
{
    // process keyboard input
    for(int i = 0; i < CORE_INPUT_KEYBOARD_BUTTONS; ++i)
    {
             if( m_Keyboard.aabButton[i][0]) CORE_INPUT_PRESS(m_Keyboard.aabButton[i])
        else if(!m_Keyboard.aabButton[i][0]) CORE_INPUT_RELEASE(m_Keyboard.aabButton[i])
    }

    // process mouse input
    for(int i = 0; i < CORE_INPUT_MOUSE_BUTTONS; ++i)
    {
             if( m_Mouse.aabButton[i][0]) CORE_INPUT_PRESS(m_Mouse.aabButton[i])
        else if(!m_Mouse.aabButton[i][0]) CORE_INPUT_RELEASE(m_Mouse.aabButton[i])
    }

    // process joystick input
    for(coreUint j = 0; j < m_aJoystick.size(); ++j)
    {
        for(int i = 0; i < CORE_INPUT_JOYSTICK_BUTTONS; ++i)
        {
                 if( m_aJoystick[j].aabButton[i][0]) CORE_INPUT_PRESS(m_aJoystick[j].aabButton[i])
            else if(!m_aJoystick[j].aabButton[i][0]) CORE_INPUT_RELEASE(m_aJoystick[j].aabButton[i])
        }

        // process latest joystick input button
        m_aJoystick[j].aiLast[0] = m_aJoystick[j].aiLast[1];
        m_aJoystick[j].aiLast[1] = -1;
    }

    // process latest keyboard input button
    m_Keyboard.aiLast[0] = m_Keyboard.aiLast[1];
    m_Keyboard.aiLast[1] = SDL_SCANCODE_UNKNOWN;

    // process latest mouse input button
    m_Mouse.aiLast[0] = m_Mouse.aiLast[1];
    m_Mouse.aiLast[1] = -1;

    // process current keyboard/textinput character
    m_Keyboard.acChar[0] = m_Keyboard.acChar[1];
    m_Keyboard.acChar[1] = (char)NULL;
}


// ****************************************************************
// update the graphical mouse cursor
void CoreInput::__UpdateCursor()
{
    // TODO: implement with coreObject2D
    if(!m_bCursorVisible) return;

    // TODO: implement with coreObject2D
    // update cursor object
    //m_pCursor->SetPosition(m_pCursor->GetSize()*coreVector2(0.5f,-0.5f));
    //m_pCursor->SetCenter(m_Mouse.vPosition);
    //m_pCursor->Move();

    //m_pCursor->Render();
}


// ****************************************************************
// set mouse cursor visibility
void CoreInput::ShowCursor(const bool& bStatus)
{
    // #TODO: handle unsupported RelativeMouseMode

    // Anzeige umschalten
    SDL_ShowCursor(m_pCursor ? SDL_DISABLE : bStatus);
    SDL_SetRelativeMouseMode(bStatus ? SDL_FALSE : SDL_TRUE);

    // Status speichern
    m_bCursorVisible = bStatus;
}


// ****************************************************************
// set the graphical mouse cursor
void CoreInput::SetCursorObject(const char* pcColorMap, const char* pcAlphaMap, const coreVector2& vSize)
{
    // TODO: implement with coreObject2D
    // create object
    //m_pCursor = new coreObject2D();
    //m_pCursor->DefineAppearance(pcColorMap, pcAlphaMap, NULL, true);
    //m_pCursor->SetSize(vSize);

    // switch visiblity
    this->ShowCursor(m_bCursorVisible);
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

    // overload active status
    const bool bActive = m_bActive;
    this->SetActive(true);

    // move the mouse cursor
    const coreVector2 vAcc = m_aJoystick[iID].vRelative.Normalized();
    if(vAcc.x || vAcc.y)
    {
        const coreVector2 vPos = this->GetMousePosition()*coreVector2(0.5f,-0.5f);
        const coreVector2 vNew = vAcc*coreVector2(1.0f,-Core::System->GetResolution().AspectRatio()) * Core::System->GetTime()*fSpeed + vPos;
        SDL_WarpMouseInWindow(Core::System->GetWindow(), int(vNew.x*Core::System->GetResolution().x), int(vNew.y*Core::System->GetResolution().y)); 
    }

    // press the mouse buttons
    if(this->GetJoystickButton(iID, iButton1, CORE_INPUT_PRESS))   this->SetMouseButton(1, true);
    if(this->GetJoystickButton(iID, iButton1, CORE_INPUT_RELEASE)) this->SetMouseButton(1, false);
    if(this->GetJoystickButton(iID, iButton2, CORE_INPUT_PRESS))   this->SetMouseButton(2, true);
    if(this->GetJoystickButton(iID, iButton2, CORE_INPUT_RELEASE)) this->SetMouseButton(2, false);

    this->SetActive(bActive);
}