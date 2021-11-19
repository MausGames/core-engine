///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_STRING_H_
#define _CORE_GUARD_STRING_H_

// TODO 3: constexpr, when std::string in libstdc++ and libc++ supports it
// TODO 4: move code to .cpp (remove inline)


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
};


// ****************************************************************
/* trim string on both sides */
inline coreString& coreString::trim(const coreChar* pcRemove)
{
    STATIC_ASSERT(coreString::npos == -1)

    // trim right
    const coreUintW iLast = this->find_last_not_of(pcRemove);
    this->erase(iLast + 1u);

    // trim left
    const coreUintW iFirst = this->find_first_not_of(pcRemove);
    if(iFirst != coreString::npos) this->erase(0u, iFirst);

    return *this;
}


// ****************************************************************
/* replace all occurrences of a sub-string with another one */
inline coreString& coreString::replace(const coreChar* pcOld, const coreChar* pcNew)
{
    coreUintW iPos = 0u;

    // get length of both sub-strings
    const coreUintW iOldLen = std::strlen(pcOld);
    const coreUintW iNewLen = std::strlen(pcNew);

    // loop only once and replace all findings
    while((iPos = this->find(pcOld, iPos, iOldLen)) != coreString::npos)
    {
        this->replace(iPos, iOldLen, pcNew);
        iPos += iNewLen;
    }

    return *this;
}


#endif /* _CORE_GUARD_STRING_H_ */