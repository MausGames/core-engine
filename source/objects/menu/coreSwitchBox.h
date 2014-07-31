//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SWITCHBOX_H_
#define _CORE_GUARD_SWITCHBOX_H_


// ****************************************************************
// menu switch-box definitions
#define CORE_SWITCHBOX_RESERVE 16u
#define CORE_SWITCHBOX_DELAY   -2.0f


// ****************************************************************
// menu switch-box class
// TODO: check for inheritance of coreLabel to remove separate caption object
// TODO: separate/remove buttons from switchbox and allow "hooks" to own coreButton/coreObject2D objects
// TODO: coreError on entry management
template <typename T> class coreSwitchBox final : public coreObject2D, public coreTranslate
{
public:
    //! internal type
    typedef std::pair<std::string, T> coreEntry;


private:
    coreButton m_aArrow[2];            //!< selection arrows (0 = left | 1 = right)
    coreLabel  m_Caption;              //!< label displaying the current entry
                                  
    std::vector<coreEntry> m_aEntry;   //!< list with entries and associated values
    coreUint m_iCurIndex;              //!< index of the current entry

    bool      m_bEndless;              //!< endless repeat behavior
    coreTimer m_Automatic;             //!< automatic forward behavior


public:
    coreSwitchBox()noexcept;
    coreSwitchBox(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept;
    coreSwitchBox(const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept;
    ~coreSwitchBox();

    //! construct the switch-box
    //! @{
    void Construct(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength);
    void Construct(const char* pcFont, const int& iHeight, const coreUint& iLength);
    //! @}

    //! render and move the switch-box
    //! @{
    void Render()override;
    void Move  ()override;
    //! @}

    //! manage entries
    //! @{
    void AddEntry        (const char* pcText, const T& Value);
    void AddEntryLanguage(const char* pcKey,  const T& Value);
    void DeleteEntry     (const coreUint& iIndex);
    void ClearEntries();
    //! @}

    //! switch current entry
    //! @{
    void Select(const coreUint& iIndex);
    void Next    ();
    void Previous();
    //! @}

    //! access entries
    //! @{
    inline const coreEntry& GetEntry   (const coreUint& iIndex)const {ASSERT(iIndex      < m_aEntry.size()) return m_aEntry[iIndex];}
    inline const coreEntry& GetCurEntry()const                       {ASSERT(m_iCurIndex < m_aEntry.size()) return m_aEntry[m_iCurIndex];}
    //! @}

    //! set object properties
    //! @{
    inline void SetEndless  (const bool&  bEndless) {m_bEndless = bEndless;}
    inline void SetAutomatic(const float& fSpeed)   {m_Automatic.SetSpeed(fSpeed);}
    //! @}

    //! get object properties
    //! @{
    inline coreButton*     GetArrow     (const coreUint& iIndex) {ASSERT(iIndex < 2) return &m_aArrow[iIndex];}
    inline coreLabel*      GetCaption   ()                       {return &m_Caption;}
    inline coreUint        GetNumEntries()const                  {return m_aEntry.size();}
    inline const coreUint& GetCurIndex  ()const                  {return m_iCurIndex;}
    inline const bool&     GetEndless   ()const                  {return m_bEndless;}
    //! @}


private:
    DISABLE_COPY(coreSwitchBox)

    //! update object after modification
    //! @{
    inline void __Update()override {m_Caption.SetText(m_aEntry.empty() ? "" : m_aEntry[m_iCurIndex].first.c_str());}
    //! @}
};


// ****************************************************************    
// constructor
template <typename T> coreSwitchBox<T>::coreSwitchBox()noexcept
: m_iCurIndex (0)
, m_bEndless  (false)
, m_Automatic (coreTimer(1.0f, 10.0f, 1))
{
}

template <typename T> coreSwitchBox<T>::coreSwitchBox(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept
: coreSwitchBox ()
{
    // construct on creation
    this->Construct(pcIdle, pcBusy, pcFont, iHeight, iLength);
}

template <typename T> coreSwitchBox<T>::coreSwitchBox(const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept
: coreSwitchBox ()
{
    // construct on creation
    this->Construct(pcFont, iHeight, iLength);
}


// ****************************************************************    
// destructor
template <typename T> coreSwitchBox<T>::~coreSwitchBox()
{
    // remove all entries
    this->ClearEntries();
}


// ****************************************************************  
// construct the switch-box
template <typename T> void coreSwitchBox<T>::Construct(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength)
{
    // create selection arrows
    m_aArrow[0].Construct(pcIdle, pcBusy, pcFont, iHeight, 2);
    m_aArrow[1].Construct(pcIdle, pcBusy, pcFont, iHeight, 2);
    m_aArrow[0].GetCaption()->SetText("<");
    m_aArrow[1].GetCaption()->SetText(">");

    // construct remaining object 
    this->Construct(pcFont, iHeight, iLength);
}

template <typename T> void coreSwitchBox<T>::Construct(const char* pcFont, const int& iHeight, const coreUint& iLength)
{
    ASSERT(iLength)

    // create the label
    m_Caption.Construct(pcFont, iHeight, iLength);

    // reserve memory for entries
    m_aEntry.reserve(CORE_SWITCHBOX_RESERVE);

    // init automatic forward behavior
    m_Automatic.SetValue(CORE_SWITCHBOX_DELAY);
}


// ****************************************************************    
// render the switch-box
template <typename T> void coreSwitchBox<T>::Render()
{
    // forward transparency 
    // TODO
    //m_aArrow[0].SetAlpha(this->GetAlpha());
    //m_aArrow[1].SetAlpha(this->GetAlpha());
    //m_Caption.SetAlpha(this->GetAlpha());

    // render selection arrows
    m_aArrow[0].Render();
    m_aArrow[1].Render();

    // render the label
    m_Caption.Render();
}    


// ****************************************************************    
// move the switch-box
// TODO: handle rotation (+ all menu objects)
template <typename T> void coreSwitchBox<T>::Move()
{
    // forward object interaction
    if(this->IsFocused())
    {
        m_aArrow[0].Interact();
        m_aArrow[1].Interact();
    }
    else
    {
        m_aArrow[0].SetFocus(false);
        m_aArrow[1].SetFocus(false);
    }

    // check for selection arrow interaction
    const bool abStatus[2] = {m_aArrow[0].IsClicked(CORE_INPUT_LEFT, CORE_INPUT_HOLD),
                              m_aArrow[1].IsClicked(CORE_INPUT_LEFT, CORE_INPUT_HOLD)};

    if(abStatus[0] || abStatus[1])
    {
        // update the automatic timer
        m_Automatic.Update(1.0f);
        if(!m_Automatic.GetStatus())
        {
            // change current entry
            if(abStatus[0]) this->Previous();
                       else this->Next();

            // manually loop the automatic timer
            m_Automatic.Play((m_Automatic.GetValue(CORE_TIMER_GET_NORMAL) > 0.0f) ? CORE_TIMER_PLAY_RESET : CORE_TIMER_PLAY_CURRENT);
        }
    }
    else
    {
        // reset the automatic timer
        if(m_Automatic.GetStatus())
        {
            m_Automatic.Pause(); 
            m_Automatic.SetValue(CORE_SWITCHBOX_DELAY);
        }
    }

    if(m_iUpdate)
    {
        ASSERT(this->GetSize().x > 2.0f*this->GetSize().y)

        const coreVector2 vPosition = this->GetPosition() + 0.5f*this->GetSize()*this->GetAlignment();
        const coreVector2 vSize     = coreVector2(this->GetSize().y, this->GetSize().y);
        const coreVector2 vOffset   = coreVector2((this->GetSize().x - this->GetSize().y)*0.5f, 0.0f);

        // update left selection arrow
        m_aArrow[0].SetPosition(vPosition - vOffset);
        m_aArrow[0].SetSize(vSize);
        m_aArrow[0].SetCenter(this->GetCenter());
        
        // update right selection arrow
        m_aArrow[1].SetPosition(vPosition + vOffset);
        m_aArrow[1].SetSize(vSize);
        m_aArrow[1].SetCenter(this->GetCenter());
        
        // update the label
        m_Caption.SetPosition(vPosition);
        m_Caption.SetCenter(this->GetCenter());
        m_Caption.Move();
    }

    // move selection arrows
    m_aArrow[0].Move();
    m_aArrow[1].Move();

    // move the 2d-object
    coreObject2D::Move();
}


// ****************************************************************    
// add entry
template <typename T> void coreSwitchBox<T>::AddEntry(const char* pcText, const T& Value)
{
    ASSERT(m_aEntry.size() < m_aEntry.capacity())

    // create new entry
    m_aEntry.push_back(coreEntry(pcText, Value));

    // update text
    if(m_aEntry.size() == 1) this->__Update();
}

template <typename T> void coreSwitchBox<T>::AddEntryLanguage(const char* pcKey, const T& Value)
{
    // create and bind new entry
    this->AddEntry("", Value);
    this->_BindString(&m_aEntry.back().first, pcKey);
}


// ****************************************************************    
// remove entry
template <typename T> void coreSwitchBox<T>::DeleteEntry(const coreUint& iIndex)
{
    ASSERT(iIndex < m_aEntry.size())
    auto it = m_aEntry.begin()+iIndex;

    // unbind entry
    this->_UnbindString(&it->first);

    // remove entry and update index
    m_aEntry.erase(it); 
    if(iIndex < m_iCurIndex) --m_iCurIndex;

    // update text
    this->__Update();
}


// ****************************************************************    
// remove all entries
template <typename T> void coreSwitchBox<T>::ClearEntries()
{
    // unbind all entries
    FOR_EACH(it, m_aEntry)
        this->_UnbindString(&it->first);

    // remove all entries and reset index
    m_aEntry.clear(); 
    m_iCurIndex = 0;

    // update text
    this->__Update();
}


// ****************************************************************
// switch to specific entry
template <typename T> void coreSwitchBox<T>::Select(const coreUint& iIndex)
{
    ASSERT(iIndex < m_aEntry.size())

    // save new index
    if(m_iCurIndex == iIndex) return;
    m_iCurIndex = iIndex; 
    
    // update text
    this->__Update();
}


// ****************************************************************    
// switch to next entry
template <typename T> void coreSwitchBox<T>::Next()
{
    ASSERT_IF(m_aEntry.empty()) return;

    // increase current index
    if(++m_iCurIndex >= m_aEntry.size()) 
        m_iCurIndex = m_bEndless ? 0 : (m_aEntry.size()-1);

    // update text
    this->__Update();
}


// ****************************************************************
// switch to previous entry
template <typename T> void coreSwitchBox<T>::Previous()
{
    ASSERT_IF(m_aEntry.empty()) return;

    // decrease current index
    if(--m_iCurIndex >= m_aEntry.size())
        m_iCurIndex = m_bEndless ? (m_aEntry.size()-1) : 0;

    // update text
    this->__Update();
}


#endif // _CORE_GUARD_SWITCHBOX_H_