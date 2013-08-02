#pragma once


// ****************************************************************
// log file class
class coreLog
{
private:
    std::string m_sName;   // name of the log file

    int m_iLevel;          // logging level (0 = all | 1 = only header and info | -1 = only error)


public:
    coreLog(const char* pcName);
    ~coreLog();

    // write formated text to the log file
    inline void Header(const char* pcText) {if(m_iLevel >= 0) this->__Write(false, "<hr /><span class=\"header\">%s</span><br />", pcText);}
    inline void Info(const char* pcText)   {if(m_iLevel >= 0) this->__Write(true, "%s<br />", pcText);}
    void Error(const bool& bShutdown, const char* pcText);

    // write formated lists to the log file
    inline void ListStart(const char* pcText) {if(m_iLevel >= 0) this->__Write(true, "<span class=\"liststart\">%s</span><ul>", pcText);}
    inline void ListEntry(const char* pcText) {if(m_iLevel >= 0) this->__Write(false, "<li>%s</li>", pcText);}
    inline void ListEnd()                     {if(m_iLevel >= 0) this->__Write(false, "</ul>");}

    // set logging level
    inline void SetLevel(const int& iLevel) {m_iLevel = iLevel;}
    

private:
    // write text to the log file
    void __Write(const bool& bTime, const char* pcText, ...);
};