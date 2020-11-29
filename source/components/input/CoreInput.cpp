///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
CoreInput::CoreInput()noexcept
: m_Keyboard       {}
, m_Mouse          {}
, m_aJoystick      {}
, m_aTouch         {{}}
, m_aiTouchCount   {}
, m_pCursor        (NULL)
, m_bCursorVisible (true)
, m_iAnyButton     (0u)
{
    Core::Log->Header("Input Interface");

    // start up joystick input
    this->__OpenJoysticks();
}


// ****************************************************************
/* destructor */
CoreInput::~CoreInput()
{
    // shut down joystick input
    this->__CloseJoysticks();

    // delete mouse cursor
    if(m_pCursor) SDL_FreeCursor(m_pCursor);

    Core::Log->Info(CORE_LOG_BOLD("Input Interface shut down"));
}


// ****************************************************************
/* set the mouse cursor */
void CoreInput::SetCursor(const coreChar* pcPath)
{
    // retrieve texture file
    coreFileScope pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // decompress file to plain pixel data
    coreSurfaceScope pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pcPath));
    if(!pData)
    {
        Core::Log->Warning("Cursor (%s) could not be loaded (SDL: %s)", pcPath, SDL_GetError());
        return;
    }

    // delete old mouse cursor
    if(m_pCursor) SDL_FreeCursor(m_pCursor);

    // create and set new mouse cursor
    m_pCursor = SDL_CreateColorCursor(pData, 0, 0);
    SDL_SetCursor(m_pCursor);

    Core::Log->Info("Cursor (%s) loaded", pcPath);
}


// ****************************************************************
/* show or hide the mouse cursor */
void CoreInput::ShowCursor(const coreBool bStatus)
{
    if(m_bCursorVisible == bStatus) return;

    // toggle cursor visibility
    SDL_ShowCursor(bStatus ? 1 : 0);
    SDL_SetRelativeMouseMode(bStatus ? SDL_FALSE : SDL_TRUE);

    // save visibility status
    m_bCursorVisible = bStatus;
}


// ****************************************************************
/* control mouse with keyboard */
void CoreInput::UseMouseWithKeyboard(const coreInputKey iLeft, const coreInputKey iRight, const coreInputKey iDown, const coreInputKey iUp, const coreInputKey iButton1, const coreInputKey iButton2, const coreFloat fSpeed)
{
    // get original input
    coreVector2 vAcc = coreVector2(0.0f,0.0f);
         if(this->GetKeyboardButton(iLeft,  CORE_INPUT_PRESS)) vAcc.x = -1.0f;
    else if(this->GetKeyboardButton(iRight, CORE_INPUT_PRESS)) vAcc.x =  1.0f;
         if(this->GetKeyboardButton(iDown,  CORE_INPUT_PRESS)) vAcc.y = -1.0f;
    else if(this->GetKeyboardButton(iUp,    CORE_INPUT_PRESS)) vAcc.y =  1.0f;

    // move the mouse cursor
    if(!vAcc.IsNull())
    {
        const coreVector2 vPos = this->GetMousePosition() + coreVector2(0.5f,-0.5f);
        const coreVector2 vNew = (vAcc.Normalized() * Core::System->GetResolution().yx() * (RCP(Core::System->GetResolution().Min()) * Core::System->GetTime() * fSpeed) + vPos) * Core::System->GetResolution();
        SDL_WarpMouseInWindow(Core::System->GetWindow(), F_TO_SI(vNew.x + 0.5f), F_TO_SI(-vNew.y + 0.5f));
    }

    // press mouse buttons
    if(this->GetKeyboardButton(iButton1, CORE_INPUT_PRESS))   this->SetMouseButton(CORE_INPUT_LEFT,  true);
    if(this->GetKeyboardButton(iButton1, CORE_INPUT_RELEASE)) this->SetMouseButton(CORE_INPUT_LEFT,  false);
    if(this->GetKeyboardButton(iButton2, CORE_INPUT_PRESS))   this->SetMouseButton(CORE_INPUT_RIGHT, true);
    if(this->GetKeyboardButton(iButton2, CORE_INPUT_RELEASE)) this->SetMouseButton(CORE_INPUT_RIGHT, false);
}


// ****************************************************************
/* control mouse with joystick */
void CoreInput::UseMouseWithJoystick(const coreUintW iIndex, const coreUint8 iButton1, const coreUint8 iButton2, const coreFloat fSpeed)
{
    WARN_IF(iIndex >= m_aJoystick.size()) return;

    // get original input
    const coreVector2 vAcc = this->GetJoystickRelativeL(iIndex);

    // move the mouse cursor
    if(!vAcc.IsNull())
    {
        const coreVector2 vPos = this->GetMousePosition() + coreVector2(0.5f,-0.5f);
        const coreVector2 vNew = (vAcc.Normalized() * Core::System->GetResolution().yx() * (RCP(Core::System->GetResolution().Min()) * Core::System->GetTime() * fSpeed) + vPos) * Core::System->GetResolution();
        SDL_WarpMouseInWindow(Core::System->GetWindow(), F_TO_SI(vNew.x + 0.5f), F_TO_SI(-vNew.y + 0.5f));
    }

    // press mouse buttons
    if(this->GetJoystickButton(iIndex, iButton1, CORE_INPUT_PRESS))   {this->SetMouseButton(CORE_INPUT_LEFT,  true); this->RumbleJoystick(iIndex, 0.2f, 200u);}
    if(this->GetJoystickButton(iIndex, iButton1, CORE_INPUT_RELEASE)) {this->SetMouseButton(CORE_INPUT_LEFT,  false);}
    if(this->GetJoystickButton(iIndex, iButton2, CORE_INPUT_PRESS))   {this->SetMouseButton(CORE_INPUT_RIGHT, true); this->RumbleJoystick(iIndex, 0.2f, 200u);}
    if(this->GetJoystickButton(iIndex, iButton2, CORE_INPUT_RELEASE)) {this->SetMouseButton(CORE_INPUT_RIGHT, false);}
}


// ****************************************************************
/* forward hat input to stick input on joystick */
void CoreInput::ForwardHatToStick(const coreUintW iIndex)
{
    WARN_IF(iIndex >= m_aJoystick.size()) return;

    // check for hat directions and invoke stick movement
         if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_LEFT,  CORE_INPUT_HOLD)) this->SetJoystickRelative(iIndex, 0u, -1.0f);
    else if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_RIGHT, CORE_INPUT_HOLD)) this->SetJoystickRelative(iIndex, 0u,  1.0f);
         if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_DOWN,  CORE_INPUT_HOLD)) this->SetJoystickRelative(iIndex, 1u, -1.0f);
    else if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_UP,    CORE_INPUT_HOLD)) this->SetJoystickRelative(iIndex, 1u,  1.0f);

    // reset stick movement on release
    if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_LEFT,  CORE_INPUT_RELEASE) ||
       this->GetJoystickHat(iIndex, CORE_INPUT_DIR_RIGHT, CORE_INPUT_RELEASE)) this->SetJoystickRelative(iIndex, 0u, 0.0f);
    if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_DOWN,  CORE_INPUT_RELEASE) ||
       this->GetJoystickHat(iIndex, CORE_INPUT_DIR_UP,    CORE_INPUT_RELEASE)) this->SetJoystickRelative(iIndex, 1u, 0.0f);
}


// ****************************************************************
/* process input events */
coreBool CoreInput::ProcessEvent(const SDL_Event& oEvent)
{
    switch(oEvent.type)
    {
    // set text-input character
    case SDL_TEXTINPUT:
        this->SetKeyboardChar(oEvent.text.text[0]);
        break;

    // press keyboard button
    case SDL_KEYDOWN:
        this->SetKeyboardButton(oEvent.key.keysym.scancode, true);
             if(oEvent.key.keysym.scancode == CORE_INPUT_KEY(BACKSPACE)) this->SetKeyboardChar(CORE_INPUT_CHAR(BACKSPACE));
        else if(oEvent.key.keysym.scancode == CORE_INPUT_KEY(RETURN))    this->SetKeyboardChar(CORE_INPUT_CHAR(RETURN));
        else if(oEvent.key.keysym.scancode == CORE_INPUT_KEY(KP_ENTER))  this->SetKeyboardChar(CORE_INPUT_CHAR(RETURN));
        else if(oEvent.key.keysym.mod & KMOD_CTRL)
        {
                 if(oEvent.key.keysym.scancode == CORE_INPUT_KEY(X)) this->SetKeyboardChar(CORE_INPUT_CHAR(CUT));
            else if(oEvent.key.keysym.scancode == CORE_INPUT_KEY(C)) this->SetKeyboardChar(CORE_INPUT_CHAR(COPY));
            else if(oEvent.key.keysym.scancode == CORE_INPUT_KEY(V)) this->SetKeyboardChar(CORE_INPUT_CHAR(PASTE));
        }
        else if(oEvent.key.keysym.scancode == CORE_INPUT_KEY(PRINTSCREEN)) return false;
        break;

    // release keyboard button
    case SDL_KEYUP:
        this->SetKeyboardButton(oEvent.key.keysym.scancode, false);
        break;

#if !defined(_CORE_MOBILE_)

    // press mouse button
    case SDL_MOUSEBUTTONDOWN:
        this->SetMouseButton(oEvent.button.button, true);
        break;

    // release mouse button
    case SDL_MOUSEBUTTONUP:
        this->SetMouseButton(oEvent.button.button, false);
        break;

    // move mouse position
    case SDL_MOUSEMOTION:
        this->SetMousePosition(coreVector2(I_TO_F(oEvent.motion.x),    -I_TO_F(oEvent.motion.y))   /Core::System->GetResolution() + coreVector2(-0.5f,0.5f));
        this->SetMouseRelative(coreVector2(I_TO_F(oEvent.motion.xrel), -I_TO_F(oEvent.motion.yrel))/Core::System->GetResolution() + this->GetMouseRelative().xy());
        break;

    // move mouse wheel
    case SDL_MOUSEWHEEL:
        this->SetMouseWheel(I_TO_F(oEvent.wheel.y));
        break;

#endif

    // press joystick button
    case SDL_JOYBUTTONDOWN:
        if(__CORE_INPUT_JOYSTICK(this->__GetJoystickIndex(oEvent.jbutton.which)).pController) break; FALLTHROUGH
    case SDL_CONTROLLERBUTTONDOWN:
        this->SetJoystickButton(this->__GetJoystickIndex(oEvent.jbutton.which), oEvent.jbutton.button, true);
        break;

    // release joystick button
    case SDL_JOYBUTTONUP:
        if(__CORE_INPUT_JOYSTICK(this->__GetJoystickIndex(oEvent.jbutton.which)).pController) break; FALLTHROUGH
    case SDL_CONTROLLERBUTTONUP:
        this->SetJoystickButton(this->__GetJoystickIndex(oEvent.jbutton.which), oEvent.jbutton.button, false);
        break;

    // move joystick axis
    case SDL_JOYAXISMOTION:
        if(__CORE_INPUT_JOYSTICK(this->__GetJoystickIndex(oEvent.jaxis.which)).pController) break; FALLTHROUGH
    case SDL_CONTROLLERAXISMOTION:
        if(oEvent.jaxis.axis < CORE_INPUT_AXIS)
        {
            if(ABS(coreInt32(oEvent.jaxis.value)) > Core::Config->GetInt(CORE_CONFIG_INPUT_JOYSTICKDEAD))
                 this->SetJoystickRelative(this->__GetJoystickIndex(oEvent.jaxis.which), oEvent.jaxis.axis, CLAMP(I_TO_F(oEvent.jaxis.value) * RCP(I_TO_F(MAX(Core::Config->GetInt(CORE_CONFIG_INPUT_JOYSTICKMAX), 1))) * (((oEvent.jaxis.axis == 1u) || (oEvent.jaxis.axis == 3u)) ? -1.0f : 1.0f), -1.0f, 1.0f));
            else this->SetJoystickRelative(this->__GetJoystickIndex(oEvent.jaxis.which), oEvent.jaxis.axis, 0.0f);
        }
        break;

    // move joystick hat
    case SDL_JOYHATMOTION:
        this->SetJoystickHat(this->__GetJoystickIndex(oEvent.jhat.which), CORE_INPUT_DIR_LEFT,  oEvent.jhat.value & SDL_HAT_LEFT);
        this->SetJoystickHat(this->__GetJoystickIndex(oEvent.jhat.which), CORE_INPUT_DIR_RIGHT, oEvent.jhat.value & SDL_HAT_RIGHT);
        this->SetJoystickHat(this->__GetJoystickIndex(oEvent.jhat.which), CORE_INPUT_DIR_DOWN,  oEvent.jhat.value & SDL_HAT_DOWN);
        this->SetJoystickHat(this->__GetJoystickIndex(oEvent.jhat.which), CORE_INPUT_DIR_UP,    oEvent.jhat.value & SDL_HAT_UP);
        break;

    // attach or detach joystick
    case SDL_JOYDEVICEADDED:
    case SDL_JOYDEVICEREMOVED:
        this->__CloseJoysticks();
        this->__OpenJoysticks();
        break;

#if defined(_CORE_MOBILE_)

    // press finger
    case SDL_FINGERDOWN:
        this->SetTouchButton  (coreUintW(oEvent.tfinger.fingerId), true);
        this->SetTouchPosition(coreUintW(oEvent.tfinger.fingerId), coreVector2(oEvent.tfinger.x, -oEvent.tfinger.y) + coreVector2(-0.5f,0.5f));
        break;

    // release finger
    case SDL_FINGERUP:
        this->SetTouchButton(coreUintW(oEvent.tfinger.fingerId), false);
        break;

    // move finger
    case SDL_FINGERMOTION:
        this->SetTouchPosition(coreUintW(oEvent.tfinger.fingerId), coreVector2(oEvent.tfinger.x,  -oEvent.tfinger.y)  + coreVector2(-0.5f,0.5f));
        this->SetTouchRelative(coreUintW(oEvent.tfinger.fingerId), coreVector2(oEvent.tfinger.dx, -oEvent.tfinger.dy) + this->GetTouchRelative(coreUintW(oEvent.tfinger.fingerId)));
        this->SetTouchPressure(coreUintW(oEvent.tfinger.fingerId), oEvent.tfinger.pressure);
        break;

#endif
    }

    return true;
}


// ****************************************************************
/* clear status of all input buttons */
void CoreInput::ClearButtonAll()
{
    // clear each available device
    this->ClearKeyboardButtonAll();
    this->ClearMouseButtonAll();
    for(coreUintW i = 0u, ie = this->GetJoystickNum(); i < ie; ++i) this->ClearJoystickButtonAll(i);
    this->ClearTouchButtonAll();

    // clear status of any available button
    this->ClearAnyButton();
}


// ****************************************************************
/* update the input button interface (start) */
void CoreInput::__UpdateButtonsStart()
{
    // process keyboard inputs
    for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_KEYBOARD; ++i)
    {
        if(CONTAINS_BIT(m_Keyboard.aiButton[i], CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (m_Keyboard.aiButton[i])
                                                             else __CORE_INPUT_RELEASE(m_Keyboard.aiButton[i])
        __CORE_INPUT_COUNT(m_Keyboard.aiButton[i], m_Keyboard.aiCount)
    }

#if !defined(_CORE_MOBILE_)

    // process mouse inputs
    for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_MOUSE; ++i)
    {
        if(CONTAINS_BIT(m_Mouse.aiButton[i], CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (m_Mouse.aiButton[i])
                                                          else __CORE_INPUT_RELEASE(m_Mouse.aiButton[i])
        __CORE_INPUT_COUNT(m_Mouse.aiButton[i], m_Mouse.aiCount)
    }

#endif

    // process joystick inputs
    FOR_EACH(it, m_aJoystick)
    {
        for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_JOYSTICK; ++i)
        {
            if(CONTAINS_BIT(it->aiButton[i], CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (it->aiButton[i])
                                                          else __CORE_INPUT_RELEASE(it->aiButton[i])
            __CORE_INPUT_COUNT(it->aiButton[i], it->aiCount)
        }

        for(coreUintW i = 0u; i < CORE_INPUT_DIRECTIONS; ++i)
        {
            if(CONTAINS_BIT(it->aiHat[i], CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (it->aiHat[i])
                                                       else __CORE_INPUT_RELEASE(it->aiHat[i])
        }
    }

#if defined(_CORE_MOBILE_)

    // process touch inputs
    for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
    {
        if(CONTAINS_BIT(m_aTouch[i].iButton, CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (m_aTouch[i].iButton)
                                                          else __CORE_INPUT_RELEASE(m_aTouch[i].iButton)
        __CORE_INPUT_COUNT(m_aTouch[i].iButton, m_aiTouchCount)
    }

#endif
}


// ****************************************************************
/* update the input button interface (end) */
void CoreInput::__UpdateButtonsEnd()
{
    // clear all numbers of input buttons with same status
    std::memset(m_Keyboard.aiCount, 0, sizeof(m_Keyboard.aiCount));
    std::memset(m_Mouse   .aiCount, 0, sizeof(m_Mouse   .aiCount));
    FOR_EACH(it, m_aJoystick) std::memset(it->aiCount, 0, sizeof(it->aiCount));
    std::memset(m_aiTouchCount,     0, sizeof(m_aiTouchCount));

    // clear all last pressed input buttons
    m_Keyboard.iLast = CORE_INPUT_INVALID_KEYBOARD;
    m_Mouse   .iLast = CORE_INPUT_INVALID_MOUSE;
    FOR_EACH(it, m_aJoystick) it->iLast = CORE_INPUT_INVALID_JOYSTICK;

    // clear all relative movements
    m_Mouse.vRelative = coreVector3(0.0f,0.0f,0.0f);
    for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
        m_aTouch[i].vRelative = coreVector2(0.0f,0.0f);

    // clear current text-input character
    m_Keyboard.iChar = CORE_INPUT_CHAR(UNKNOWN);

    // clear status of any available button
    this->ClearAnyButton();
}


// ****************************************************************
/* start up joystick input */
void CoreInput::__OpenJoysticks()
{
    ASSERT(m_aJoystick.empty())

    // check for available joystick devices
    const coreUintW iNumJoysticks = SDL_NumJoysticks();
    if(iNumJoysticks)
    {
        Core::Log->ListStartInfo("Joysticks and Gamepads found");
        {
            m_aJoystick.reserve(iNumJoysticks + 1u);
            for(coreUintW i = 0u; i < iNumJoysticks; ++i)
            {
                coreJoystick oJoystick = {};

                // open game controller and joystick device
                oJoystick.pController = SDL_GameControllerOpen(i);
                oJoystick.pJoystick   = oJoystick.pController ? SDL_GameControllerGetJoystick(oJoystick.pController) : SDL_JoystickOpen(i);

                // try to open haptic device
                oJoystick.pHaptic = SDL_HapticOpenFromJoystick(oJoystick.pJoystick);
                if(oJoystick.pHaptic)
                {
                    // check for simple rumble playback
                    if(SDL_HapticRumbleInit(oJoystick.pHaptic))
                    {
                        SDL_HapticClose(oJoystick.pHaptic);
                        oJoystick.pHaptic = NULL;
                    }
                }

                // get device type
                const SDL_GameControllerType eControllerType = SDL_GameControllerGetType(oJoystick.pController);
                const SDL_JoystickType       eJoystickType   = SDL_JoystickGetType      (oJoystick.pJoystick);

                // get device features
                const coreInt32  iNumButtons = SDL_JoystickNumButtons(oJoystick.pJoystick);
                const coreInt32  iNumAxes    = SDL_JoystickNumAxes   (oJoystick.pJoystick);
                const coreInt32  iNumHats    = SDL_JoystickNumHats   (oJoystick.pJoystick);
                const coreInt32  iNumBalls   = SDL_JoystickNumBalls  (oJoystick.pJoystick);
                const coreUint32 iQuery      = SDL_HapticQuery       (oJoystick.pHaptic);

                // save joystick object
                m_aJoystick.push_back(oJoystick);
                Core::Log->ListAdd(CORE_LOG_BOLD("%s:") " %s (%s, %d/%d, %d buttons, %d axes, %d hats, %d balls, %s 0x%04X)",
                                   oJoystick.pController ? "Gamepad" : "Joystick", this->GetJoystickName(i), this->GetJoystickGUID(i),
                                   eControllerType, eJoystickType, iNumButtons, iNumAxes, iNumHats, iNumBalls, oJoystick.pHaptic ? "haptic" : "not haptic", iQuery);
            }
        }
        Core::Log->ListEnd();

        // sort correctly by joystick instance ID
        std::sort(m_aJoystick.begin(), m_aJoystick.end(), [](const coreJoystick& a, const coreJoystick& b)
        {
            return (SDL_JoystickInstanceID(a.pJoystick) < SDL_JoystickInstanceID(b.pJoystick));
        });
    }
    else Core::Log->Info("No Joysticks or Gamepads found");

    // append empty joystick object to prevent problems
    m_aJoystick.emplace_back();
}


// ****************************************************************
/* shut down joystick input */
void CoreInput::__CloseJoysticks()
{
    // close all joystick and haptic devices
    FOR_EACH(it, m_aJoystick)
    {
             if(it->pHaptic)     SDL_HapticClose        (it->pHaptic);
             if(it->pController) SDL_GameControllerClose(it->pController);
        else if(it->pJoystick)   SDL_JoystickClose      (it->pJoystick);
    }

    // clear memory
    m_aJoystick.clear();
}


// ****************************************************************
/* convert joystick instance ID to joystick index */
coreUintW CoreInput::__GetJoystickIndex(const SDL_JoystickID iID)const
{
    const SDL_Joystick* pFind = SDL_JoystickFromInstanceID(iID);

    // find required joystick object
    FOR_EACH(it, m_aJoystick)
    {
        if(it->pJoystick == pFind)
            return (it - m_aJoystick.begin());
    }

    // return index to empty joystick object
    return (m_aJoystick.size() - 1u);
}