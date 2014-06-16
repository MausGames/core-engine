//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LANGUAGE_H_
#define _CORE_GUARD_LANGUAGE_H_


// ****************************************************************
// translation interface
class coreTranslate
{
private:
    coreLanguage* m_pLanguage;                            //!< associated language file
    coreLookup<std::string*, std::string> m_apsPointer;   //!< own string pointers connected with keys <own, key>


public:
    coreTranslate()noexcept : m_pLanguage (NULL) {}
    virtual ~coreTranslate() {this->ChangeLanguage(NULL); m_apsPointer.clear();}
    friend class coreLanguage;

    //! change associated language file
    //! @{
    void ChangeLanguage(coreLanguage* pLanguage);
    //! @}

    
protected:
    //! bind and unbind own string pointer
    //! @{
    void _BindString(std::string* psString, const char* pcKey);
    void _UnbindString(std::string* psString);
    //! @}


private:
    DISABLE_COPY(coreTranslate)

    //! update object after modification
    //! @{
    virtual void __Update() = 0;
    //! @}
};


// ****************************************************************
// language file class
// TODO: save main language file to config
class coreLanguage final
{
private:
    coreLookupStr<std::string> m_asString;                 //!< list with language-strings to specific keys

    std::u_map<std::string*, std::string*> m_apsPointer;   //!< foreign string pointers connected with language-strings <foreign, language>
    std::u_set<coreTranslate*> m_apObject;                 //!< objects to update after modification

    std::string m_sPath;                                   //!< relative path of the file


public:
    explicit coreLanguage(const char* pcPath)noexcept : m_sPath ("") {this->Load(pcPath);}
    ~coreLanguage() {SDL_assert(m_apsPointer.empty() && m_apObject.empty()); m_asString.clear();}
    friend class coreTranslate;

    //! load and access the language file
    //! @{
    coreError Load(const char* pcPath);
    inline const char* GetString(const char* pcKey)const {return m_asString.at(pcKey).c_str();}
    //! @}

    //! bind and unbind foreign string pointer
    //! @{
    void BindString(std::string* psString, const char* pcKey);
    inline void UnbindString(std::string* psString) {SDL_assert(m_apsPointer.count(psString)); m_apsPointer.erase(psString);}
    //! @}


private:
    DISABLE_COPY(coreLanguage)

    //! bind and unbind update objects
    //! @{
    void __BindObject(coreTranslate* pObject)   {SDL_assert(!m_apObject.count(pObject)); m_apObject.insert(pObject);}
    void __UnbindObject(coreTranslate* pObject) {SDL_assert( m_apObject.count(pObject)); m_apObject.erase(pObject);}
    //! @}
};


#endif // _CORE_GUARD_LANGUAGE_H_