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

// TODO: save current main language file to config
// TODO: is it possible that pointers to strings may change (vector resize) ?


// ****************************************************************
/* language definitions */
#define CORE_LANGUAGE_KEY    "$"    //!< prefix used to identify keys in the language file
#define CORE_LANGUAGE_ASSIGN "="    //!< delimiter used to separate keys and their assigned strings


// ****************************************************************
/* translation interface */
class coreTranslate
{
private:
    coreLanguage* m_pLanguage;                            //!< associated language file
    coreLookup<std::string*, std::string> m_apsPointer;   //!< own string pointers connected with keys <own, key>


public:
    coreTranslate()noexcept : m_pLanguage (NULL) {}
    virtual ~coreTranslate() {this->ChangeLanguage(NULL); m_apsPointer.clear();}
    friend class coreLanguage;

    /*! change associated language file */
    //! @{
    void ChangeLanguage(coreLanguage* pLanguage);
    //! @}

    
protected:
    /*! bind and unbind own string pointer */
    //! @{
    void _BindString  (std::string* psString, const char* pcKey);
    void _UnbindString(std::string* psString);
    //! @}


private:
    DISABLE_COPY(coreTranslate)

    /*! update object after modification */
    //! @{
    virtual void __Update() = 0;
    //! @}
};


// ****************************************************************
/* language file class */
class coreLanguage final
{
private:
    coreLookupStr<std::string> m_asString;                 //!< list with language-strings to specific keys

    coreLookup<std::string*, std::string*> m_apsForeign;   //!< foreign string pointers connected with language-strings <foreign, language>
    std::u_set<coreTranslate*> m_apObject;                 //!< objects to update after modification

    std::string m_sPath;                                   //!< relative path of the file


public:
    explicit coreLanguage(const char* pcPath)noexcept;
    ~coreLanguage();
    friend class coreTranslate;

    /*! load and access the language file */
    //! @{
    coreError Load(const char* pcPath);
    inline const char* GetString(const char* pcKey)const {return m_asString.count(pcKey) ? m_asString.at(pcKey).c_str() : pcKey;}
    //! @}

    /*! bind and unbind foreign string pointers */
    //! @{
    void        BindForeign  (std::string* psForeign, const char* pcKey);
    inline void UnbindForeign(std::string* psForeign) {ASSERT(m_apsForeign.count(psForeign)) m_apsForeign.erase(psForeign);}
    //! @}


private:
    DISABLE_COPY(coreLanguage)

    /*! bind and unbind translation objects */
    //! @{
    void __BindObject  (coreTranslate* pObject) {ASSERT(!m_apObject.count(pObject)) m_apObject.insert(pObject);}
    void __UnbindObject(coreTranslate* pObject) {ASSERT( m_apObject.count(pObject)) m_apObject.erase(pObject);}
    //! @}
};


#endif /* _CORE_GUARD_LANGUAGE_H_ */