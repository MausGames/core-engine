//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef CORE_INPUT_H
#define CORE_INPUT_H


// ****************************************************************
// input definitions
#define CORE_INPUT_KEYBOARD_BUTTONS 232 // SDL_NUM_SCANCODES
#define CORE_INPUT_MOUSE_BUTTONS    16
#define CORE_INPUT_JOYSTICK_BUTTONS 32

#define CORE_INPUT_PRESS(x)   {(x)[3] = false;  (x)[2] = !(x)[1]; (x)[1] = true;}
#define CORE_INPUT_RELEASE(x) {(x)[3] = (x)[1]; (x)[2] = false;   (x)[1] = false;}


// ****************************************************************
// input enumerations
enum coreInputType
{
    CORE_INPUT_HOLD    = 1,
    CORE_INPUT_PRESS   = 2,
    CORE_INPUT_RELEASE = 3,
    CORE_INPUT_DATA    = 0
};


// ****************************************************************
// main input interface
class CoreInput final
{
private:
    // keyboard struct
    struct coreKeyboard
    {
        bool aabButton[CORE_INPUT_KEYBOARD_BUTTONS][4];   //!< status of the keyboard buttons
        SDL_Scancode aiLast[2];                           //!< last pressed keyboard button

        char acChar[2];                                   //!< current keyboard/textinput character
    };

    // mouse struct
    struct coreMouse
    {
        bool aabButton[CORE_INPUT_MOUSE_BUTTONS][4];   //!< status of the mouse buttons
        int aiLast[2];                                 //!< last pressed mouse button

        coreVector2 vPosition;                         //!< absolute position of the mouse cursor
        coreVector3 vRelative;                         //!< relative movement of the mouse cursor
    };

    // joystick struct
    struct coreJoystick
    {
        SDL_Joystick* pHandle;                            //!< joystick object handle

        bool aabButton[CORE_INPUT_JOYSTICK_BUTTONS][4];   //!< status of the joystick buttons
        int aiLast[2];                                    //!< last pressed joystick button

        coreVector2 vRelative;                            //!< relative movement of the control axis
    };


private:                                   
    coreKeyboard m_Keyboard;                 //!< main keyboard object
    coreMouse m_Mouse;                       //!< main mouse object
    std::vector<coreJoystick> m_aJoystick;   //!< list with joystick objects
                                           
    coreObject2D* m_pCursor;                 //!< graphical mouse cursor object
    bool m_bCursorVisible;                   //!< status of the mouse cursor
                                           
    bool m_bActive;                          //!< input status
    

private:
    CoreInput();
    ~CoreInput();
    friend class Core;

    // update the input interface
    void __UpdateInput();

    // update the graphical mouse cursor
    void __UpdateCursor();


public:
    // control the graphical mouse cursor
    void SetCursorObject(const char* pcColorMap, const char* pcAlphaMap, const coreVector2& vSize);
    inline coreObject2D* GetCursorObject()const {return m_pCursor;}
    
    // control mouse cursor visibility
    void ShowCursor(const bool& bStatus);
    inline const bool& IsCursorVisible()const {return m_bCursorVisible;}

    // control active status
    inline void SetActive(const bool& bStatus) {m_bActive = bStatus;}
    inline const bool& IsActive()const         {return m_bActive;}

    // use mouse with other devices
    void UseMouseWithKeyboard(const SDL_Scancode& iLeft, const SDL_Scancode& iRight, const SDL_Scancode& iUp, const SDL_Scancode& iDown, const SDL_Scancode& iButton1, const SDL_Scancode& iButton2, const float& fSpeed);
    void UseMouseWithJoystick(const coreUint& iID, const int& iButton1, const int& iButton2, const float& fSpeed);

    // get joystick data
    inline const char* GetJoystickName(const coreUint& iID)const {return (iID >= m_aJoystick.size()) ? (SDL_JoystickName(m_aJoystick[iID].pHandle)) : "";}
    inline coreUint GetJoystickNum()const                        {return m_aJoystick.size();}

    // access keyboard input
    inline void SetKeyboardButton(const SDL_Scancode& iButton, const bool& bStatus)             {if(iButton >= CORE_INPUT_KEYBOARD_BUTTONS) return; m_Keyboard.aabButton[iButton][0] = bStatus; if(bStatus) m_Keyboard.aiLast[1] = iButton;}
    inline void SetKeyboardChar(const char& cChar)                                              {m_Keyboard.acChar[1] = cChar;}
    inline bool GetKeyboardButton(const SDL_Scancode& iButton, const coreInputType& iType)const {return m_bActive ? m_Keyboard.aabButton[iButton][iType] : false;}
    inline char GetKeyboardChar()const                                                          {return m_bActive ? m_Keyboard.acChar[0] : (char)NULL;}

    // access mouse input
    inline void SetMouseButton(const int& iButton, const bool& bStatus)                  {if(iButton >= CORE_INPUT_MOUSE_BUTTONS) return; m_Mouse.aabButton[iButton][0] = bStatus; if(bStatus) m_Mouse.aiLast[1] = iButton;}
    inline void SetMousePosition(const coreVector2& vPosition)                           {if(m_bActive) m_Mouse.vPosition = vPosition;}
    inline void SetMouseRelative(const coreVector2& vRelative)                           {m_Mouse.vRelative.x = vRelative.x; m_Mouse.vRelative.y = vRelative.y;}
    inline void SetMouseWheel(const float& fValue)                                       {m_Mouse.vRelative.z = fValue;}
    inline bool GetMouseButton(const coreByte& iButton, const coreInputType& iType)const {return m_bActive ? m_Mouse.aabButton[iButton][iType] : false;}
    inline const coreVector2& GetMousePosition()const                                    {return m_Mouse.vPosition;}
    inline coreVector2 GetMouseRelative()const                                           {return m_bActive ? m_Mouse.vRelative.xy() : coreVector2(0.0f,0.0f);}
    inline float GetMouseWheel()const                                                    {return m_bActive ? m_Mouse.vRelative.z : 0.0f;}

    // access joystick input
    inline void SetJoystickButton(const coreUint& iID, const int& iButton, const bool& bStatus)             {if(iID >= m_aJoystick.size() || iButton >= CORE_INPUT_JOYSTICK_BUTTONS) return; m_aJoystick[iID].aabButton[iButton][0] = bStatus; if(bStatus) m_aJoystick[iID].aiLast[1] = iButton;}
    inline void SetJoystickRelative(const coreUint& iID, const coreByte& iAxis, const float& fValue)        {if(iID >= m_aJoystick.size()) return; m_aJoystick[iID].vRelative.m[iAxis] = fValue*(iAxis ? -1.0f : 1.0f);}
    inline bool GetJoystickButton(const coreUint& iID, const int& iButton, const coreInputType& iType)const {return (m_bActive && iID >= m_aJoystick.size()) ? m_aJoystick[iID].aabButton[iButton][iType] : false;}
    inline coreVector2 GetJoystickRelative(const coreUint& iID)const                                        {return (m_bActive && iID >= m_aJoystick.size()) ? m_aJoystick[iID].vRelative : coreVector2(0.0f,0.0f);}

    // get latest input buttons
    inline SDL_Scancode GetCurKeyboard()const           {return m_bActive ? m_Keyboard.aiLast[0] : SDL_SCANCODE_UNKNOWN;}
    inline int GetCurMouse()const                       {return m_bActive ? m_Mouse.aiLast[0] : -1;}
    inline int GetCurJoystick(const coreUint& iID)const {return (m_bActive && iID >= m_aJoystick.size()) ? m_aJoystick[iID].aiLast[0] : -1;}

    // clear status of input buttons
    inline void ClearKeyboardButton(const SDL_Scancode& iButton)             {for(int i = 0; i < 4; ++i) m_Keyboard.aabButton[iButton][i] = false;}
    inline void ClearMouseButton(const int& iButton)                         {for(int i = 0; i < 4; ++i) m_Mouse.aabButton[iButton][i] = false;}
    inline void ClearJoystickButton(const coreUint& iID, const int& iButton) {if(iID >= m_aJoystick.size()) return; for(int i = 0; i < 4; ++i) m_aJoystick[iID].aabButton[iButton][i] = false;}
};


#endif // CORE_INPUT_H