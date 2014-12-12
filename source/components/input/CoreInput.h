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


// ****************************************************************
// input definitions
#define CORE_INPUT_BUTTONS_KEYBOARD (284)   //!< number of regarded keyboard buttons (#SDL_NUM_SCANCODES)
#define CORE_INPUT_BUTTONS_MOUSE    (16)    //!< number of regarded mouse buttons
#define CORE_INPUT_BUTTONS_JOYSTICK (32)    //!< number of regarded joystick buttons
#define CORE_INPUT_FINGERS          (5)     //!< maximum number of simultaneous fingers

#define CORE_INPUT_ID MIN(iID, coreUint(m_aJoystick.size()-1))

#define CORE_INPUT_KEY(k) (SDL_SCANCODE_ ## k)
typedef SDL_Scancode coreInputKey;

enum coreInputButton
{
    CORE_INPUT_LEFT   = 1,
    CORE_INPUT_MIDDLE = 2,
    CORE_INPUT_RIGHT  = 3
};

enum coreInputType
{
    CORE_INPUT_DATA    = 0,
    CORE_INPUT_HOLD    = 1,
    CORE_INPUT_PRESS   = 2,
    CORE_INPUT_RELEASE = 3
};


// ****************************************************************
// main input component
// TODO: identify joysticks by ID (maybe switch to gamepad interface)
class CoreInput final
{
private:
    //! keyboard structure
    struct coreKeyboard
    {
        bool aabButton[CORE_INPUT_BUTTONS_KEYBOARD][4];   //!< status of the keyboard buttons
        coreInputKey iLast;                               //!< last pressed keyboard button
        char cChar;                                       //!< current text-input character

        coreKeyboard()noexcept;
    };

    //! mouse structure
    struct coreMouse
    {
        bool aabButton[CORE_INPUT_BUTTONS_MOUSE][4];   //!< status of the mouse buttons
        int  iLast;                                    //!< last pressed mouse button
        coreVector2 vPosition;                         //!< absolute position of the mouse cursor
        coreVector3 vRelative;                         //!< relative movement of the mouse cursor

        coreMouse()noexcept;
    };

    //! joystick structure
    struct coreJoystick
    {
        SDL_Joystick* pHandle;                            //!< joystick object handle
        bool aabButton[CORE_INPUT_BUTTONS_JOYSTICK][4];   //!< status of the joystick buttons
        int  iLast;                                       //!< last pressed joystick button
        coreVector2 vRelative;                            //!< relative movement of the control axis

        coreJoystick()noexcept;
    };

    //! touch structure
    struct coreTouch
    {
        bool        abButton[4];   //!< status of the finger
        coreVector2 vPosition;     //!< absolute position of the finger
        coreVector2 vRelative;     //!< relative movement of the finger
        float       fPressure;     //!< current quantity of pressure applied

        coreTouch()noexcept;
    };


private:
    coreKeyboard              m_Keyboard;     //!< main keyboard object
    coreMouse                 m_Mouse;        //!< main mouse object
    std::vector<coreJoystick> m_aJoystick;    //!< list with joystick objects

    coreTouch m_aTouch[CORE_INPUT_FINGERS];   //!< array with touch objects

    SDL_Cursor* m_pCursor;                    //!< hardware mouse cursor
    bool m_bCursorVisible;                    //!< status of the mouse cursor


private:
    CoreInput()noexcept;
    ~CoreInput();
    friend class Core;


public:
    //! control the mouse cursor
    //! @{
    void DefineCursor(const char* pcPath);
    void ShowCursor  (const bool& bStatus);
    inline const bool& IsCursorVisible()const {return m_bCursorVisible;}
    //! @}

    //! use mouse with other devices
    //! @{
    void UseMouseWithKeyboard(const coreInputKey& iLeft, const coreInputKey& iRight, const coreInputKey& iUp, const coreInputKey& iDown, const coreInputKey& iButton1, const coreInputKey& iButton2, const float& fSpeed);
    void UseMouseWithJoystick(const coreUint& iID, const int& iButton1, const int& iButton2, const float& fSpeed);
    //! @}

    //! get joystick data
    //! @{
    inline const char* GetJoystickName(const coreUint& iID)const {return m_aJoystick[CORE_INPUT_ID].pHandle ? (SDL_JoystickName(m_aJoystick[CORE_INPUT_ID].pHandle)) : "";}
    inline const char* GetJoystickGUID(const coreUint& iID)const {if(m_aJoystick[CORE_INPUT_ID].pHandle) {char acGUID[64]; SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(m_aJoystick[CORE_INPUT_ID].pHandle), acGUID, ARRAY_SIZE(acGUID)); return PRINT("%s", acGUID);} return "";}
    inline coreUint    GetJoystickNum ()const                    {return m_aJoystick.size()-1;}
    //! @}

    //! process input events
    //! @{
    bool ProcessEvent(const SDL_Event& Event);
    //! @}

    //! access keyboard input
    //! @{
    inline void        SetKeyboardButton(const coreInputKey& iButton, const bool& bStatus)             {WARN_IF(iButton >= CORE_INPUT_BUTTONS_KEYBOARD) return; m_Keyboard.aabButton[iButton][0] = bStatus; if(bStatus) m_Keyboard.iLast = iButton;}
    inline void        SetKeyboardChar  (const char& cChar)                                            {m_Keyboard.cChar = cChar;}
    inline const bool& GetKeyboardButton(const coreInputKey& iButton, const coreInputType& iType)const {return m_Keyboard.aabButton[iButton][iType];}
    inline const char& GetKeyboardChar  ()const                                                        {return m_Keyboard.cChar;}
    //! @}

    //! access mouse input
    //! @{
    inline void               SetMouseButton  (const int& iButton, const bool& bStatus)                  {WARN_IF(iButton >= CORE_INPUT_BUTTONS_MOUSE) return; m_Mouse.aabButton[iButton][0] = bStatus; if(bStatus) m_Mouse.iLast = iButton;}
    inline void               SetMousePosition(const coreVector2& vPosition)                             {m_Mouse.vPosition   = vPosition;}
    inline void               SetMouseRelative(const coreVector2& vRelative)                             {m_Mouse.vRelative.x = vRelative.x; m_Mouse.vRelative.y = vRelative.y;}
    inline void               SetMouseWheel   (const float& fValue)                                      {m_Mouse.vRelative.z = fValue;}
    inline const bool&        GetMouseButton  (const coreByte& iButton, const coreInputType& iType)const {return m_Mouse.aabButton[iButton][iType];}
    inline const coreVector2& GetMousePosition()const                                                    {return m_Mouse.vPosition;}
    inline const coreVector3& GetMouseRelative()const                                                    {return m_Mouse.vRelative;}
    inline const float&       GetMouseWheel   ()const                                                    {return m_Mouse.vRelative.z;}
    //! @}

    //! access joystick input
    //! @{
    inline void               SetJoystickButton  (const coreUint& iID, const int& iButton, const bool& bStatus)             {WARN_IF(iButton >= CORE_INPUT_BUTTONS_JOYSTICK) return; m_aJoystick[CORE_INPUT_ID].aabButton[iButton][0] = bStatus; if(bStatus) m_aJoystick[CORE_INPUT_ID].iLast = iButton;}
    inline void               SetJoystickRelative(const coreUint& iID, const coreByte& iAxis, const float& fValue)          {m_aJoystick[CORE_INPUT_ID].vRelative.m[iAxis] = fValue;}
    inline const bool&        GetJoystickButton  (const coreUint& iID, const int& iButton, const coreInputType& iType)const {return m_aJoystick[CORE_INPUT_ID].aabButton[iButton][iType];}
    inline const coreVector2& GetJoystickRelative(const coreUint& iID)const                                                 {return m_aJoystick[CORE_INPUT_ID].vRelative;}
    //! @}

    //! access touch input
    //! @{
    inline void               SetTouchButton  (const coreUint& iID, const bool& bStatus)             {WARN_IF(iID >= CORE_INPUT_FINGERS) return; m_aTouch[iID].abButton[0] = bStatus;}
    inline void               SetTouchPosition(const coreUint& iID, const coreVector2& vPosition)    {WARN_IF(iID >= CORE_INPUT_FINGERS) return; m_aTouch[iID].vPosition   = vPosition;}
    inline void               SetTouchRelative(const coreUint& iID, const coreVector2& vRelative)    {WARN_IF(iID >= CORE_INPUT_FINGERS) return; m_aTouch[iID].vRelative   = vRelative;}
    inline void               SetTouchPressure(const coreUint& iID, const float& fPressure)          {WARN_IF(iID >= CORE_INPUT_FINGERS) return; m_aTouch[iID].fPressure   = fPressure;}
    inline const bool&        GetTouchButton  (const coreUint& iID, const coreInputType& iType)const {return m_aTouch[iID].abButton[iType];}
    inline const coreVector2& GetTouchPosition(const coreUint& iID)const                             {return m_aTouch[iID].vPosition;}
    inline const coreVector2& GetTouchRelative(const coreUint& iID)const                             {return m_aTouch[iID].vRelative;}
    inline const float&       GetTouchPressure(const coreUint& iID)const                             {return m_aTouch[iID].fPressure;}
    template <typename F> void ForEachFinger(const coreInputType& iType, F&& nFunction);
    //! @}

    //! get last pressed input button
    //! @{
    inline const coreInputKey& GetCurKeyboard()const                    {return m_Keyboard.iLast;}
    inline const int&          GetCurMouse   ()const                    {return m_Mouse.iLast;}
    inline const int&          GetCurJoystick(const coreUint& iID)const {return m_aJoystick[CORE_INPUT_ID].iLast;}
    //! @}

    //! clear status of specific input button
    //! @{
    inline void ClearKeyboardButton(const coreInputKey& iButton)             {WARN_IF(iButton >= CORE_INPUT_BUTTONS_KEYBOARD) return; for(int i = 0; i < 4; ++i) m_Keyboard.aabButton[iButton][i]                 = false;}
    inline void ClearMouseButton   (const int&          iButton)             {WARN_IF(iButton >= CORE_INPUT_BUTTONS_MOUSE)    return; for(int i = 0; i < 4; ++i) m_Mouse.aabButton[iButton][i]                    = false;}
    inline void ClearJoystickButton(const coreUint& iID, const int& iButton) {WARN_IF(iButton >= CORE_INPUT_BUTTONS_JOYSTICK) return; for(int i = 0; i < 4; ++i) m_aJoystick[CORE_INPUT_ID].aabButton[iButton][i] = false;}
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
template <typename F> void CoreInput::ForEachFinger(const coreInputType& iType, F&& nFunction)
{
    for(coreUint i = 0; i < CORE_INPUT_FINGERS; ++i)
    {
        // check finger status and call function
        if(Core::Input->GetTouchButton(i, iType))
            nFunction(i);
    }
}


#endif // _CORE_GUARD_INPUT_H_