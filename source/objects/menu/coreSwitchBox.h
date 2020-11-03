///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SWITCHBOX_H_
#define _CORE_GUARD_SWITCHBOX_H_

// TODO: check for inheritance of coreLabel to remove separate caption object
// TODO: separate/remove buttons from switchbox and allow "hooks" to own coreButton/coreObject2D objects
// TODO: coreStatus on entry management
// TODO: handle rotation on move (+ all menu objects)


// ****************************************************************
// menu switch-box definitions
#define CORE_SWITCHBOX_DELAY (-2.0f)


// ****************************************************************
// menu switch-box class
template <typename T> class coreSwitchBox : public coreObject2D, public coreTranslate
{
public:
    //! entry structure
    struct coreEntry final
    {
        std::string* psText;   //!< visible text
        T            tValue;   //!< associated value
    };


private:
    coreButton m_aArrow[2];            //!< selection arrows (0 = left | 1 = right)
    coreLabel  m_Caption;              //!< label displaying the current entry

    std::vector<coreEntry> m_aEntry;   //!< list with entries and associated values
    coreUintW m_iCurIndex;             //!< index of the current entry
    coreInt8  m_iUserSwitch;           //!< current entry changed by interaction (0 = unchanged | -1 = left | 1 = right)

    coreBool  m_bEndless;              //!< endless repeat behavior
    coreInt8  m_iOverride;             //!< override for selection arrows (0 = normal | 1 = always busy | -1 = always idle)
    coreTimer m_Automatic;             //!< automatic forward behavior


public:
    coreSwitchBox()noexcept;
    coreSwitchBox(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)noexcept;
    coreSwitchBox(const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)noexcept;
    virtual ~coreSwitchBox()override;

    DISABLE_COPY(coreSwitchBox)

    //! construct the switch-box
    //! @{
    void Construct(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline);
    void Construct(const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline);
    //! @}

    //! render and move the switch-box
    //! @{
    virtual void Render()override;
    virtual void Move  ()override;
    //! @}

    //! manage entries
    //! @{
    void AddEntry        (const coreChar*       pcText, const T& tValue);
    void AddEntryLanguage(const coreHashString& sKey,   const T& tValue);
    void DeleteEntry     (const coreUintW       iIndex);
    void ClearEntries();
    //! @}

    //! switch current entry
    //! @{
    void        SelectIndex(const coreUintW iIndex);
    coreBool    SelectText (const coreChar* pcText);
    coreBool    SelectValue(const T&        tValue);
    inline void SelectFirst() {if(!m_aEntry.empty()) this->SelectIndex(0u);}
    inline void SelectLast () {if(!m_aEntry.empty()) this->SelectIndex(m_aEntry.size() - 1u);}
    void        Next       ();
    void        Previous   ();
    //! @}

    //! access entries
    //! @{
    inline const coreEntry& GetEntry   (const coreUintW iIndex)const {ASSERT(iIndex      < m_aEntry.size()) return m_aEntry[iIndex];}
    inline const coreEntry& GetCurEntry()const                       {ASSERT(m_iCurIndex < m_aEntry.size()) return m_aEntry[m_iCurIndex];}
    //! @}

    //! set object properties
    //! @{
    inline void SetEndless  (const coreBool  bEndless)  {m_bEndless  = bEndless;}
    inline void SetOverride (const coreInt8  iOverride) {m_iOverride = iOverride;}
    inline void SetAutomatic(const coreFloat fSpeed)    {m_Automatic.SetSpeed(fSpeed);}
    //! @}

    //! get object properties
    //! @{
    inline       coreButton* GetArrow     (const coreUintW iIndex) {ASSERT(iIndex < 2u) return &m_aArrow[iIndex];}
    inline       coreLabel*  GetCaption   ()                       {return &m_Caption;}
    inline       coreUintW   GetNumEntries()const                  {return m_aEntry.size();}
    inline const coreUintW&  GetCurIndex  ()const                  {return m_iCurIndex;}
    inline const coreInt8&   GetUserSwitch()const                  {return m_iUserSwitch;}
    inline const coreBool&   GetEndless   ()const                  {return m_bEndless;}
    inline const coreInt8&   GetOverride  ()const                  {return m_iOverride;}
    //! @}


private:
    //! update object after modification
    //! @{
    inline void __Update()final {m_Caption.SetText(m_aEntry.empty() ? "" : m_aEntry[m_iCurIndex].psText->c_str());}
    //! @}
};


// ****************************************************************
// constructor
template <typename T> coreSwitchBox<T>::coreSwitchBox()noexcept
: coreObject2D  ()
, coreTranslate ()
, m_aArrow      ()
, m_Caption     ()
, m_aEntry      {}
, m_iCurIndex   (0u)
, m_iUserSwitch (0)
, m_bEndless    (false)
, m_iOverride   (0)
, m_Automatic   (coreTimer(1.0f, 10.0f, 1u))
{
}

template <typename T> coreSwitchBox<T>::coreSwitchBox(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)noexcept
: coreSwitchBox ()
{
    // construct on creation
    this->Construct(sIdle, sBusy, sFont, iHeight, iOutline);
}

template <typename T> coreSwitchBox<T>::coreSwitchBox(const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)noexcept
: coreSwitchBox ()
{
    // construct on creation
    this->Construct(sFont, iHeight, iOutline);
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
template <typename T> void coreSwitchBox<T>::Construct(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)
{
    // create selection arrows
    m_aArrow[0].Construct(sIdle, sBusy, sFont, iHeight, iOutline);
    m_aArrow[1].Construct(sIdle, sBusy, sFont, iHeight, iOutline);
    m_aArrow[0].SetAlpha (0.0f);
    m_aArrow[1].SetAlpha (0.0f);
    m_aArrow[0].GetCaption()->SetText("<");
    m_aArrow[1].GetCaption()->SetText(">");

    // construct remaining object
    this->Construct(sFont, iHeight, iOutline);
}

template <typename T> void coreSwitchBox<T>::Construct(const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)
{
    // create the label
    m_Caption.Construct(sFont, iHeight, iOutline);

    // init automatic forward behavior
    m_Automatic.SetValue(CORE_SWITCHBOX_DELAY);

    // enable interaction handling
    m_bFocusable = true;
}


// ****************************************************************
// render the switch-box
template <typename T> void coreSwitchBox<T>::Render()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return;

    // render selection arrow backgrounds (separately, to reduce shader changes)
    if(m_aArrow[0].GetTexture(0u)) m_aArrow[0].coreObject2D::Render();
    if(m_aArrow[1].GetTexture(0u)) m_aArrow[1].coreObject2D::Render();

    // render selection arrow texts
    m_aArrow[0].GetCaption()->SetAlpha(m_aArrow[0].GetAlpha());
    m_aArrow[1].GetCaption()->SetAlpha(m_aArrow[1].GetAlpha());
    m_aArrow[0].GetCaption()->Render();
    m_aArrow[1].GetCaption()->Render();

    // render the label
    m_Caption.SetAlpha(this->GetAlpha());
    m_Caption.Render();
}


// ****************************************************************
// move the switch-box
template <typename T> void coreSwitchBox<T>::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // override focus status
    if(m_iOverride < 0) this->SetFocused(false);

    // forward transparency
    m_aArrow[0].SetAlpha(this->GetAlpha());
    m_aArrow[1].SetAlpha(this->GetAlpha());

    // forward override
    m_aArrow[0].SetOverride(this->GetOverride());
    m_aArrow[1].SetOverride(this->GetOverride());

    // forward interaction
    if(this->IsFocused())
    {
        m_aArrow[0].Interact();
        m_aArrow[1].Interact();
    }
    else
    {
        m_aArrow[0].SetFocused(false);
        m_aArrow[1].SetFocused(false);
    }

    // reset switch status
    m_iUserSwitch = 0;

    // check for selection arrow interaction
    const coreInputType eType  = m_Automatic.GetStatus() ? CORE_INPUT_HOLD : CORE_INPUT_PRESS;
    const coreBool      bLeft  = m_aArrow[0].IsClicked(CORE_INPUT_LEFT, eType);
    const coreBool      bRight = m_aArrow[1].IsClicked(CORE_INPUT_LEFT, eType);

    if(bLeft || bRight)
    {
        // update the automatic timer
        m_Automatic.Update(1.0f);
        if(!m_Automatic.GetStatus())
        {
            const coreUintW iOldIndex = m_iCurIndex;

            // change current entry
            if(bLeft) this->Previous();
                 else this->Next();

            // set switch status
            if(m_iCurIndex != iOldIndex) m_iUserSwitch = bLeft ? -1 : 1;

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

    if(m_eUpdate)
    {
        ASSERT(this->GetSize().x > 2.0f*this->GetSize().y)

        const coreVector2 vPosition = this->GetPosition() + 0.5f*this->GetSize()*this->GetAlignment();
        const coreVector2 vSize     = coreVector2(1.0f,1.0f) * this->GetSize().y;
        const coreVector2 vOffset   = coreVector2((this->GetSize().x - this->GetSize().y)*0.5f, 0.0f);

        // update left selection arrow
        m_aArrow[0].SetPosition(vPosition - vOffset);
        m_aArrow[0].SetSize    (vSize);
        m_aArrow[0].SetCenter  (this->GetCenter());
        m_aArrow[0].SetStyle   (this->GetStyle());

        // update right selection arrow
        m_aArrow[1].SetPosition(vPosition + vOffset);
        m_aArrow[1].SetSize    (vSize);
        m_aArrow[1].SetCenter  (this->GetCenter());
        m_aArrow[1].SetStyle   (this->GetStyle());

        // update the label
        m_Caption.SetPosition(vPosition);
        m_Caption.SetCenter  (this->GetCenter());
        m_Caption.SetStyle   (this->GetStyle());
        m_Caption.Move();
    }

    // move selection arrows
    m_aArrow[0].Move();
    m_aArrow[1].Move();

    // move the 2d-object
    this->coreObject2D::Move();
}


// ****************************************************************
// add entry
template <typename T> void coreSwitchBox<T>::AddEntry(const coreChar* pcText, const T& tValue)
{
    // create new entry
    coreEntry oNewEntry;
    oNewEntry.psText = MANAGED_NEW(std::string, pcText);
    oNewEntry.tValue = tValue;

    // add entry to the list
    m_aEntry.push_back(oNewEntry);

    // update text
    if(m_aEntry.size() == 1u) this->__Update();
}

template <typename T> void coreSwitchBox<T>::AddEntryLanguage(const coreHashString& sKey, const T& tValue)
{
    // create and bind new entry
    this->AddEntry("", tValue);
    this->_BindString(m_aEntry.back().psText, sKey);
}


// ****************************************************************
// remove entry
template <typename T> void coreSwitchBox<T>::DeleteEntry(const coreUintW iIndex)
{
    ASSERT(iIndex < m_aEntry.size())
    const auto it = m_aEntry.begin() + iIndex;

    // unbind entry
    this->_UnbindString(it->psText);
    MANAGED_DELETE(it->psText)

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
        this->_UnbindString(it->psText);
        MANAGED_DELETE(it->psText)
    }

    // remove all entries and reset index
    m_aEntry.clear();
    m_iCurIndex = 0u;

    // update text
    this->__Update();
}


// ****************************************************************
// switch to specific entry
template <typename T> void coreSwitchBox<T>::SelectIndex(const coreUintW iIndex)
{
    WARN_IF(iIndex >= m_aEntry.size()) return;

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
        if(!std::strcmp(m_aEntry[i].psText->c_str(), pcText))
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
        if(m_aEntry[i].tValue == tValue)
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