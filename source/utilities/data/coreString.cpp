///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* replace all occurrences of a sub-string with another one */
coreString& coreString::replace(const coreChar* pcOld, const coreChar* pcNew)
{
    ASSERT(pcOld && pcNew)

    coreUintW iPos = 0u;

    // get length of both sub-strings
    const coreUintW iOldLen = std::strlen(pcOld);
    const coreUintW iNewLen = std::strlen(pcNew);

    // loop only once and replace all findings
    while((iPos = this->find(pcOld, iPos, iOldLen)) != coreString::npos)
    {
        this->replace(iPos, iOldLen, pcNew, iNewLen);
        iPos += iNewLen;
    }

    return *this;
}


// ****************************************************************
/* trim string on both sides */
coreString& coreString::trim(const coreChar* pcRemove)
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
/* destructor */
coreWorkString::~coreWorkString()
{
    // free string buffer
    DYNAMIC_DELETE(m_pcBuffer)
}


// ****************************************************************
/* assignment operations */
coreWorkString& coreWorkString::operator = (coreWorkString&& m)noexcept
{
    // swap properties
    std::swap(m_pcBuffer,  m.m_pcBuffer);
    std::swap(m_iSize,     m.m_iSize);
    std::swap(m_iCapacity, m.m_iCapacity);

    return *this;
}


// ****************************************************************
/* reserve string memory */
coreBool coreWorkString::reserve(const coreUintW iCapacity)
{
    ASSERT(iCapacity <= 0xFFFFFFFFu)

    if(iCapacity > m_iCapacity)
    {
        // align to allocator granularity
        const coreUintW iAlign = coreMath::CeilAlign(iCapacity, ALIGNMENT_NEW);

        // reallocate to new capacity
        DYNAMIC_RESIZE(m_pcBuffer, iAlign)
        m_iCapacity = iAlign;

        return true;
    }

    return false;
}


// ****************************************************************
/* remove unused capacity */
void coreWorkString::shrink_to_fit()
{
    if(m_iSize == 1u)
    {
        // free string buffer
        DYNAMIC_DELETE(m_pcBuffer)
        m_iCapacity = 0u;
    }
    else if(m_iSize < m_iCapacity)
    {
        // reallocate to current string size
        DYNAMIC_RESIZE(m_pcBuffer, m_iSize)
        m_iCapacity = m_iSize;
    }
}


// ****************************************************************
/* assign new string */
void coreWorkString::assign(const coreChar* pcText)
{
    if(!pcText) pcText = "";

    const coreUintW iLen = std::strlen(pcText);

    // adjust size and capacity
    m_iSize = iLen + 1u;
    this->reserve(m_iSize);

    // copy new string over string buffer
    std::memcpy(m_pcBuffer, pcText, iLen + 1u);
}


// ****************************************************************
/* append new string */
void coreWorkString::append(const coreChar* pcText)
{
    if(!pcText) pcText = "";

    const coreUintW iLen = std::strlen(pcText);

    // adjust size and capacity
    m_iSize += iLen;
    this->reserve(m_iSize);

    // copy new string to the end of string buffer
    std::memcpy(m_pcBuffer + m_iSize - iLen - 1u, pcText, iLen + 1u);
}


// ****************************************************************
/* replace all occurrences of a sub-string with another one */
void coreWorkString::replace(const coreChar* pcOld, const coreChar* pcNew)
{
    ASSERT(pcOld && pcNew)

    coreChar* pCursor = m_pcBuffer;

    // get length of both sub-strings
    const coreUintW iOldLen = std::strlen(pcOld);
    const coreUintW iNewLen = std::strlen(pcNew);

    // loop only once and replace all findings
    while((pCursor = std::strstr(pCursor, pcOld)) != NULL)
    {
        const coreUintW iPos = pCursor - m_pcBuffer;

        // adjust size and capacity
        m_iSize = m_iSize - iOldLen + iNewLen;
        if(this->reserve(m_iSize)) pCursor = m_pcBuffer + iPos;

        // shift ending and insert new sub-string
        std::memmove(pCursor + iNewLen, pCursor + iOldLen, m_iSize - iPos - iNewLen);
        std::memcpy (pCursor,           pcNew,             iNewLen);
        pCursor += iNewLen;
    }
}


// ****************************************************************
/* remove range of string data */
void coreWorkString::erase(const coreUintW iIndex, const coreUintW iCount)
{
    ASSERT(iIndex + iCount < m_iSize)

    // move ending over target-range
    std::memmove(m_pcBuffer + iIndex, m_pcBuffer + iIndex + iCount, m_iSize - iIndex - iCount);
    m_iSize -= iCount;
}


// ****************************************************************
/* remove all string data */
void coreWorkString::clear()
{
    // just terminate string
    if(m_pcBuffer) m_pcBuffer[0] = '\0';
    m_iSize = 1u;
}


// ****************************************************************
/* create formatted string */
void coreWorkString::__print(const coreUintW iOffset, const coreChar* pcFormat, ...)
{
    if(!pcFormat) pcFormat = "";

    do
    {
        // prepare variable arguments
        va_list oArgs;
        va_start(oArgs, pcFormat);

        // assemble string
        m_iSize = coreData::PrintBaseV(m_pcBuffer + iOffset, m_iCapacity - iOffset, pcFormat, oArgs) + iOffset + 1u;
        va_end(oArgs);

        // check for success
        ASSERT(m_iSize > 0u)
        if(m_iSize <= m_iCapacity) return;

        // reserve more string memory
        this->reserve(m_iSize);
    }
    while(true);
}