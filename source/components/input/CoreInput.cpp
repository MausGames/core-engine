///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
CoreInput::CoreInput()noexcept
: m_Keyboard       {}
, m_Mouse          {}
, m_aJoystick      {}
, m_aTouch         {}
, m_aiTouchCount   {}
, m_pCursor        (NULL)
, m_bCursorVisible (true)
, m_iAnyButton     (0u)
{
    Core::Log->Header("Input Interface");

    const auto nLoadFunc = [](const coreChar* pcPath)
    {
        // retrieve game controller database file
        if(!coreData::FileExists(pcPath)) return;
        const coreFile oFile(pcPath);

        // load additional game controller mappings
        const coreInt32 iNum = SDL_GameControllerAddMappingsFromRW(oFile.CreateReadStream(), 1);
        if(iNum < 0) Core::Log->Warning("Game controller database could not be loaded (SDL: %s)", SDL_GetError());
                else Core::Log->Info   ("Game controller database loaded (%d added, %d total)",   iNum, SDL_GameControllerNumMappings());
    };
    nLoadFunc("data/other/gamecontrollerdb.txt");
    nLoadFunc(coreData::UserFolderPrivate("gamecontrollerdb.txt"));

    // start up joystick input
    this->__OpenJoysticks();

    // clear all last pressed input buttons
    m_Keyboard.iLast = CORE_INPUT_INVALID_KEYBOARD;
    m_Mouse   .iLast = CORE_INPUT_INVALID_MOUSE;
    FOR_EACH(it, m_aJoystick) it->iLast = CORE_INPUT_INVALID_JOYSTICK;
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
/* process input events */
coreBool CoreInput::ProcessEvent(const SDL_Event& oEvent)
{
    switch(oEvent.type)
    {
    // process window events
    case SDL_WINDOWEVENT:
        switch(oEvent.window.event)
        {
        // window focus gained
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            if(!m_bCursorVisible) SDL_SetRelativeMouseMode(SDL_TRUE);
            break;

        // window focus lost
        case SDL_WINDOWEVENT_FOCUS_LOST:
            if(!m_bCursorVisible) SDL_SetRelativeMouseMode(SDL_FALSE);
            break;
        }
        break;

    // set text-input character
    case SDL_TEXTINPUT:
        this->SetKeyboardCharUTF8(oEvent.text.text);
        break;

    // press keyboard button
    case SDL_KEYDOWN:
        if(!oEvent.key.repeat)
        {
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
            else if(oEvent.key.keysym.scancode == CORE_INPUT_KEY(PRINTSCREEN)) return false;   // # return early
        }
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
        this->SetMouseWheel(oEvent.wheel.preciseY);
        break;

#endif

    // press joystick button
    case SDL_JOYBUTTONDOWN:
        if(__CORE_INPUT_JOYSTICK(this->__GetJoystickIndex(oEvent.jbutton.which)).pController && (oEvent.jbutton.button < SDL_CONTROLLER_BUTTON_MAX)) break; FALLTHROUGH
    case SDL_CONTROLLERBUTTONDOWN:
        {
            const coreUintW iIndex = this->__GetJoystickIndex(oEvent.jbutton.which);

                 if(oEvent.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)  this->SetJoystickHat(iIndex, CORE_INPUT_DIR_LEFT,  true);
            else if(oEvent.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) this->SetJoystickHat(iIndex, CORE_INPUT_DIR_RIGHT, true);
            else if(oEvent.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)  this->SetJoystickHat(iIndex, CORE_INPUT_DIR_DOWN,  true);
            else if(oEvent.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)    this->SetJoystickHat(iIndex, CORE_INPUT_DIR_UP,    true);
            else
            {
                this->SetJoystickButton(iIndex, oEvent.jbutton.button, true);
            }
        }
        break;

    // release joystick button
    case SDL_JOYBUTTONUP:
        if(__CORE_INPUT_JOYSTICK(this->__GetJoystickIndex(oEvent.jbutton.which)).pController && (oEvent.jbutton.button < SDL_CONTROLLER_BUTTON_MAX)) break; FALLTHROUGH
    case SDL_CONTROLLERBUTTONUP:
        {
            const coreUintW iIndex = this->__GetJoystickIndex(oEvent.jbutton.which);

                 if(oEvent.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)  this->SetJoystickHat(iIndex, CORE_INPUT_DIR_LEFT,  false);
            else if(oEvent.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) this->SetJoystickHat(iIndex, CORE_INPUT_DIR_RIGHT, false);
            else if(oEvent.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)  this->SetJoystickHat(iIndex, CORE_INPUT_DIR_DOWN,  false);
            else if(oEvent.jbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)    this->SetJoystickHat(iIndex, CORE_INPUT_DIR_UP,    false);
            else
            {
                this->SetJoystickButton(iIndex, oEvent.jbutton.button, false);
            }
        }
        break;

    // move joystick axis
    case SDL_JOYAXISMOTION:
        if(__CORE_INPUT_JOYSTICK(this->__GetJoystickIndex(oEvent.jaxis.which)).pController && (oEvent.jaxis.axis < SDL_CONTROLLER_AXIS_MAX)) break; FALLTHROUGH
    case SDL_CONTROLLERAXISMOTION:
        {
            const coreUintW iIndex = this->__GetJoystickIndex(oEvent.jaxis.which);
            const coreBool  bNew   = (ABS(oEvent.jaxis.value) > Core::Config->GetInt(CORE_CONFIG_INPUT_JOYSTICKDEAD));
            const coreBool  bOld   = (this->GetJoystickAxis(iIndex, oEvent.jaxis.axis) != 0.0f);

            if(bNew != bOld)
            {
                     if(oEvent.jaxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)  this->SetJoystickButton(iIndex, CORE_INPUT_BUTTON_LEFTTRIGGER,  bNew);
                else if(oEvent.jaxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) this->SetJoystickButton(iIndex, CORE_INPUT_BUTTON_RIGHTTRIGGER, bNew);
            }

            if(bNew) this->SetJoystickAxis(iIndex, oEvent.jaxis.axis, CLAMP(I_TO_F(oEvent.jaxis.value) * RCP(I_TO_F(MAX(Core::Config->GetInt(CORE_CONFIG_INPUT_JOYSTICKMAX), 1))) * (((oEvent.jaxis.axis == 1u) || (oEvent.jaxis.axis == 3u)) ? -1.0f : 1.0f), -1.0f, 1.0f));
                else this->SetJoystickAxis(iIndex, oEvent.jaxis.axis, 0.0f);
        }
        break;

    // move joystick hat
    case SDL_JOYHATMOTION:
        {
            const coreUintW iIndex = this->__GetJoystickIndex(oEvent.jhat.which);

            this->SetJoystickHat(iIndex, CORE_INPUT_DIR_LEFT,  oEvent.jhat.value & SDL_HAT_LEFT);
            this->SetJoystickHat(iIndex, CORE_INPUT_DIR_RIGHT, oEvent.jhat.value & SDL_HAT_RIGHT);
            this->SetJoystickHat(iIndex, CORE_INPUT_DIR_DOWN,  oEvent.jhat.value & SDL_HAT_DOWN);
            this->SetJoystickHat(iIndex, CORE_INPUT_DIR_UP,    oEvent.jhat.value & SDL_HAT_UP);
        }
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

    // test if events are identical
    #define __COMPARE(a,b,x) STATIC_ASSERT((offsetof(a, x) == offsetof(b, x)) && (std::is_same_v<decltype(a::x), decltype(b::x)>))
    {
        __COMPARE(SDL_JoyButtonEvent, SDL_ControllerButtonEvent, which)
        __COMPARE(SDL_JoyButtonEvent, SDL_ControllerButtonEvent, button)
        __COMPARE(SDL_JoyAxisEvent,   SDL_ControllerAxisEvent,   which)
        __COMPARE(SDL_JoyAxisEvent,   SDL_ControllerAxisEvent,   axis)
        __COMPARE(SDL_JoyAxisEvent,   SDL_ControllerAxisEvent,   value)
    }
    #undef __COMPARE

    return true;
}


// ****************************************************************
/* set the mouse cursor */
void CoreInput::SetCursor(const coreChar* pcPath)
{
    if(pcPath)
    {
        // retrieve texture file
        coreFileScope pFile = Core::Manager::Resource->RetrieveFile(pcPath);

        // decompress file to plain pixel data
        coreSurfaceScope pData = IMG_LoadTyped_RW(pFile->CreateReadStream(), 1, coreData::StrExtension(pcPath));
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
    }
    else
    {
        // reset to default cursor
        SDL_SetCursor(SDL_GetDefaultCursor());
    }

    Core::Log->Info("Cursor (%s) loaded", pcPath);
}


// ****************************************************************
/* show or hide the mouse cursor */
void CoreInput::ShowCursor(const coreBool bStatus)
{
#if !defined(_CORE_DEBUG_)

    if(m_bCursorVisible == bStatus) return;

    // toggle cursor visibility
    SDL_SetRelativeMouseMode(bStatus ? SDL_FALSE : SDL_TRUE);

    // save visibility status
    m_bCursorVisible = bStatus;

#endif
}


// ****************************************************************
/* control mouse with keyboard */
void CoreInput::UseMouseWithKeyboard(const coreInputKey iLeft, const coreInputKey iRight, const coreInputKey iDown, const coreInputKey iUp, const coreInputKey iButton1, const coreInputKey iButton2, const coreFloat fSpeed)
{
    // get original input
    coreVector2 vAcc = coreVector2(0.0f,0.0f);
    if(this->GetKeyboardButton(iLeft,  CORE_INPUT_HOLD)) vAcc.x -= 1.0f;
    if(this->GetKeyboardButton(iRight, CORE_INPUT_HOLD)) vAcc.x += 1.0f;
    if(this->GetKeyboardButton(iDown,  CORE_INPUT_HOLD)) vAcc.y -= 1.0f;
    if(this->GetKeyboardButton(iUp,    CORE_INPUT_HOLD)) vAcc.y += 1.0f;

    // move the mouse cursor
    if(!vAcc.IsNull())
    {
        const coreVector2 vPos = this->GetMousePosition() + coreVector2(0.5f,-0.5f);
        const coreVector2 vNew = (vAcc.Normalized() * Core::System->GetResolution().yx().HighRatio() * (Core::System->GetTime() * fSpeed) + vPos) * Core::System->GetResolution();
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
    const coreVector2 vVelocity = this->GetJoystickStickL(iIndex);

    // move the mouse cursor
    if(!vVelocity.IsNull())
    {
        const coreVector2 vPos = this->GetMousePosition() + coreVector2(0.5f,-0.5f);
        const coreVector2 vNew = (vVelocity * Core::System->GetResolution().yx().HighRatio() * (Core::System->GetTime() * fSpeed) + vPos) * Core::System->GetResolution();
        SDL_WarpMouseInWindow(Core::System->GetWindow(), F_TO_SI(vNew.x + 0.5f), F_TO_SI(-vNew.y + 0.5f));
    }

    // press mouse buttons
    if(this->GetJoystickButton(iIndex, iButton1, CORE_INPUT_PRESS))   this->SetMouseButton(CORE_INPUT_LEFT,  true);
    if(this->GetJoystickButton(iIndex, iButton1, CORE_INPUT_RELEASE)) this->SetMouseButton(CORE_INPUT_LEFT,  false);
    if(this->GetJoystickButton(iIndex, iButton2, CORE_INPUT_PRESS))   this->SetMouseButton(CORE_INPUT_RIGHT, true);
    if(this->GetJoystickButton(iIndex, iButton2, CORE_INPUT_RELEASE)) this->SetMouseButton(CORE_INPUT_RIGHT, false);
}


// ****************************************************************
/* forward hat input to stick input on joystick */
void CoreInput::ForwardHatToStick(const coreUintW iIndex)
{
    WARN_IF(iIndex >= m_aJoystick.size()) return;

    // check for hat directions
    coreVector2 vVelocity = coreVector2(0.0f,0.0f);
    if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_LEFT,  CORE_INPUT_HOLD)) vVelocity.x -= 1.0f;
    if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_RIGHT, CORE_INPUT_HOLD)) vVelocity.x += 1.0f;
    if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_DOWN,  CORE_INPUT_HOLD)) vVelocity.y -= 1.0f;
    if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_UP,    CORE_INPUT_HOLD)) vVelocity.y += 1.0f;

    // invoke stick movement
    if(!vVelocity.IsNull())
    {
        vVelocity = vVelocity.Normalized();
        this->SetJoystickAxis(iIndex, 0u, vVelocity.x);
        this->SetJoystickAxis(iIndex, 1u, vVelocity.y);
    }

    // reset stick movement on release
    if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_LEFT,  CORE_INPUT_RELEASE) ||
       this->GetJoystickHat(iIndex, CORE_INPUT_DIR_RIGHT, CORE_INPUT_RELEASE)) this->SetJoystickAxis(iIndex, 0u, 0.0f);
    if(this->GetJoystickHat(iIndex, CORE_INPUT_DIR_DOWN,  CORE_INPUT_RELEASE) ||
       this->GetJoystickHat(iIndex, CORE_INPUT_DIR_UP,    CORE_INPUT_RELEASE)) this->SetJoystickAxis(iIndex, 1u, 0.0f);
}


// ****************************************************************
/* create joystick rumble effect */
void CoreInput::JoystickRumble(const coreUintW iIndex, const coreFloat fStrengthLow, const coreFloat fStrengthHigh, const coreUint32 iLengthMs)
{
    ASSERT((fStrengthLow >= 0.0f) && (fStrengthLow <= 1.0f) && (fStrengthHigh >= 0.0f) && (fStrengthHigh <= 1.0f))

    if(Core::Config->GetBool(CORE_CONFIG_INPUT_RUMBLE))
    {
        // start simple rumble effect
        SDL_JoystickRumble(__CORE_INPUT_JOYSTICK(iIndex).pJoystick, F_TO_UI(fStrengthLow * 65535.0f), F_TO_UI(fStrengthHigh * 65535.0f), iLengthMs);
    }
}


// ****************************************************************
/* change joystick LED color */
void CoreInput::JoystickChangeLED(const coreUintW iIndex, const coreVector3 vColor)
{
    coreJoystick& oJoystick = __CORE_INPUT_JOYSTICK(iIndex);

    // check for current color
    const coreUint32 iNewColor = coreVector4(vColor, 1.0f).PackUnorm4x8();
    if(oJoystick.iColor != iNewColor)
    {
        // set new color
        oJoystick.iColor = iNewColor;
        SDL_JoystickSetLED(oJoystick.pJoystick, F_TO_UI(vColor.x * 255.0f), F_TO_UI(vColor.y * 255.0f), F_TO_UI(vColor.z * 255.0f));
    }
}


// ****************************************************************
/* clear status of all input buttons */
void CoreInput::ClearButtonAll()
{
    // clear each available device
    this->ClearKeyboardButtonAll();
    this->ClearMouseButtonAll();
    this->ClearJoystickButtonAll();
    this->ClearTouchButtonAll();

    // clear status of any available button
    this->ClearAnyButton();
}


// ****************************************************************
/* update the input button interface (start) */
void CoreInput::__UpdateButtonsStart()
{
    // merge all available joysticks
    if(m_aJoystick.size() > 1u)
    {
        coreJoystick& oAny = m_aJoystick.back();

        // reset joystick input
        for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_JOYSTICK; ++i) REMOVE_BIT(oAny.aiButton[i], CORE_INPUT_DATA)
        for(coreUintW i = 0u; i < CORE_INPUT_DIRECTIONS;       ++i) REMOVE_BIT(oAny.aiHat   [i], CORE_INPUT_DATA)
        std::memset(oAny.afAxis, 0, sizeof(coreJoystick::afAxis));

        for(coreUintW j = 0u, je = this->GetJoystickNum(); j < je; ++j)
        {
            const coreJoystick& oReal = m_aJoystick[j];

            // forward joystick input
            for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_JOYSTICK; ++i) if(HAS_BIT(oReal.aiButton[i], CORE_INPUT_DATA)) ADD_BIT(oAny.aiButton[i], CORE_INPUT_DATA)
            for(coreUintW i = 0u; i < CORE_INPUT_DIRECTIONS;       ++i) if(HAS_BIT(oReal.aiHat   [i], CORE_INPUT_DATA)) ADD_BIT(oAny.aiHat   [i], CORE_INPUT_DATA)
            for(coreUintW i = 0u; i < CORE_INPUT_AXIS;             ++i) if(oReal.afAxis[i]) oAny.afAxis[i] = oReal.afAxis[i];

            // forward last pressed joystick button
            if(oReal.iLast != CORE_INPUT_INVALID_JOYSTICK) oAny.iLast = oReal.iLast;
        }
    }

    // process keyboard inputs
    for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_KEYBOARD; ++i)
    {
        if(HAS_BIT(m_Keyboard.aiButton[i], CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (m_Keyboard.aiButton[i])
                                                        else __CORE_INPUT_RELEASE(m_Keyboard.aiButton[i])
        __CORE_INPUT_COUNT(m_Keyboard.aiButton[i], m_Keyboard.aiCount)
    }

#if !defined(_CORE_MOBILE_)

    // process mouse inputs
    for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_MOUSE; ++i)
    {
        if(HAS_BIT(m_Mouse.aiButton[i], CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (m_Mouse.aiButton[i])
                                                     else __CORE_INPUT_RELEASE(m_Mouse.aiButton[i])
        __CORE_INPUT_COUNT(m_Mouse.aiButton[i], m_Mouse.aiCount)
    }

#endif

    // process joystick inputs
    FOR_EACH(it, m_aJoystick)
    {
        for(coreUintW i = 0u; i < CORE_INPUT_BUTTONS_JOYSTICK; ++i)
        {
            if(HAS_BIT(it->aiButton[i], CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (it->aiButton[i])
                                                     else __CORE_INPUT_RELEASE(it->aiButton[i])
            __CORE_INPUT_COUNT(it->aiButton[i], it->aiCount)
        }

        for(coreUintW i = 0u; i < CORE_INPUT_DIRECTIONS; ++i)
        {
            if(HAS_BIT(it->aiHat[i], CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (it->aiHat[i])
                                                  else __CORE_INPUT_RELEASE(it->aiHat[i])
        }
    }

#if defined(_CORE_MOBILE_)

    // process touch inputs
    for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
    {
        if(HAS_BIT(m_aTouch[i].iButton, CORE_INPUT_DATA)) __CORE_INPUT_PRESS  (m_aTouch[i].iButton)
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
    {
        m_aTouch[i].vRelative = coreVector2(0.0f,0.0f);
    }

    // clear current text-input character
    std::memset(m_Keyboard.acChar, 0, sizeof(m_Keyboard.acChar));

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

                // get device type
                oJoystick.eControllerType = SDL_GameControllerGetType(oJoystick.pController);
                oJoystick.eJoystickType   = SDL_JoystickGetType      (oJoystick.pJoystick);

                if(oJoystick.eControllerType == SDL_CONTROLLER_TYPE_UNKNOWN)
                {
                    // get device information
                    coreUint16 iVendor, iProduct;
                    SDL_GetJoystickGUIDInfo(SDL_JoystickGetGUID(oJoystick.pJoystick), &iVendor, &iProduct, NULL, NULL);

                    // identify special controller type
                    if((iVendor == 0x28DEu) && (iProduct == 0x11FFu))
                    {
                        oJoystick.eControllerType = SDL_GameControllerType(CORE_INPUT_TYPE_STEAM);
                    }
                }

                // get device features
                const coreInt32 iNumButtons   = SDL_JoystickNumButtons           (oJoystick.pJoystick);
                const coreInt32 iNumAxes      = SDL_JoystickNumAxes              (oJoystick.pJoystick);
                const coreInt32 iNumHats      = SDL_JoystickNumHats              (oJoystick.pJoystick);
                const coreInt32 iNumTouchpads = SDL_GameControllerGetNumTouchpads(oJoystick.pController);
                const coreBool  bHasAccel     = SDL_GameControllerHasSensor      (oJoystick.pController, SDL_SENSOR_ACCEL) != SDL_FALSE;
                const coreBool  bHasGyro      = SDL_GameControllerHasSensor      (oJoystick.pController, SDL_SENSOR_GYRO)  != SDL_FALSE;

                // get current power-level
                const SDL_JoystickPowerLevel ePowerLevel = SDL_JoystickCurrentPowerLevel(oJoystick.pJoystick);

                // save joystick object
                m_aJoystick.push_back(oJoystick);
                Core::Log->ListAdd(CORE_LOG_BOLD("%s:") " %s (%s, %s, %s, powerlevel %d, type %d+%d, %d buttons, %d axes, %d hats, %d touchpads, rumble %s, led %s, accelerometer %s, gyroscope %s)",
                                   oJoystick.pController ? "Gamepad" : "Joystick", this->GetJoystickName(i), this->GetJoystickSerial(i), this->GetJoystickPath(i), this->GetJoystickGUID(i),
                                   ePowerLevel, oJoystick.eControllerType, oJoystick.eJoystickType, iNumButtons, iNumAxes, iNumHats, iNumTouchpads,
                                   this->GetJoystickHasRumble(i) ? "YES" : "NO", this->GetJoystickHasLED(i) ? "YES" : "NO", bHasAccel ? "YES" : "NO", bHasGyro ? "YES" : "NO");
            }
        }
        Core::Log->ListEnd();

        // sort correctly by joystick instance ID
        std::sort(m_aJoystick.begin(), m_aJoystick.end(), [](const coreJoystick& A, const coreJoystick& B)
        {
            return (SDL_JoystickInstanceID(A.pJoystick) < SDL_JoystickInstanceID(B.pJoystick));
        });
    }
    else Core::Log->Info("No Joysticks or Gamepads found");

    // append empty/merged joystick object
    m_aJoystick.emplace_back();

    // clear last pressed joystick buttons
    FOR_EACH(it, m_aJoystick) it->iLast = CORE_INPUT_INVALID_JOYSTICK;
}


// ****************************************************************
/* shut down joystick input */
void CoreInput::__CloseJoysticks()
{
    // close all joystick devices
    FOR_EACH(it, m_aJoystick)
    {
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
        if(it->pJoystick == pFind) return m_aJoystick.index(it);
    }

    // return index to empty/merged joystick object
    return (m_aJoystick.size() - 1u);
}