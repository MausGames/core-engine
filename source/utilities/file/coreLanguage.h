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
// TODO: currently key-identifier $ may cause problems in normal text
// TODO: reduce memory consumption with all the redundant keys in containers


// ****************************************************************
/* language definitions */
#define CORE_LANGUAGE_KEY    "$"   //!< prefix used to identify keys in the language file
#define CORE_LANGUAGE_ASSIGN "="   //!< delimiter used to separate keys and their assigned strings


// ****************************************************************
/* translation interface */
class INTERFACE coreTranslate
{
private:
    coreLanguage* m_pLanguage;                            //!< associated language file
    coreLookup<std::string*, std::string> m_apsPointer;   //!< own string pointers connected with keys <own, key>


public:
    coreTranslate()noexcept;
    coreTranslate(const coreTranslate& c)noexcept;
    virtual ~coreTranslate();

    FRIEND_CLASS(coreLanguage)

    /*! assignment operations */
    //! @{
    coreTranslate& operator = (const coreTranslate& c)noexcept;
    //! @}

    /*! change associated language file */
    //! @{
    void ChangeLanguage(coreLanguage* pLanguage);
    //! @}


protected:
    /*! bind and unbind own string pointers */
    //! @{
    void _BindString  (std::string* psString, const coreChar* pcKey);
    void _UnbindString(std::string* psString);
    //! @}


private:
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
    coreLookupStr<std::string> m_asStringList;            //!< list with language-strings to specific keys

    coreLookup<std::string*, std::string> m_apsForeign;   //!< foreign string pointers connected with keys <foreign, key>
    coreSet<coreTranslate*> m_apObject;                   //!< objects to update after modification

    std::string m_sPath;                                  //!< relative path of the file


public:
    explicit coreLanguage(const coreChar* pcPath)noexcept;
    ~coreLanguage();

    FRIEND_CLASS(coreTranslate)
    DISABLE_COPY(coreLanguage)

    /*! load and access the language file */
    //! @{
    coreStatus Load(const coreChar* pcPath);
    inline const coreChar* GetString(const coreChar* pcKey)const {return m_asStringList.count(pcKey) ? m_asStringList.at(pcKey).c_str() : pcKey;}
    //! @}

    /*! bind and unbind foreign string pointers */
    //! @{
    void        BindForeign  (std::string* psForeign, const coreChar* pcKey);
    inline void UnbindForeign(std::string* psForeign) {ASSERT(m_apsForeign.count(psForeign)) m_apsForeign.erase(psForeign);}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreChar* GetPath      ()const {return m_sPath.c_str();}
    inline       coreUintW GetNumStrings()const {return m_asStringList.size();}
    //! @}


private:
    /*! bind and unbind translation objects */
    //! @{
    inline void __BindObject  (coreTranslate* pObject) {ASSERT(!m_apObject.count(pObject)) m_apObject.insert(pObject);}
    inline void __UnbindObject(coreTranslate* pObject) {ASSERT( m_apObject.count(pObject)) m_apObject.erase (pObject);}
    //! @}
};


#endif /* _CORE_GUARD_LANGUAGE_H_ */