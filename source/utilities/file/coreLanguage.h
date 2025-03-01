///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LANGUAGE_H_
#define _CORE_GUARD_LANGUAGE_H_

// TODO 3: save current main language file to config
// TODO 3: currently key-identifier $ may cause problems in normal text, maybe use $$
// TODO 3: reduce memory consumption with all the redundant keys in containers
// TODO 3: determine number of entries and call reserve(x), or use property (e.g. RESERVE)
// TODO 5: try to use strtok_s/_r()
// TODO 3: use stream/mmap for FindString if possible


// ****************************************************************
/* language definitions */
#define CORE_LANGUAGE_KEY    "$"   // prefix used to identify keys in the language file
#define CORE_LANGUAGE_ASSIGN "="   // delimiter used to separate keys and their assigned strings
#define CORE_LANGUAGE_EMPTY  "#"   // value used to mark empty strings

#define CORE_LANGUAGE_BRAZILIAN  "brazilian"
#define CORE_LANGUAGE_ENGLISH    "english"
#define CORE_LANGUAGE_FRENCH     "french"
#define CORE_LANGUAGE_GERMAN     "german"
#define CORE_LANGUAGE_ITALIAN    "italian"
#define CORE_LANGUAGE_JAPANESE   "japanese"
#define CORE_LANGUAGE_KOREAN     "korean"
#define CORE_LANGUAGE_LATAM      "latam"
#define CORE_LANGUAGE_POLISH     "polish"
#define CORE_LANGUAGE_PORTUGUESE "portuguese"
#define CORE_LANGUAGE_RUSSIAN    "russian"
#define CORE_LANGUAGE_SCHINESE   "schinese"
#define CORE_LANGUAGE_SPANISH    "spanish"
#define CORE_LANGUAGE_TCHINESE   "tchinese"
#define CORE_LANGUAGE_UKRAINIAN  "ukrainian"

using coreAssembleFunc = std::function<void(coreString* OUTPUT)>;


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

    /* manually refresh own string pointers */
    void RefreshLanguage();


protected:
    /* bind and unbind own string pointers */
    void _BindString  (coreString* psString, const coreHashString& sKey, coreAssembleFunc nFunc);
    void _UnbindString(coreString* psString);


private:
    /* update object after modification */
    virtual void __UpdateTranslate() = 0;
};


// ****************************************************************
/* language file class */
class coreLanguage final
{
private:
    coreMapStrFull<coreString> m_asStringList;             // list with language-strings to specific keys

    coreMap<coreString*, coreString>       m_asForeign;    // foreign string pointers connected with keys <foreign, key>
    coreMap<coreString*, coreAssembleFunc> m_anAssemble;   // possible assemble functions (to allow complex text construction)
    coreSet<coreTranslate*>                m_apObject;     // objects to update after modification

    coreString m_sPath;                                    // relative path of the file


public:
    coreLanguage()noexcept;
    explicit coreLanguage(const coreChar* pcPath)noexcept;
    ~coreLanguage();

    FRIEND_CLASS(coreTranslate)
    DISABLE_COPY(coreLanguage)

    /* load and access the language file */
    coreStatus Load(const coreChar* pcPath);
    inline const coreChar* GetString(const coreHashString& sKey)const {return m_asStringList.count_bs(sKey) ? m_asStringList.at_bs(sKey).c_str() : PRINT(CORE_LANGUAGE_KEY "%s", sKey.GetString());}
    inline coreBool        HasString(const coreHashString& sKey)const {return m_asStringList.count_bs(sKey);}

    /* bind and unbind foreign string pointers */
    void        BindForeign  (coreString* psForeign, const coreHashString& sKey, coreAssembleFunc nFunc = NULL);   // [](coreString* OUTPUT psString) -> void
    inline void UnbindForeign(coreString* psForeign) {ASSERT(m_asForeign.count_bs(psForeign)) m_asForeign.erase_bs(psForeign); m_anAssemble.erase_bs(psForeign);}

    /* manually refresh foreign string pointers */
    void RefreshForeign(coreString* psForeign, const coreHashString& sKey);

    /* get object properties */
    inline const coreChar* GetPath      ()const {return m_sPath.c_str();}
    inline       coreUintW GetNumStrings()const {return m_asStringList.size();}

    /* access language files directly */
    static coreBool FindString           (const coreChar* pcPath, const coreChar* pcKey, coreString* OUTPUT psOutput);
    static void     GetAvailableLanguages(const coreChar* pcPath, const coreChar* pcFilter, coreMap<coreString, coreString>* OUTPUT pasOutput);
    static void     GetAvailableLanguages(coreMap<coreString, coreString>* OUTPUT pasOutput);


private:
    /* bind and unbind translation objects */
    inline void __BindObject  (coreTranslate* pObject) {ASSERT(!m_apObject.count_bs(pObject)) m_apObject.insert_bs(pObject);}
    inline void __UnbindObject(coreTranslate* pObject) {ASSERT( m_apObject.count_bs(pObject)) m_apObject.erase_bs (pObject);}

    /* update the foreign string */
    void __Assemble(coreString* psForeign, const coreString& sText);
};


#endif /* _CORE_GUARD_LANGUAGE_H_ */