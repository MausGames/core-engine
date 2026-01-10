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

// TODO 4: when to use append, when to use += (which is only used rarely) ? change everything to append ?


// ****************************************************************
/* base string type */
using coreStringBase = std::string;


// ****************************************************************
/* string class */
class coreString final : public coreStringBase
{
public:
    coreString() = default;
    constexpr coreString(const coreChar* pcText)noexcept                       : coreStringBase (pcText ? pcText : "")       {}
    constexpr coreString(const coreChar* pcText, const coreUintW iNum)noexcept : coreStringBase (pcText ? pcText : "", iNum) {}
    constexpr coreString(const coreUintW iNum, const coreChar cChar)noexcept   : coreStringBase (iNum, cChar)                {}
    constexpr coreString(coreStringBase&& m)noexcept                           : coreStringBase (std::move(m))               {}

    ENABLE_COPY(coreString)

    /* assign new string */
    using coreStringBase::assign;
    constexpr coreString& assign(const coreChar* pcText)                       {this->coreStringBase::assign(pcText ? pcText : "");       return *this;}
    constexpr coreString& assign(const coreChar* pcText, const coreUintW iNum) {this->coreStringBase::assign(pcText ? pcText : "", iNum); return *this;}

    /* append new string */
    using coreStringBase::append;
    constexpr coreString& append(const coreChar* pcText)                       {this->coreStringBase::append(pcText ? pcText : "");       return *this;}
    constexpr coreString& append(const coreChar* pcText, const coreUintW iNum) {this->coreStringBase::append(pcText ? pcText : "", iNum); return *this;}

    /* append new string with operator */
    using coreStringBase::operator +=;
    constexpr coreString& operator += (const coreChar* pcText) {return this->append(pcText);}

    /* prepend new string */
    constexpr coreString& prepend(const coreChar* pcText)                       {this->insert(0u, pcText);       return *this;}
    constexpr coreString& prepend(const coreChar* pcText, const coreUintW iNum) {this->insert(0u, pcText, iNum); return *this;}

    /* replace all occurrences of a sub-string with another one */
    using coreStringBase::replace;
    coreString& replace(const coreChar* pcOld, const coreChar* pcNew);

    /* replace multiple sub-strings at once */
    template <typename... A> coreString& replace_many(A&&... vArgs);

    /* trim string on both sides */
    coreString& trim(const coreChar* pcRemove = " \n\r\t");
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
    inline coreWorkString(const coreChar* pcText)noexcept;
    inline coreWorkString(coreWorkString&& m)noexcept;
    ~coreWorkString();

    CUSTOM_COPY(coreWorkString)

    /* assignment operations */
    coreWorkString& operator = (coreWorkString&& m)noexcept;

    /* access string buffer */
    inline const coreChar& operator [] (const coreUintW iIndex)const {ASSERT(iIndex < m_iCapacity) return m_pcBuffer[iIndex];}
    inline const coreChar* c_str       ()const                       {ASSERT(m_pcBuffer)           return m_pcBuffer;}

    /* control memory allocation */
    coreBool reserve(const coreUintW iCapacity);
    void     shrink_to_fit();

    /* add string data */
    void assign (const coreChar* pcText);
    void append (const coreChar* pcText);
    void replace(const coreChar* pcOld, const coreChar* pcNew);

    /* remove string data */
    void erase(const coreUintW iIndex, const coreUintW iCount);
    void clear();

    /* create formatted string */
    template <typename... A> inline void print_assign(const coreChar* pcFormat, A&&... vArgs) {this->__print(0u,             pcFormat, std::forward<A>(vArgs)...);}
    template <typename... A> inline void print_append(const coreChar* pcFormat, A&&... vArgs) {this->__print(this->length(), pcFormat, std::forward<A>(vArgs)...);}
    inline void                          print_assign(const coreChar* pcFormat)               {this->assign(pcFormat);}
    inline void                          print_append(const coreChar* pcFormat)               {this->append(pcFormat);}

    /* get object properties */
    inline coreUintW capacity()const {return m_iCapacity;}
    inline coreUintW length  ()const {return m_iSize -  1u;}
    inline coreBool  empty   ()const {return m_iSize == 1u;}


private:
    /* create formatted string */
    void __print(const coreUintW iOffset, SDL_PRINTF_FORMAT_STRING const coreChar* pcFormat, ...) SDL_PRINTF_VARARG_FUNC(3);
};


// ****************************************************************
/* replace multiple sub-strings at once */
template <typename... A> coreString& coreString::replace_many(A&&... vArgs)
{
    STATIC_ASSERT(coreMath::IsAligned(sizeof...(A), 2u))

    // allow variadic argument iteration
    const coreChar* apcText[] = {vArgs...};

    // replace all available sub-strings
    for(coreUintW i = 0u; i < sizeof...(A); i += 2u)
    {
        this->replace(apcText[i + 0u], apcText[i + 1u]);
    }

    return *this;
}


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

inline coreWorkString::coreWorkString(coreWorkString&& m)noexcept
: m_pcBuffer  (std::exchange(m.m_pcBuffer, NULL))
, m_iSize     (m.m_iSize)
, m_iCapacity (m.m_iCapacity)
{
}


#endif /* _CORE_GUARD_STRING_H_ */