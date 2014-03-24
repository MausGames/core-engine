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
#define CORE_SWITCHBOX_DELAY -2.0f


// ****************************************************************
// menu switch-box class
template <typename T> class coreSwitchBox final : public coreObject2D, public coreTranslate
{
public:
    //! internal type
    typedef std::pair<std::string, T> coreEntry;


private:
    coreButton* m_apArrow[2];          //!< selection arrows (0 = left | 1 = right)
    coreLabel* m_pCaption;             //!< label displaying the current entry
                                  
    std::vector<coreEntry> m_aEntry;   //!< list with entries and associated values
    coreUint m_iCurIndex;              //!< index of the current entry

    bool m_bEndless;                   //!< endless repeat behavior
    coreTimer m_pAutomatic;            //!< automatic forward behavior


public:
    coreSwitchBox(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength, const coreUint& iReserve)noexcept;
    ~coreSwitchBox();

    //! render and move the switch-box
    //! @{
    void Render()override;
    void Move()override;
    //! @}

    //! manage entries
    //! @{
    void AddEntry(const char* pcEntry, const T& Value);
    void AddEntryLanguage(const char* pcKey, const T& Value);
    void DeleteEntry(const coreUint& iIndex);
    void ClearEntries();
    //! @}

    //! switch current entry
    //! @{
    void Select(const coreUint& iIndex);
    void Next();
    void Previous();
    //! @}

    //! access entries
    //! @{
    inline const coreEntry& GetEntry(const coreUint& iIndex)const {SDL_assert(iIndex      < m_aEntry.size()); return m_aEntry[iIndex];}
    inline const coreEntry& GetCurEntry()const                    {SDL_assert(m_iCurIndex < m_aEntry.size()); return m_aEntry[m_iCurIndex];}
    //! @}

    //! set object properties
    //! @{
    inline void SetEndless(const bool& bEndless)  {m_bEndless = bEndless;}
    inline void SetAutomatic(const float& fSpeed) {m_pAutomatic.SetSpeed(fSpeed);}
    //! @}

    //! get object properties
    //! @{
    inline coreButton* GetArrow(const coreUint& iIndex)const {SDL_assert(iIndex < 2); return m_apArrow[iIndex];}
    inline coreLabel* GetCaption()const                      {return m_pCaption;}
    inline coreUint GetNumEntries()const                     {return m_aEntry.size();}
    inline const coreUint& GetCurIndex()const                {return m_iCurIndex;}
    inline const bool& GetEndless()const                     {return m_bEndless;}
    //! @}


private:
    DISABLE_COPY(coreSwitchBox)

    //! update object after modification
    //! @{
    inline void __Update()override {m_pCaption->SetText(m_aEntry.empty() ? "" : m_aEntry[m_iCurIndex].first.c_str());}
    //! @}
};


// ****************************************************************    
// constructor
template <typename T> coreSwitchBox<T>::coreSwitchBox(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength, const coreUint& iReserve)
: m_iCurIndex  (0)
, m_bEndless   (false)
, m_pAutomatic (coreTimer(1.0f, 10.0f, 1))
{
    SDL_assert(iLength);

    // create selection arrows
    m_apArrow[0] = new coreButton(pcIdle, pcBusy, pcFont, iHeight, 2);
    m_apArrow[1] = new coreButton(pcIdle, pcBusy, pcFont, iHeight, 2);
    m_apArrow[0]->GetCaption()->SetText("<");
    m_apArrow[1]->GetCaption()->SetText(">");

    // create the caption
    m_pCaption = new coreLabel(pcFont, iHeight, iLength);

    // reserve memory for entries
    m_aEntry.reserve(iReserve);

    // init automatic forward behavior
    m_pAutomatic.SetCurrent(CORE_SWITCHBOX_DELAY);
}


// ****************************************************************    
// destructor
template <typename T> coreSwitchBox<T>::~coreSwitchBox()
{
    // remove all entries
    this->ClearEntries();

    // delete selection arrows
    SAFE_DELETE(m_apArrow[0])
    SAFE_DELETE(m_apArrow[1])

    // delete the caption
    SAFE_DELETE(m_pCaption)
}    


// ****************************************************************    
// render the switch-box
template <typename T> void coreSwitchBox<T>::Render()
{
    // render selection arrows
    m_apArrow[0]->Render();
    m_apArrow[1]->Render();

    // render the caption
    m_pCaption->Render();
}    


// ****************************************************************    
// move the switch-box
// TODO: handle rotation (+ all menu objects)
template <typename T> void coreSwitchBox<T>::Move()
{
    // forward object interaction
    if(this->IsFocused())
    {
        m_apArrow[0]->Interact();
        m_apArrow[1]->Interact();
    }
    else
    {
        m_apArrow[0]->SetFocus(false);
        m_apArrow[1]->SetFocus(false);
    }

    // check for selection arrow interaction
    const bool abStatus[2] = {m_apArrow[0]->IsClicked(CORE_INPUT_LEFT, CORE_INPUT_HOLD),
                              m_apArrow[1]->IsClicked(CORE_INPUT_LEFT, CORE_INPUT_HOLD)};

    if(abStatus[0] || abStatus[1])
    {
        // update the automatic timer
        m_pAutomatic.Update(1.0f);
        if(!m_pAutomatic.GetStatus())
        {
            // change current entry
            if(abStatus[0]) this->Previous();
                       else this->Next();

            // manually loop the automatic timer
            m_pAutomatic.Play(m_pAutomatic.GetCurrent(false) > 0.0f);
        }
    }
    else
    {
        // reset the automatic timer
        if(m_pAutomatic.GetStatus())
        {
            m_pAutomatic.Pause(); 
            m_pAutomatic.SetCurrent(CORE_SWITCHBOX_DELAY);
        }
    }

    // forward transparency 
    m_apArrow[0]->SetAlpha(this->GetAlpha());
    m_apArrow[1]->SetAlpha(this->GetAlpha());
    m_pCaption->SetAlpha(this->GetAlpha());

    if(m_iUpdate)
    {
        SDL_assert(this->GetSize().x > 2.0f*this->GetSize().y);

        const coreVector2 vPosition = this->GetPosition() + 0.5f*this->GetSize()*this->GetAlignment();
        const coreVector2 vSize     = coreVector2(this->GetSize().y, this->GetSize().y);
        const coreVector2 vOffset   = coreVector2((this->GetSize().x - this->GetSize().y)*0.5f, 0.0f);

        // update left selection arrow
        m_apArrow[0]->SetPosition(vPosition - vOffset);
        m_apArrow[0]->SetSize(vSize);
        m_apArrow[0]->SetCenter(this->GetCenter());
        
        // update right selection arrow
        m_apArrow[1]->SetPosition(vPosition + vOffset);
        m_apArrow[1]->SetSize(vSize);
        m_apArrow[1]->SetCenter(this->GetCenter());
        
        // update the caption
        m_pCaption->SetPosition(vPosition);
        m_pCaption->SetCenter(this->GetCenter());
        m_pCaption->Move();
    }

    // move selection arrows
    m_apArrow[0]->Move();
    m_apArrow[1]->Move();

    // move the 2d-object
    coreObject2D::Move();
}


// ****************************************************************    
// add entry
template <typename T> void coreSwitchBox<T>::AddEntry(const char* pcEntry, const T& Value)
{
    SDL_assert(m_aEntry.size() < m_aEntry.capacity());

    // create new entry
    m_aEntry.push_back(coreEntry(pcEntry, Value));

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
    SDL_assert(iIndex < m_aEntry.size());
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
    SDL_assert(iIndex < m_aEntry.size()); 

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