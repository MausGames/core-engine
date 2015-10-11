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

// TODO: check for inheritance of coreLabel to remove separate caption object
// TODO: separate/remove buttons from switchbox and allow "hooks" to own coreButton/coreObject2D objects
// TODO: coreError on entry management
// TODO: handle rotation on move (+ all menu objects)


// ****************************************************************
// menu switch-box definitions
#define CORE_SWITCHBOX_DELAY (-2.0f)


// ****************************************************************
// menu switch-box class
template <typename T> class coreSwitchBox final : public coreObject2D, public coreTranslate
{
public:
    //! internal type
    using coreEntry = std::pair<std::string*, T>;


private:
    coreButton m_aArrow[2];            //!< selection arrows (0 = left | 1 = right)
    coreLabel  m_Caption;              //!< label displaying the current entry

    std::vector<coreEntry> m_aEntry;   //!< list with entries and associated values
    coreUintW m_iCurIndex;             //!< index of the current entry

    coreBool  m_bEndless;              //!< endless repeat behavior
    coreTimer m_Automatic;             //!< automatic forward behavior


public:
    coreSwitchBox()noexcept;
    coreSwitchBox(const coreChar* pcIdle, const coreChar* pcBusy, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength)noexcept;
    coreSwitchBox(const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength)noexcept;
    ~coreSwitchBox();

    DISABLE_COPY(coreSwitchBox)

    //! construct the switch-box
    //! @{
    void Construct(const coreChar* pcIdle, const coreChar* pcBusy, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength);
    void Construct(const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength);
    //! @}

    //! render and move the switch-box
    //! @{
    void Render()override;
    void Move  ()override;
    //! @}

    //! manage entries
    //! @{
    void AddEntry        (const coreChar*  pcText, const T& tValue);
    void AddEntryLanguage(const coreChar*  pcKey,  const T& tValue);
    void DeleteEntry     (const coreUintW& iIndex);
    void ClearEntries();
    //! @}

    //! switch current entry
    //! @{
    void     SelectIndex(const coreUintW& iIndex);
    coreBool SelectText (const coreChar*  pcText);
    coreBool SelectValue(const T&         tValue);
    void Next    ();
    void Previous();
    //! @}

    //! access entries
    //! @{
    inline const coreEntry& GetEntry   (const coreUintW& iIndex)const {ASSERT(iIndex      < m_aEntry.size()) return m_aEntry[iIndex];}
    inline const coreEntry& GetCurEntry()const                        {ASSERT(m_iCurIndex < m_aEntry.size()) return m_aEntry[m_iCurIndex];}
    //! @}

    //! interact with the switch-box
    //! @{
    inline coreBool IsClickedArrow(const coreUint8 iButton = CORE_INPUT_LEFT, const coreInputType iType = CORE_INPUT_PRESS)const {return (m_aArrow[0].IsClicked(iButton, iType) || m_aArrow[1].IsClicked(iButton, iType)) ? true : false;}
    inline coreBool IsFocusedArrow()const                                                                                        {return (m_aArrow[0].IsFocused()               || m_aArrow[1].IsFocused())               ? true : false;}
    //! @}

    //! set object properties
    //! @{
    inline void SetEndless  (const coreBool&  bEndless) {m_bEndless = bEndless;}
    inline void SetAutomatic(const coreFloat& fSpeed)   {m_Automatic.SetSpeed(fSpeed);}
    //! @}

    //! get object properties
    //! @{
    inline       coreButton* GetArrow     (const coreUintW& iIndex) {ASSERT(iIndex < 2u) return &m_aArrow[iIndex];}
    inline       coreLabel*  GetCaption   ()                        {return &m_Caption;}
    inline       coreUintW   GetNumEntries()const                   {return m_aEntry.size();}
    inline const coreUintW&  GetCurIndex  ()const                   {return m_iCurIndex;}
    inline const coreBool&   GetEndless   ()const                   {return m_bEndless;}
    //! @}


private:
    //! update object after modification
    //! @{
    inline void __Update()override {m_Caption.SetText(m_aEntry.empty() ? "" : m_aEntry[m_iCurIndex].first->c_str());}
    //! @}
};


// ****************************************************************
// constructor
template <typename T> coreSwitchBox<T>::coreSwitchBox()noexcept
: coreObject2D  ()
, coreTranslate ()
, m_aArrow      {}
, m_Caption     ()
, m_aEntry      {}
, m_iCurIndex   (0)
, m_bEndless    (false)
, m_Automatic   (coreTimer(1.0f, 10.0f, 1u))
{
}

template <typename T> coreSwitchBox<T>::coreSwitchBox(const coreChar* pcIdle, const coreChar* pcBusy, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength)noexcept
: coreSwitchBox ()
{
    // construct on creation
    this->Construct(pcIdle, pcBusy, pcFont, iHeight, iOutline, iLength);
}

template <typename T> coreSwitchBox<T>::coreSwitchBox(const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength)noexcept
: coreSwitchBox ()
{
    // construct on creation
    this->Construct(pcFont, iHeight, iOutline, iLength);
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
template <typename T> void coreSwitchBox<T>::Construct(const coreChar* pcIdle, const coreChar* pcBusy, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength)
{
    // create selection arrows
    m_aArrow[0].Construct(pcIdle, pcBusy, pcFont, iHeight, iOutline, 0u);
    m_aArrow[1].Construct(pcIdle, pcBusy, pcFont, iHeight, iOutline, 0u);
    m_aArrow[0].SetAlpha (0.0f);
    m_aArrow[1].SetAlpha (0.0f);
    m_aArrow[0].GetCaption()->SetText("<");
    m_aArrow[1].GetCaption()->SetText(">");

    // construct remaining object
    this->Construct(pcFont, iHeight, iOutline, iLength);
}

template <typename T> void coreSwitchBox<T>::Construct(const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength)
{
    // create the label
    m_Caption.Construct(pcFont, iHeight, iOutline, iLength);

    // init automatic forward behavior
    m_Automatic.SetValue(CORE_SWITCHBOX_DELAY);
}


// ****************************************************************
// render the switch-box
template <typename T> void coreSwitchBox<T>::Render()
{
    // forward transparency
    // TODO: check implementation
    //m_aArrow[0].SetAlpha(this->GetAlpha());
    //m_aArrow[1].SetAlpha(this->GetAlpha());

    // render selection arrows
    m_aArrow[0].Render();
    m_aArrow[1].Render();

    // render the label
    m_Caption.SetAlpha(this->GetAlpha());
    m_Caption.Render();
}


// ****************************************************************
// move the switch-box
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
    const coreBool abStatus[2] = {m_aArrow[0].IsClicked(CORE_INPUT_LEFT, CORE_INPUT_HOLD),
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
        m_aArrow[0].SetSize    (vSize);
        m_aArrow[0].SetCenter  (this->GetCenter());

        // update right selection arrow
        m_aArrow[1].SetPosition(vPosition + vOffset);
        m_aArrow[1].SetSize    (vSize);
        m_aArrow[1].SetCenter  (this->GetCenter());

        // update the label
        m_Caption.SetPosition(vPosition);
        m_Caption.SetCenter  (this->GetCenter());
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
template <typename T> void coreSwitchBox<T>::AddEntry(const coreChar* pcText, const T& tValue)
{
    // create new entry
    m_aEntry.emplace_back(new std::string(pcText), tValue);

    // update text
    if(m_aEntry.size() == 1u) this->__Update();
}

template <typename T> void coreSwitchBox<T>::AddEntryLanguage(const coreChar* pcKey, const T& tValue)
{
    // create and bind new entry
    this->AddEntry("", tValue);
    this->_BindString(m_aEntry.back().first, pcKey);
}


// ****************************************************************
// remove entry
template <typename T> void coreSwitchBox<T>::DeleteEntry(const coreUintW& iIndex)
{
    ASSERT(iIndex < m_aEntry.size())
    auto it = m_aEntry.begin() + iIndex;

    // unbind entry
    this->_UnbindString(it->first);
    SAFE_DELETE(it->first)

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
    {
        this->_UnbindString(it->first);
        SAFE_DELETE(it->first)
    }

    // remove all entries and reset index
    m_aEntry.clear();
    m_iCurIndex = 0u;

    // update text
    this->__Update();
}


// ****************************************************************
// switch to specific entry
template <typename T> void coreSwitchBox<T>::SelectIndex(const coreUintW& iIndex)
{
    ASSERT(iIndex < m_aEntry.size())

    // save new index
    if(m_iCurIndex == iIndex) return;
    m_iCurIndex = iIndex;

    // update text
    this->__Update();
}


template <typename T> coreBool coreSwitchBox<T>::SelectText(const coreChar* pcText)
{
    // loop through all entries
    for(coreUintW i = 0u, ie = m_aEntry.size(); i < ie; ++i)
    {
        // search and select specific text
        if(!std::strcmp(m_aEntry[i].first->c_str(), pcText))
        {
            this->SelectIndex(i);
            return true;
        }
    }

    // text not found
    return false;
}

template <typename T> coreBool coreSwitchBox<T>::SelectValue(const T& tValue)
{
    // loop through all entries
    for(coreUintW i = 0u, ie = m_aEntry.size(); i < ie; ++i)
    {
        // search and select specific value
        if(m_aEntry[i].second == tValue)
        {
            this->SelectIndex(i);
            return true;
        }
    }

    // value not found
    return false;
}


// ****************************************************************
// switch to next entry
template <typename T> void coreSwitchBox<T>::Next()
{
    WARN_IF(m_aEntry.empty()) return;

    // increase current index
    if(++m_iCurIndex >= m_aEntry.size())
        m_iCurIndex = m_bEndless ? 0u : (m_aEntry.size() - 1u);

    // update text
    this->__Update();
}


// ****************************************************************
// switch to previous entry
template <typename T> void coreSwitchBox<T>::Previous()
{
    WARN_IF(m_aEntry.empty()) return;

    // decrease current index
    if(--m_iCurIndex >= m_aEntry.size())
        m_iCurIndex = m_bEndless ? (m_aEntry.size() - 1u) : 0u;

    // update text
    this->__Update();
}


// ****************************************************************
// default switch-box types
using coreSwitchBoxU8  = coreSwitchBox<coreUint8>;
using coreSwitchBoxStr = coreSwitchBox<std::string>;


#endif // _CORE_GUARD_SWITCHBOX_H_