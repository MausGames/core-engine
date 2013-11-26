//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_INPUT_H_
#define _CORE_GUARD_INPUT_H_


// ****************************************************************
// input definitions
#define CORE_INPUT_BUTTONS_KEYBOARD 232   //!< number of regarded keyboard buttons (#SDL_NUM_SCANCODES)
#define CORE_INPUT_BUTTONS_MOUSE    16    //!< number of regarded mouse buttons
#define CORE_INPUT_BUTTONS_JOYSTICK 32    //!< number of regarded joystick buttons

enum coreInputType
{
    CORE_INPUT_DATA    = 0,
    CORE_INPUT_HOLD    = 1,
    CORE_INPUT_PRESS   = 2,
    CORE_INPUT_RELEASE = 3
};


// ****************************************************************
// main input component
//! \ingroup component
class CoreInput final
{
private:
    //! keyboard structure
    struct coreKeyboard
    {
        bool aabButton[CORE_INPUT_BUTTONS_KEYBOARD][4];   //!< status of the keyboard buttons
        SDL_Scancode iLast;                               //!< last pressed keyboard button

        char cChar;                                       //!< current textinput character
    };

    //! mouse structure
    struct coreMouse
    {
        bool aabButton[CORE_INPUT_BUTTONS_MOUSE][4];   //!< status of the mouse buttons
        int iLast;                                     //!< last pressed mouse button

        coreVector2 vPosition;                         //!< absolute position of the mouse cursor
        coreVector3 vRelative;                         //!< relative movement of the mouse cursor
    };

    //! joystick structure
    struct coreJoystick
    {
        SDL_Joystick* pHandle;                            //!< joystick object handle

        bool aabButton[CORE_INPUT_BUTTONS_JOYSTICK][4];   //!< status of the joystick buttons
        int iLast;                                        //!< last pressed joystick button

        coreVector2 vRelative;                            //!< relative movement of the control axis
    };


private:
    coreKeyboard m_Keyboard;                 //!< main keyboard object
    coreMouse m_Mouse;                       //!< main mouse object
    std::vector<coreJoystick> m_aJoystick;   //!< list with joystick objects

    SDL_Cursor* m_pCursor;                   //!< hardware mouse cursor
    bool m_bCursorVisible;                   //!< status of the mouse cursor


private:
    CoreInput()noexcept;
    ~CoreInput();
    friend class Core;


public:
    //! control the mouse cursor
    //! @{
    void DefineCursor(const char* pcTexture);
    void ShowCursor(const bool& bStatus);
    inline const bool& IsCursorVisible()const {return m_bCursorVisible;}
    //! @}

    //! use mouse with other devices
    //! @{
    void UseMouseWithKeyboard(const SDL_Scancode& iLeft, const SDL_Scancode& iRight, const SDL_Scancode& iUp, const SDL_Scancode& iDown, const SDL_Scancode& iButton1, const SDL_Scancode& iButton2, const float& fSpeed);
    void UseMouseWithJoystick(const coreUint& iID, const int& iButton1, const int& iButton2, const float& fSpeed);
    //! @}

    //! get joystick data
    //! @{
    inline const char* GetJoystickName(const coreUint& iID)const {return (iID >= m_aJoystick.size()) ? (SDL_JoystickName(m_aJoystick[iID].pHandle)) : "";}
    inline coreUint GetJoystickNum()const                        {return m_aJoystick.size();}
    //! @}

    //! process input events
    //! @{
    bool ProcessEvent(const SDL_Event& Event);
    //! @}

    //! access keyboard input
    //! @{
    inline void SetKeyboardButton(const SDL_Scancode& iButton, const bool& bStatus)                    {if(iButton >= CORE_INPUT_BUTTONS_KEYBOARD) return; m_Keyboard.aabButton[iButton][0] = bStatus; if(bStatus) m_Keyboard.iLast = iButton;}
    inline void SetKeyboardChar(const char& cChar)                                                     {m_Keyboard.cChar = cChar;}
    inline const bool& GetKeyboardButton(const SDL_Scancode& iButton, const coreInputType& iType)const {return m_Keyboard.aabButton[iButton][iType];}
    inline const char& GetKeyboardChar()const                                                          {return m_Keyboard.cChar;}
    //! @}

    //! access mouse input
    //! @{
    inline void SetMouseButton(const int& iButton, const bool& bStatus)                         {if(iButton >= CORE_INPUT_BUTTONS_MOUSE) return; m_Mouse.aabButton[iButton][0] = bStatus; if(bStatus) m_Mouse.iLast = iButton;}
    inline void SetMousePosition(const coreVector2& vPosition)                                  {m_Mouse.vPosition = vPosition;}
    inline void SetMouseRelative(const coreVector2& vRelative)                                  {m_Mouse.vRelative.x = vRelative.x; m_Mouse.vRelative.y = vRelative.y;}
    inline void SetMouseWheel(const float& fValue)                                              {m_Mouse.vRelative.z = fValue;}
    inline const bool& GetMouseButton(const coreByte& iButton, const coreInputType& iType)const {return m_Mouse.aabButton[iButton][iType];}
    inline const coreVector2& GetMousePosition()const                                           {return m_Mouse.vPosition;}
    inline coreVector2 GetMouseRelative()const                                                  {return m_Mouse.vRelative.xy();}
    inline const float& GetMouseWheel()const                                                    {return m_Mouse.vRelative.z;}
    //! @}

    //! access joystick input
    //! @{
    inline void SetJoystickButton(const coreUint& iID, const int& iButton, const bool& bStatus)             {if(iID >= m_aJoystick.size() || iButton >= CORE_INPUT_BUTTONS_JOYSTICK) return; m_aJoystick[iID].aabButton[iButton][0] = bStatus; if(bStatus) m_aJoystick[iID].iLast = iButton;}
    inline void SetJoystickRelative(const coreUint& iID, const coreByte& iAxis, const float& fValue)        {if(iID >= m_aJoystick.size()) return; m_aJoystick[iID].vRelative.m[iAxis] = fValue*(iAxis ? -1.0f : 1.0f);}
    inline bool GetJoystickButton(const coreUint& iID, const int& iButton, const coreInputType& iType)const {return (iID >= m_aJoystick.size()) ? m_aJoystick[iID].aabButton[iButton][iType] : false;}
    inline coreVector2 GetJoystickRelative(const coreUint& iID)const                                        {return (iID >= m_aJoystick.size()) ? m_aJoystick[iID].vRelative : coreVector2(0.0f,0.0f);}
    //! @}

    //! get last pressed input button
    //! @{
    inline const SDL_Scancode& GetCurKeyboard()const    {return m_Keyboard.iLast;}
    inline const int& GetCurMouse()const                {return m_Mouse.iLast;}
    inline int GetCurJoystick(const coreUint& iID)const {return (iID >= m_aJoystick.size()) ? m_aJoystick[iID].iLast : -1;}
    //! @}

    //! clear status of specific input button
    //! @{
    inline void ClearKeyboardButton(const SDL_Scancode& iButton)             {for(int i = 0; i < 4; ++i) m_Keyboard.aabButton[iButton][i] = false;}
    inline void ClearMouseButton(const int& iButton)                         {for(int i = 0; i < 4; ++i) m_Mouse.aabButton[iButton][i] = false;}
    inline void ClearJoystickButton(const coreUint& iID, const int& iButton) {if(iID >= m_aJoystick.size()) return; for(int i = 0; i < 4; ++i) m_aJoystick[iID].aabButton[iButton][i] = false;}
    //! @}


private:  
    //! update and clear the input button interface
    //! @{
    void __UpdateButtons();
    void __ClearButtons();
    //! @}
};


#endif // _CORE_GUARD_INPUT_H_