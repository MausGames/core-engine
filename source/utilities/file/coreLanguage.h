///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LANGUAGE_H_
#define _CORE_GUARD_LANGUAGE_H_

// TODO: save current main language file to config
// TODO: currently key-identifier $ may cause problems in normal text
// TODO: reduce memory consumption with all the redundant keys in containers
// TODO: determine number of entries and call reserve(x)
// TODO: try to use strtok()


// ****************************************************************
/* language definitions */
#define CORE_LANGUAGE_KEY    "$"   // prefix used to identify keys in the language file
#define CORE_LANGUAGE_ASSIGN "="   // delimiter used to separate keys and their assigned strings


// ****************************************************************
/* translation interface */
class INTERFACE coreTranslate
{
private:
    coreLanguage* m_pLanguage;                       // associated language file
    coreMap<coreString*, coreString> m_apsPointer;   // own string pointers connected with keys <own, key>


public:
    coreTranslate()noexcept;
    coreTranslate(const coreTranslate& c)noexcept;
    virtual ~coreTranslate();

    FRIEND_CLASS(coreLanguage)

    /* assignment operations */
    coreTranslate& operator = (const coreTranslate& c)noexcept;

    /* change associated language file */
    void ChangeLanguage(coreLanguage* pLanguage);


protected:
    /* bind and unbind own string pointers */
    void _BindString  (coreString* psString, const coreHashString& sKey);
    void _UnbindString(coreString* psString);


private:
    /* update object after modification */
    virtual void __Update() = 0;
};


// ****************************************************************
/* language file class */
class coreLanguage final
{
private:
    coreMapStrFull<coreString> m_asStringList;       // list with language-strings to specific keys

    coreMap<coreString*, coreString> m_apsForeign;   // foreign string pointers connected with keys <foreign, key>
    coreSet<coreTranslate*> m_apObject;              // objects to update after modification

    coreString m_sPath;                              // relative path of the file


public:
    coreLanguage()noexcept;
    explicit coreLanguage(const coreChar* pcPath)noexcept;
    ~coreLanguage();

    FRIEND_CLASS(coreTranslate)
    DISABLE_COPY(coreLanguage)

    /* load and access the language file */
    coreStatus Load(const coreChar* pcPath);
    inline const coreChar* GetString(const coreHashString& sKey)const {return m_asStringList.count_bs(sKey) ? m_asStringList.at_bs(sKey).c_str() : sKey.GetString();}
    inline coreBool        HasString(const coreHashString& sKey)const {return m_asStringList.count_bs(sKey);}

    /* bind and unbind foreign string pointers */
    void        BindForeign  (coreString* psForeign, const coreHashString& sKey);
    inline void UnbindForeign(coreString* psForeign) {ASSERT(m_apsForeign.count_bs(psForeign)) m_apsForeign.erase_bs(psForeign);}

    /* get object properties */
    inline const coreChar* GetPath      ()const {return m_sPath.c_str();}
    inline       coreUintW GetNumStrings()const {return m_asStringList.size();}

    /* access language files directly */
    static coreBool FindString(const coreChar* pcPath, const coreChar* pcKey, coreString* OUTPUT psOutput);
    static void GetAvailableLanguages(const coreChar* pcPath, const coreChar* pcFilter, coreMap<coreString, coreString>* OUTPUT pasOutput);
    static void GetAvailableLanguages(coreMap<coreString, coreString>* OUTPUT pasOutput);


private:
    /* bind and unbind translation objects */
    inline void __BindObject  (coreTranslate* pObject) {ASSERT(!m_apObject.count_bs(pObject)) m_apObject.insert_bs(pObject);}
    inline void __UnbindObject(coreTranslate* pObject) {ASSERT( m_apObject.count_bs(pObject)) m_apObject.erase_bs (pObject);}
};


#endif /* _CORE_GUARD_LANGUAGE_H_ */