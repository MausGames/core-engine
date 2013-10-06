//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#define CORE_INPUT_PRESS(x)   {(x)[3] = false;  (x)[2] = !(x)[1]; (x)[1] = true;}
#define CORE_INPUT_RELEASE(x) {(x)[3] = (x)[1]; (x)[2] = false;   (x)[1] = false;}


// ****************************************************************
// constructor
CoreInput::CoreInput()noexcept
: m_pCursor        (NULL)
, m_bCursorVisible (true)
{
    Core::Log->Header("Input Interface");

    // reset memory
    memset(&m_Keyboard, 0, sizeof(coreKeyboard));
    memset(&m_Mouse,    0, sizeof(coreMouse));

    // check for joystick devices
    const int iNumJoysticks = SDL_NumJoysticks();
    if(iNumJoysticks)
    {
        Core::Log->ListStart("Joysticks found");
        for(int i = 0; i < iNumJoysticks; ++i)
        {
            coreJoystick Joystick;

            // open joystick device
            memset(&Joystick, 0, sizeof(coreJoystick));
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
    Core::Log->Info("Input Interface shut down");

    // close all joystick devices
    for(coreUint i = 0; i < m_aJoystick.size(); ++i)
        SDL_JoystickClose(m_aJoystick[i].pHandle);
    m_aJoystick.clear();

    // delete the cursor object
    // TODO: implement with coreObject2D
    //SAFE_DELETE(m_pCursor)
}


// ****************************************************************
// update the input interface
void CoreInput::__UpdateInput()
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
    for(coreUint j = 0; j < m_aJoystick.size(); ++j)
    {
        for(int i = 0; i < CORE_INPUT_BUTTONS_JOYSTICK; ++i)
        {
                 if( m_aJoystick[j].aabButton[i][0]) CORE_INPUT_PRESS(m_aJoystick[j].aabButton[i])
            else if(!m_aJoystick[j].aabButton[i][0]) CORE_INPUT_RELEASE(m_aJoystick[j].aabButton[i])
        }

        // forward last joystick input button
        m_aJoystick[j].aiLast[0] = m_aJoystick[j].aiLast[1];
        m_aJoystick[j].aiLast[1] = -1;
    }

    // forward last keyboard input button
    m_Keyboard.aiLast[0] = m_Keyboard.aiLast[1];
    m_Keyboard.aiLast[1] = SDL_SCANCODE_UNKNOWN;

    // forward last mouse input button
    m_Mouse.aiLast[0] = m_Mouse.aiLast[1];
    m_Mouse.aiLast[1] = -1;

    // forward current textinput character
    m_Keyboard.acChar[0] = m_Keyboard.acChar[1];
    m_Keyboard.acChar[1] = '\0';
}


// ****************************************************************
// update the cursor object
// TODO: implement with coreObject2D
void CoreInput::__UpdateCursor()
{
    if(!m_bCursorVisible) return;

    // move and render the cursor object
    //m_pCursor->SetCenter(m_Mouse.vPosition);
    //m_pCursor->Move();
    //m_pCursor->Render();
}


// ****************************************************************
// set the cursor object
// TODO: implement with coreObject2D
void CoreInput::SetCursorObject(const char* pcColorMap, const char* pcAlphaMap, const coreVector2& vSize)
{
    //if(m_pCursor) SAFE_DELETE(m_pCursor)

    // create the cursor object
    //m_pCursor = new coreObject2D();
    //m_pCursor->DefineAppearance(pcColorMap, pcAlphaMap, NULL, true);
    //m_pCursor->SetPosition(vSize*coreVector2(0.5f,-0.5f));
    //m_pCursor->SetSize(vSize);

    // update visibility
    this->ShowCursor(m_bCursorVisible);
}


// ****************************************************************
// show or hide the mouse cursor
// TODO: handle unsupported RelativeMouseMode
void CoreInput::ShowCursor(const bool& bStatus)
{
    if(m_bCursorVisible == bStatus) return;

    // toggle cursor visibility
    SDL_ShowCursor(m_pCursor ? 0 : (bStatus ? 1 : 0));
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