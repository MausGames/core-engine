//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once


// ****************************************************************
// log file class
class coreLog final
{
private:
    std::string m_sPath;   // relative path of the log file
    int m_iLevel;          // logging level (0 = all | 1 = only non-errors | -1 = only errors)


public:
    coreLog(const char* pcPath);

    // message functions
    inline void Header(const std::string& sText) {this->__Write(1, false, "<hr /><span class=\"header\">" + sText + "</span><br />");}
    inline void Info(const std::string& sText)   {this->__Write(1, true, sText + "<br />");}
    void Error(const bool& bShutdown, const std::string& sText);

    // list functions
    inline void ListStart(const std::string& sText) {this->__Write(1, true, "<span class=\"liststart\">" + sText + "</span><ul>");}
    inline void ListEntry(const std::string& sText) {this->__Write(1, false, "<li>" + sText + "</li>");}
    inline void ListEnd()                           {this->__Write(1, false, "</ul>");}

    // set logging level
    inline void SetLevel(const int& iLevel) {m_iLevel = iLevel;}
    

private:
    // disable copy
    coreLog(const coreLog& c) deletefunc;
    coreLog& operator = (const coreLog& c) deletefunc;

    // write text to the log file
    void __Write(const int& iLevel, const bool& bTime, std::string sText);
};