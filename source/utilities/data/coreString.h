///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_STRING_H_
#define _CORE_GUARD_STRING_H_

// TODO 3: constexpr coreString, when std::string in libstdc++ and libc++ supports it
// TODO 3: work-string should also handle NULL
// TODO 3: remove custom contains function with C++23


// ****************************************************************
/* string class */
class coreString final : public std::string
{
public:
    coreString() = default;
    coreString(const coreChar* pcText)noexcept                       : std::string (pcText ? pcText : "")       {}
    coreString(const coreChar* pcText, const coreUintW iNum)noexcept : std::string (pcText ? pcText : "", iNum) {}
    coreString(const coreUintW iNum, const coreChar cChar)noexcept   : std::string (iNum, cChar)                {}
    coreString(std::string&& m)noexcept                              : std::string (std::move(m))               {}

    ENABLE_COPY(coreString)

    /* trim string on both sides */
    coreString& trim(const coreChar* pcRemove = " \n\r\t");

    /* replace all occurrences of a sub-string with another one */
    using std::string::replace;
    coreString& replace(const coreChar* pcOld, const coreChar* pcNew);

    /* check for existence of a sub-string */
    inline coreBool contains(const coreChar* pcString)const {return (this->find(pcString) != coreString::npos);}
};


// ****************************************************************
/* work-string class */
class coreWorkString final
{
private:
    coreChar* m_pcBuffer;     // string buffer (may be NULL)

    coreUint32 m_iSize;       // string size (including null-terminator)
    coreUint32 m_iCapacity;   // buffer capacity


public:
    constexpr coreWorkString()noexcept;
    coreWorkString(const coreChar* pcText)noexcept;
    ~coreWorkString();

    DISABLE_COPY(coreWorkString)

    /* access string buffer */
    inline const coreChar& operator [] (const coreUintW iIndex)const {ASSERT(iIndex < m_iCapacity) return m_pcBuffer[iIndex];}
    inline const coreChar* c_str       ()const                       {ASSERT(m_pcBuffer)           return m_pcBuffer;}

    /* control memory allocation */
    coreBool reserve(const coreUintW iCapacity);
    void     shrink_to_fit();

    /* add string data */
    void assign (const coreChar* pcString);
    void append (const coreChar* pcString);
    void replace(const coreChar* pcOld, const coreChar* pcNew);

    /* remove string data */
    void erase(const coreUintW iIndex, const coreUintW iCount);
    void clear();

    /* create formatted string */
    void print(const coreChar* pcFormat, ...);

    /* get object properties */
    inline coreUintW capacity()const {return m_iCapacity;}
    inline coreUintW length  ()const {return m_iSize -  1u;}
    inline coreBool  empty   ()const {return m_iSize == 1u;}
};


// ****************************************************************
/* constructor */
constexpr coreWorkString::coreWorkString()noexcept
: m_pcBuffer  (NULL)
, m_iSize     (1u)
, m_iCapacity (0u)
{
}

inline coreWorkString::coreWorkString(const coreChar* pcText)noexcept
: coreWorkString ()
{
    // assign initial string
    this->assign(pcText);
}


#endif /* _CORE_GUARD_STRING_H_ */