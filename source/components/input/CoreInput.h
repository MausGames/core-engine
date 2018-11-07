///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_INPUT_H_
#define _CORE_GUARD_INPUT_H_

// TODO: touch-related functions empty on non-mobile
// TODO: save GUID ?, format Name and GUID get functions
// TODO: remove the 1-frame delay when using the Set* interface from game-code
// TODO: add SDL_JoystickGetType


// ****************************************************************
// input definitions
#define __CORE_INPUT_PRESS(x)       {SET_BIT(x, CORE_INPUT_RELEASE, false)                            SET_BIT(x, CORE_INPUT_PRESS, !CONTAINS_BIT(x, CORE_INPUT_HOLD)) SET_BIT(x, CORE_INPUT_HOLD, true)}
#define __CORE_INPUT_RELEASE(x)     {SET_BIT(x, CORE_INPUT_RELEASE, CONTAINS_BIT(x, CORE_INPUT_HOLD)) SET_BIT(x, CORE_INPUT_PRESS, false)                             SET_BIT(x, CORE_INPUT_HOLD, false)}
#define __CORE_INPUT_COUNT(x,c)     {if(x) for(coreUintW j = 0u; j < CORE_INPUT_TYPES; ++j) if(CONTAINS_BIT(x, j)) {++(c)[j]; ADD_BIT(m_iAnyButton, j)}}
#define __CORE_INPUT_JOYSTICK(i)    (m_aJoystick[MIN(coreUintW(i), m_aJoystick.size() - 1u)])

#define CORE_INPUT_BUTTONS_KEYBOARD (287u)   //!< number of regarded keyboard buttons (#SDL_NUM_SCANCODES)
#define CORE_INPUT_BUTTONS_MOUSE    (16u)    //!< number of regarded mouse buttons
#define CORE_INPUT_BUTTONS_JOYSTICK (32u)    //!< number of regarded joystick buttons
#if defined(_CORE_MOBILE_)
    #define CORE_INPUT_FINGERS      (5u)     //!< maximum number of simultaneous fingers
#else
    #define CORE_INPUT_FINGERS      (1u)
#endif
#define CORE_INPUT_DIRECTIONS       (4u)
#define CORE_INPUT_TYPES            (4u)

#define CORE_INPUT_INVALID_KEYBOARD (CORE_INPUT_KEY(UNKNOWN))
#define CORE_INPUT_INVALID_MOUSE    (0xFFu)
#define CORE_INPUT_INVALID_JOYSTICK (0xFFu)

#define CORE_INPUT_KEY(k)           (SDL_SCANCODE_ ## k)
#define CORE_INPUT_CHAR(c)          (SDLK_         ## c)

using coreInputKey  = SDL_Scancode;
using coreInputChar = SDL_Keycode;

enum coreInputButton : coreUint8
{
    CORE_INPUT_LEFT   = 1u,
    CORE_INPUT_MIDDLE = 2u,
    CORE_INPUT_RIGHT  = 3u
};

enum coreInputDir : coreUint8
{
    CORE_INPUT_DIR_LEFT  = 0u,
    CORE_INPUT_DIR_RIGHT = 1u,
    CORE_INPUT_DIR_DOWN  = 2u,
    CORE_INPUT_DIR_UP    = 3u
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
    struct coreKeyboard final
    {
        coreUint8     aiButton[CORE_INPUT_BUTTONS_KEYBOARD];   //!< status of the keyboard buttons
        coreUint16    aiCount [CORE_INPUT_TYPES];              //!< number of keyboard buttons with same status
        coreInputKey  iLast;                                   //!< last pressed keyboard button
        coreInputChar iChar;                                   //!< current text-input character
    };

    //! mouse structure
    struct coreMouse final
    {
        coreUint8   aiButton[CORE_INPUT_BUTTONS_MOUSE];   //!< status of the mouse buttons
        coreUint8   aiCount [CORE_INPUT_TYPES];           //!< number of mouse buttons with same status
        coreUint8   iLast;                                //!< last pressed mouse button
        coreVector2 vPosition;                            //!< absolute position of the mouse cursor
        coreVector3 vRelative;                            //!< relative movement of the mouse cursor
    };

    //! joystick structure
    struct coreJoystick final
    {
        SDL_GameController* pController;                     //!< game controller handle
        SDL_Joystick*       pJoystick;                       //!< joystick device handle
        SDL_Haptic*         pHaptic;                         //!< haptic device handle

        coreUint8   aiButton[CORE_INPUT_BUTTONS_JOYSTICK];   //!< status of the joystick buttons
        coreUint8   aiCount [CORE_INPUT_TYPES];              //!< number of joystick buttons with same status
        coreUint8   iLast;                                   //!< last pressed joystick button
        coreUint8   aiHat[CORE_INPUT_DIRECTIONS];            //!< status of the joystick hat
        coreVector2 vRelative;                               //!< relative movement of the control axis
    };

    //! touch structure
    struct coreTouch final
    {
        coreUint8   iButton;     //!< status of the finger
        coreVector2 vPosition;   //!< absolute position of the finger
        coreVector2 vRelative;   //!< relative movement of the finger
        coreFloat   fPressure;   //!< current quantity of pressure applied
    };


private:
    coreKeyboard              m_Keyboard;           //!< main keyboard object
    coreMouse                 m_Mouse;              //!< main mouse object
    std::vector<coreJoystick> m_aJoystick;          //!< list with joystick objects

    coreTouch m_aTouch      [CORE_INPUT_FINGERS];   //!< array with touch objects
    coreUint8 m_aiTouchCount[CORE_INPUT_TYPES];     //!< number of fingers with the same status

    SDL_Cursor* m_pCursor;                          //!< hardware mouse cursor
    coreBool    m_bCursorVisible;                   //!< status of the mouse cursor

    coreUint8 m_iAnyButton;                         //!< status of any available button (keyboard, mouse, joystick, touch)


private:
    CoreInput()noexcept;
    ~CoreInput();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreInput)

    //! control the mouse cursor
    //! @{
    void SetCursor (const coreChar* pcPath);
    void ShowCursor(const coreBool  bStatus);
    inline const coreBool& IsCursorVisible()const {return m_bCursorVisible;}
    //! @}

    //! use controls from other devices
    //! @{
    void UseMouseWithKeyboard(const coreInputKey iLeft, const coreInputKey iRight, const coreInputKey iDown, const coreInputKey iUp, const coreInputKey iButton1, const coreInputKey iButton2, const coreFloat fSpeed);
    void UseMouseWithJoystick(const coreUintW iIndex, const coreUint8 iButton1, const coreUint8 iButton2, const coreFloat fSpeed);
    void ForwardHatToStick   (const coreUintW iIndex);
    //! @}

    //! get joystick data
    //! @{
    inline const coreChar* GetJoystickName  (const coreUintW iIndex)const {return __CORE_INPUT_JOYSTICK(iIndex).pController ? SDL_GameControllerName(__CORE_INPUT_JOYSTICK(iIndex).pController) : __CORE_INPUT_JOYSTICK(iIndex).pJoystick ? SDL_JoystickName(__CORE_INPUT_JOYSTICK(iIndex).pJoystick) : "";}
    inline const coreChar* GetJoystickGUID  (const coreUintW iIndex)const {if(__CORE_INPUT_JOYSTICK(iIndex).pJoystick) {static coreChar acGUID[64]; SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(__CORE_INPUT_JOYSTICK(iIndex).pJoystick), acGUID, ARRAY_SIZE(acGUID)); return acGUID;} return "";}
    inline coreBool        GetJoystickRumble(const coreUintW iIndex)const {return __CORE_INPUT_JOYSTICK(iIndex).pHaptic ? true : false;}
    inline coreUintW       GetJoystickNum   ()const                       {return m_aJoystick.size() - 1u;}
    //! @}

    //! process input events
    //! @{
    coreBool ProcessEvent(const SDL_Event& oEvent);
    //! @}

    //! access keyboard input
    //! @{
    inline void                 SetKeyboardButton(const coreInputKey  iButton, const coreBool bStatus)         {WARN_IF(iButton >= CORE_INPUT_BUTTONS_KEYBOARD) return; SET_BIT(m_Keyboard.aiButton[iButton], CORE_INPUT_DATA, bStatus) if(bStatus) m_Keyboard.iLast = iButton;}
    inline void                 SetKeyboardChar  (const coreInputChar iChar)                                   {m_Keyboard.iChar = iChar;}
    inline coreBool             GetKeyboardButton(const coreInputKey  iButton, const coreInputType iType)const {ASSERT(iButton < CORE_INPUT_BUTTONS_KEYBOARD) return CONTAINS_BIT(m_Keyboard.aiButton[iButton], iType);}
    inline const coreInputChar& GetKeyboardChar  ()const                                                       {return m_Keyboard.iChar;}
    //! @}

    //! access mouse input
    //! @{
    inline void               SetMouseButton  (const coreUint8 iButton, const coreBool bStatus)         {WARN_IF(iButton >= CORE_INPUT_BUTTONS_MOUSE) return; SET_BIT(m_Mouse.aiButton[iButton], CORE_INPUT_DATA, bStatus) if(bStatus) m_Mouse.iLast = iButton;}
    inline void               SetMousePosition(const coreVector2& vPosition)                            {m_Mouse.vPosition   = vPosition;}
    inline void               SetMouseRelative(const coreVector2& vRelative)                            {m_Mouse.vRelative.x = vRelative.x; m_Mouse.vRelative.y = vRelative.y;}
    inline void               SetMouseWheel   (const coreFloat fValue)                                  {m_Mouse.vRelative.z = fValue;}
    inline coreBool           GetMouseButton  (const coreUint8 iButton, const coreInputType iType)const {ASSERT(iButton < CORE_INPUT_BUTTONS_MOUSE) return CONTAINS_BIT(m_Mouse.aiButton[iButton], iType);}
    inline const coreVector2& GetMousePosition()const                                                   {return m_Mouse.vPosition;}
    inline const coreVector3& GetMouseRelative()const                                                   {return m_Mouse.vRelative;}
    inline const coreFloat&   GetMouseWheel   ()const                                                   {return m_Mouse.vRelative.z;}
    //! @}

    //! access joystick input
    //! @{
    inline void               SetJoystickButton  (const coreUintW iIndex, const coreUint8 iButton, const coreBool bStatus)               {WARN_IF(iButton    >= CORE_INPUT_BUTTONS_JOYSTICK) return; SET_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiButton[iButton], CORE_INPUT_DATA, bStatus) if(bStatus) __CORE_INPUT_JOYSTICK(iIndex).iLast = iButton;}
    inline void               SetJoystickHat     (const coreUintW iIndex, const coreInputDir iDirection, const coreBool bStatus)         {WARN_IF(iDirection >= CORE_INPUT_DIRECTIONS)       return; SET_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiHat[iDirection], CORE_INPUT_DATA, bStatus)}
    inline void               SetJoystickRelative(const coreUintW iIndex, const coreUint8 iAxis, const coreFloat fValue)                 {WARN_IF(iAxis      >= 2u)                          return; __CORE_INPUT_JOYSTICK(iIndex).vRelative.arr(iAxis) = fValue;}
    inline coreBool           GetJoystickButton  (const coreUintW iIndex, const coreUint8 iButton, const coreInputType iType)const       {ASSERT(iButton    < CORE_INPUT_BUTTONS_JOYSTICK) return CONTAINS_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiButton[iButton], iType);}
    inline coreBool           GetJoystickHat     (const coreUintW iIndex, const coreInputDir iDirection, const coreInputType iType)const {ASSERT(iDirection < CORE_INPUT_DIRECTIONS)       return CONTAINS_BIT(__CORE_INPUT_JOYSTICK(iIndex).aiHat[iDirection], iType);}
    inline const coreVector2& GetJoystickRelative(const coreUintW iIndex)const                                                           {return __CORE_INPUT_JOYSTICK(iIndex).vRelative;}
    inline void               RumbleJoystick     (const coreUintW iIndex, const coreFloat fStrength, const coreUint32 iLength)           {if(__CORE_INPUT_JOYSTICK(iIndex).pHaptic) SDL_HapticRumblePlay(__CORE_INPUT_JOYSTICK(iIndex).pHaptic, fStrength, iLength);}
    //! @}

    //! access touch input
    //! @{
    inline void                SetTouchButton  (const coreUintW iIndex, const coreBool bStatus)         {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; SET_BIT(m_aTouch[iIndex].iButton, CORE_INPUT_DATA, bStatus)}
    inline void                SetTouchPosition(const coreUintW iIndex, const coreVector2& vPosition)   {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; m_aTouch[iIndex].vPosition = vPosition;}
    inline void                SetTouchRelative(const coreUintW iIndex, const coreVector2& vRelative)   {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; m_aTouch[iIndex].vRelative = vRelative;}
    inline void                SetTouchPressure(const coreUintW iIndex, const coreFloat fPressure)      {WARN_IF(iIndex >= CORE_INPUT_FINGERS) return; m_aTouch[iIndex].fPressure = fPressure;}
    inline coreBool            GetTouchButton  (const coreUintW iIndex, const coreInputType iType)const {ASSERT(iIndex < CORE_INPUT_FINGERS) return CONTAINS_BIT(m_aTouch[iIndex].iButton, iType);}
    inline const coreVector2&  GetTouchPosition(const coreUintW iIndex)const                            {ASSERT(iIndex < CORE_INPUT_FINGERS) return m_aTouch[iIndex].vPosition;}
    inline const coreVector2&  GetTouchRelative(const coreUintW iIndex)const                            {ASSERT(iIndex < CORE_INPUT_FINGERS) return m_aTouch[iIndex].vRelative;}
    inline const coreFloat&    GetTouchPressure(const coreUintW iIndex)const                            {ASSERT(iIndex < CORE_INPUT_FINGERS) return m_aTouch[iIndex].fPressure;}
    template <typename F> void ForEachFinger   (const coreInputType iType, F&& nFunction);   //!< [](const coreUintW iFingerIndex) -> void
    //! @}

    //! get number of input buttons with same status
    //! @{
    inline const coreUint16& GetCountKeyboard(const coreInputType iType)const                         {ASSERT(iType < CORE_INPUT_TYPES) return m_Keyboard.aiCount[iType];}
    inline const coreUint8&  GetCountMouse   (const coreInputType iType)const                         {ASSERT(iType < CORE_INPUT_TYPES) return m_Mouse.aiCount[iType];}
    inline const coreUint8&  GetCountJoystick(const coreUintW iIndex, const coreInputType iType)const {ASSERT(iType < CORE_INPUT_TYPES) return __CORE_INPUT_JOYSTICK(iIndex).aiCount[iType];}
    inline const coreUint8&  GetCountTouch   (const coreInputType iType)const                         {ASSERT(iType < CORE_INPUT_TYPES) return m_aiTouchCount[iType];}
    //! @}

    //! get last pressed input button
    //! @{
    inline const coreInputKey& GetLastKeyboard()const                       {return m_Keyboard.iLast;}
    inline const coreUint8&    GetLastMouse   ()const                       {return m_Mouse   .iLast;}
    inline const coreUint8&    GetLastJoystick(const coreUintW iIndex)const {return __CORE_INPUT_JOYSTICK(iIndex).iLast;}
    //! @}

    //! get status of any available button
    //! @{
    inline coreBool GetAnyButton(const coreInputType iType)const {return CONTAINS_BIT(m_iAnyButton, iType);}
    //! @}

    //! clear status of input buttons
    //! @{
    inline void ClearKeyboardButton   (const coreInputKey iButton)                      {WARN_IF(iButton >= CORE_INPUT_BUTTONS_KEYBOARD) return; m_Keyboard.aiButton[iButton]                    = 0u;}
    inline void ClearMouseButton      (const coreUint8    iButton)                      {WARN_IF(iButton >= CORE_INPUT_BUTTONS_MOUSE)    return; m_Mouse   .aiButton[iButton]                    = 0u;}
    inline void ClearJoystickButton   (const coreUintW iIndex, const coreUint8 iButton) {WARN_IF(iButton >= CORE_INPUT_BUTTONS_JOYSTICK) return; __CORE_INPUT_JOYSTICK(iIndex).aiButton[iButton] = 0u;}
    inline void ClearKeyboardButtonAll()                                                {std::memset(m_Keyboard.aiButton,                    0, sizeof(m_Keyboard.aiButton));}
    inline void ClearMouseButtonAll   ()                                                {std::memset(m_Mouse   .aiButton,                    0, sizeof(m_Mouse   .aiButton));}
    inline void ClearJoystickButtonAll(const coreUintW iIndex)                          {std::memset(__CORE_INPUT_JOYSTICK(iIndex).aiButton, 0, sizeof(__CORE_INPUT_JOYSTICK(iIndex).aiButton));}
    inline void ClearTouchButtonAll   ()                                                {for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i) m_aTouch[i].iButton = 0u;}
    void        ClearButtonAll        ();
    //! @}


private:
    //! update the input button interface
    //! @{
    void __UpdateButtonsStart();
    void __UpdateButtonsEnd();
    //! @}

    //! handle joystick input
    //! @{
    void      __OpenJoysticks();
    void      __CloseJoysticks();
    coreUintW __GetJoystickIndex(const SDL_JoystickID iID)const;
    //! @}
};


// ****************************************************************
// call function for each active finger
template <typename F> void CoreInput::ForEachFinger(const coreInputType iType, F&& nFunction)
{
    for(coreUintW i = 0u; i < CORE_INPUT_FINGERS; ++i)
    {
        // check finger status and call function
        if(this->GetTouchButton(i, iType))
            nFunction(i);
    }
}


#endif // _CORE_GUARD_INPUT_H_