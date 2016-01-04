//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_INPUT_H_
#define _CORE_GUARD_INPUT_H_

// TODO: touch-related functions empty on non-mobile
// TODO: death-zone and max joystick value into config file
// TODO: identify joysticks by ID (maybe switch to gamepad interface)
// TODO: save GUID ?, format Name and GUID get functions
// TODO: handle unsupported RelativeMouseMode, SDL_HINT_MOUSE_RELATIVE_MODE_WARP
// TODO: handle joystick/gamepad remove and insert during execution (also events in CoreSystem)


// ****************************************************************
// input definitions
#define __CORE_INPUT_PRESS(x)       {SET_BIT(x, 3u, false)               SET_BIT(x, 2u, !CONTAINS_BIT(x, 1u)) SET_BIT(x, 1u, true)}
#define __CORE_INPUT_RELEASE(x)     {SET_BIT(x, 3u, CONTAINS_BIT(x, 1u)) SET_BIT(x, 2u, false)                SET_BIT(x, 1u, false)}
#define __CORE_INPUT_JOYSTICK(i)    (m_aJoystick[MIN(coreUintW(i), m_aJoystick.size() - 1u)])

#define CORE_INPUT_BUTTONS_KEYBOARD (284u)   //!< number of regarded keyboard buttons (#SDL_NUM_SCANCODES)
#define CORE_INPUT_BUTTONS_MOUSE    (16u)    //!< number of regarded mouse buttons
#define CORE_INPUT_BUTTONS_JOYSTICK (32u)    //!< number of regarded joystick buttons
#if defined(_CORE_ANDROID_)
    #define CORE_INPUT_FINGERS      (5u)     //!< maximum number of simultaneous fingers
#else
    #define CORE_INPUT_FINGERS      (1u)
#endif

#define CORE_INPUT_JOYSTICK_DEAD    (0x2000)
#define CORE_INPUT_JOYSTICK_MAX     (0x7000)

#define CORE_INPUT_KEY(k)  (SDL_SCANCODE_ ## k)
#define CORE_INPUT_CHAR(c) (SDLK_         ## c)
using coreInputKey  = SDL_Scancode;
using coreInputChar = SDL_Keycode;

enum coreInputButton : coreUint8
{
    CORE_INPUT_LEFT   = 1u,
    CORE_INPUT_MIDDLE = 2u,
    CORE_INPUT_RIGHT  = 3u
};

enum coreInputType : coreUint8
{
    CORE_INPUT_DATA    = 0u,
    CORE_INPUT_HOLD    = 1u,
    CORE_INPUT_PRESS   = 2u,
    CORE_INPUT_RELEASE = 3u
};


// ****************************************************************
// main input component
class CoreInput final
{
private:
    //! keyboard structure
    struct coreKeyboard
    {
        coreUint8     aiButton[CORE_INPUT_BUTTONS_KEYBOARD];   //!< status of the keyboard buttons
        coreInputKey  iLast;                                   //!< last pressed keyboard button
        coreInputChar iChar;                                   //!< current text-input character
    };

    //! mouse structure
    struct coreMouse
    {
        coreUint8   aiButton[CORE_INPUT_BUTTONS_MOUSE];   //!< status of the mouse buttons
        coreUint8   iLast;                                //!< last pressed mouse button
        coreVector2 vPosition;                            //!< absolute position of the mouse cursor
        coreVector3 vRelative;                            //!< relative movement of the mouse cursor
    };

    //! joystick structure
    struct coreJoystick
    {
        SDL_GameController* pController;                     //!< game controller handle
        SDL_Joystick*       pJoystick;                       //!< joystick device handle
        SDL_Haptic*         pHaptic;                         //!< haptic device handle

        coreUint8   aiButton[CORE_INPUT_BUTTONS_JOYSTICK];   //!< status of the joystick buttons
        coreUint8   iLast;                                   //!< last pressed joystick button
        coreVector2 vRelative;                               //!< relative movement of the control axis
    };

    //! touch structure
    struct coreTouch
    {
        coreUint8   iButton;     //!< status of the finger
        coreVector2 vPosition;   //!< absolute position of the finger
        coreVector2 vRelative;   //!< relative movement of the finger
        coreFloat   fPressure;   //!< current quantity of pressure applied
    };


private:
    coreKeyboard              m_Keyboard;     //!< main keyboard object
    coreMouse                 m_Mouse;        //!< main mouse object
    std::vector<coreJoystick> m_aJoystick;    //!< list with joystick objects

    coreTouch m_aTouch[CORE_INPUT_FINGERS];   //!< array with touch objects

    SDL_Cursor* m_pCursor;                    //!< hardware mouse cursor
    coreBool    m_bCursorVisible;             //!< status of the mouse cursor


private:
    CoreInput()noexcept;
    ~CoreInput();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreInput)

    //! control the mouse cursor
    //! @{
    void DefineCursor(const coreChar* pcPath);
    void ShowCursor  (const coreBool  bStatus);
    inline const coreBool& IsCursorVisible()const {return m_bCursorVisible;}
    //! @}

    //! use controls from other devices
    //! @{
    void UseMouseWithKeyboard(const coreInputKey iLeft, const coreInputKey iRight, const coreInputKey iUp, const coreInputKey iDown, const coreInputKey iButton1, const coreInputKey iButton2, const coreFloat fSpeed);
    void UseMouseWithJoystick(const coreUintW iIndex, const coreUint8 iButton1, const coreUint8 iButton2, const coreFloat fSpeed);
    void ForwardDpadToStick  (const coreUintW iIndex);
    //! @}

    //! get joystick data
    //! @{
    inline const coreChar* GetJoystickName(const coreUintW iIndex)const {return __CORE_INPUT_JOYSTICK(iIndex).pController ? SDL_GameControllerName(__CORE_INPUT_JOYSTICK(iIndex).pController) : __CORE_INPUT_JOYSTICK(iIndex).pJoystick ? SDL_JoystickName(__CORE_INPUT_JOYSTICK(iIndex).pJoystick) : "";}
    inline const coreChar* GetJoystickGUID(const coreUintW iIndex)const {if(__CORE_INPUT_JOYSTICK(iIndex).pJoystick) {coreChar acGUID[64]; SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(__CORE_INPUT_JOYSTICK(iIndex).pJoystick), acGUID, ARRAY_SIZE(acGUID)); return PRINT("%s", acGUID);} return "";}
    inline coreUintW       GetJoystickNum ()const                       {return m_aJoystick.size() - 1u;}
    //! @}

    //! process input events
    //! @{
    coreBool ProcessEvent(const SDL_Event& oEvent);
    //! @}

    //! access keyboard input
    //! @{
    inline void                 SetKeyboardButton(const coreInputKey  iButton, const coreBool bStatus)         {WARN_IF(iButton >= CORE_INPUT_BUTTONS_KEYBOARD) return; SET_BIT(m_Keyboard.aiButton[iButton], 0u, bStatus) if(bStatus) m_Keyboard.iLast = iButton;}
    inline void                 SetKeyboardChar  (const coreInputChar iChar)                                   {m_Keyboard.iChar = iChar;}
    inline coreBool             GetKeyboardButton(const coreInputKey  iButton, const coreInputType iType)const {return CONTAINS_BIT(m_Keyboard.aiButton[iButton], iType);}
    inline const coreInputChar& GetKeyboardChar  ()const                                                       {return m_Keyboard.iChar;}
    //! @}

    //! access mouse input
    //! @{
    inline void               SetMouseButton  (const coreUint8 iButton, const coreBool bStatus)         {WARN_IF(iButton >= CORE_INPUT_BUTTONS_MOUSE) return; SET_BIT(m_Mouse.aiButton[iButton], 0u, bStatus) if(bStatus) m_Mouse.iLast = iButton;}
    inline void               SetMousePosition(const coreVector2& vPosition)                            {m_Mouse.vPosition   = vPosition;}
    inline void               SetMouseRelative(const coreVector2& vRelative)                            {m_Mouse.vRelative.x = vRelative.x; m_Mouse.vRelative.y = vRelative.y;}
    inline void               SetMouseWheel   (const coreFloat fValue)                                  {m_Mouse.vRelative.z = fValue;}
    inline coreBool           GetMouseButton  (const coreUint8 iButton, const coreInputType iType)const {return CONTAINS_BIT(m_Mouse.aiButton[iButton], iType);}
    inline const coreVector2& GetMousePosition()const                                                   {return m_Mouse.vPosition;}
    inline const coreVector3& GetMouseRelative()const                                                   {return m_Mouse.vRelative;}
    inline const coreFloat&   GetMouseWheel   ()const                                                   {return m_Mouse.vRelative.z;}
    //! @}

    //! access joystick input
    //! @{
    inline void               SetJoystickButton  (const coreUintW iIndex, const coreUint8 iButton, const coreBool bStatus)         {WARN_IF(iButton >= CORE_INPUT_BUTTONS_JOYSTICK) return; SET_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiButton[iButton], 0u, bStatus) if(bStatus) __CORE_INPUT_JOYSTICK(iIndex).iLast = iButton;}
    inline void               SetJoystickRelative(const coreUintW iIndex, const coreUint8 iAxis, const coreFloat fValue)           {__CORE_INPUT_JOYSTICK(iIndex).vRelative.arr[iAxis] = fValue;}
    inline coreBool           GetJoystickButton  (const coreUintW iIndex, const coreUint8 iButton, const coreInputType iType)const {return CONTAINS_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiButton[iButton], iType);}
    inline const coreVector2& GetJoystickRelative(const coreUintW iIndex)const                                                     {return __CORE_INPUT_JOYSTICK(iIndex).vRelative;}
    inline void               RumbleJoystick     (const coreUintW iIndex, const coreFloat fStrength, const coreUint32 iLength)     {if(Core::Config->GetBool(CORE_CONFIG_INPUT_HAPTIC)) SDL_HapticRumblePlay(__CORE_INPUT_JOYSTICK(iIndex).pHaptic, fStrength, iLength);}
    //! @}

    //! access touch input
    //! @{
    inline void               SetTouchButton  (const coreUintW iIndex, const coreBool bStatus)         {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; SET_BIT(m_aTouch[iIndex].iButton, 0u, bStatus)}
    inline void               SetTouchPosition(const coreUintW iIndex, const coreVector2& vPosition)   {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; m_aTouch[iIndex].vPosition = vPosition;}
    inline void               SetTouchRelative(const coreUintW iIndex, const coreVector2& vRelative)   {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; m_aTouch[iIndex].vRelative = vRelative;}
    inline void               SetTouchPressure(const coreUintW iIndex, const coreFloat fPressure)      {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; m_aTouch[iIndex].fPressure = fPressure;}
    inline coreBool           GetTouchButton  (const coreUintW iIndex, const coreInputType iType)const {return CONTAINS_BIT(m_aTouch[iIndex].iButton, iType);}
    inline const coreVector2& GetTouchPosition(const coreUintW iIndex)const                            {return m_aTouch[iIndex].vPosition;}
    inline const coreVector2& GetTouchRelative(const coreUintW iIndex)const                            {return m_aTouch[iIndex].vRelative;}
    inline const coreFloat&   GetTouchPressure(const coreUintW iIndex)const                            {return m_aTouch[iIndex].fPressure;}
    template <typename F> void ForEachFinger(const coreInputType iType, F&& nFunction);   //!< [](const coreUintW iFingerIndex) -> void
    //! @}

    //! get last pressed input button
    //! @{
    inline const coreInputKey& GetCurKeyboard()const                       {return m_Keyboard.iLast;}
    inline const coreUint8&    GetCurMouse   ()const                       {return m_Mouse   .iLast;}
    inline const coreUint8&    GetCurJoystick(const coreUintW iIndex)const {return __CORE_INPUT_JOYSTICK(iIndex).iLast;}
    //! @}

    //! clear status of specific input button
    //! @{
    inline void ClearKeyboardButton(const coreInputKey iButton)                      {WARN_IF(iButton >= CORE_INPUT_BUTTONS_KEYBOARD) return; m_Keyboard.aiButton[iButton]                    = 0u;}
    inline void ClearMouseButton   (const coreUint8    iButton)                      {WARN_IF(iButton >= CORE_INPUT_BUTTONS_MOUSE)    return; m_Mouse   .aiButton[iButton]                    = 0u;}
    inline void ClearJoystickButton(const coreUintW iIndex, const coreUint8 iButton) {WARN_IF(iButton >= CORE_INPUT_BUTTONS_JOYSTICK) return; __CORE_INPUT_JOYSTICK(iIndex).aiButton[iButton] = 0u;}
    //! @}


private:
    //! update and clear the input button interface
    //! @{
    void __UpdateButtons();
    void __ClearButtons();
    //! @}
};


// ****************************************************************
// call function for each active finger
template <typename F> void CoreInput::ForEachFinger(const coreInputType iType, F&& nFunction)
{
    for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
    {
        // check finger status and call function
        if(Core::Input->GetTouchButton(i, iType))
            nFunction(i);
    }
}


#endif // _CORE_GUARD_INPUT_H_