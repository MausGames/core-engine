///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreLog::coreLog(const coreChar* pcPath)noexcept
: m_pFile       (NULL)
, m_sPath       (pcPath)
, m_eLevel      (CORE_LOG_LEVEL_ALL)
, m_iListStatus (0u)
, m_iLastTime   (0u)
, m_iMainThread (0u)
, m_Lock        ()
{
    // open and reset log file
    m_pFile = std::fopen(m_sPath.c_str(), "wb");
    if(m_pFile)
    {
        // write basic style sheet
        std::fputs("<!DOCTYPE html>"                                  "\n", m_pFile);
        std::fputs("<meta charset=\"utf-8\">"                         "\n", m_pFile);
        std::fputs("<style>"                                          "\n", m_pFile);
        std::fputs("  body    {font: 0.95em courier new;}"            "\n", m_pFile);
        std::fputs(" .time    {color: #AAA; white-space: pre;}"       "\n", m_pFile);
        std::fputs(" .thread1 {color: green;}"                        "\n", m_pFile);
        std::fputs(" .thread2 {color: olive;}"                        "\n", m_pFile);
        std::fputs(" .data    {color: teal;}"                         "\n", m_pFile);
        std::fputs(" .gl      {color: purple;}"                       "\n", m_pFile);
        std::fputs(" .warning {font-weight: bold; color: coral;}"     "\n", m_pFile);
        std::fputs(" .error   {font-weight: bold; color: red;}"       "\n", m_pFile);
        std::fputs(" .header  {font-weight: bold; font-size: 1.4em;}" "\n", m_pFile);
        std::fputs(" .list    {font-weight: bold;}"                   "\n", m_pFile);
        std::fputs("</style>"                                         "\n", m_pFile);

        // write application data and timestamp
        std::fprintf(m_pFile, CORE_LOG_BOLD("Executable:") " %s (%s %s, %s, %s-bit)" "<br>\n", coreData::AppName(), __DATE__, __TIME__, DEFINED(_CORE_DEBUG_) ? "Debug" : "Release", DEFINED(_CORE_64BIT_) ? "64" : "32");
        std::fprintf(m_pFile, CORE_LOG_BOLD("Built with:") " %s, %s"                 "<br>\n", coreData::BuildCompiler(), coreData::BuildLibrary());
        std::fprintf(m_pFile, CORE_LOG_BOLD("Started on:") " %s %s"                  "<br>\n", coreData::DateString(), coreData::TimeString());

        // flush log file
        std::fflush(m_pFile);

        // save first time-value (and init tick-counter)
        m_iLastTime = SDL_GetTicks();

        // save thread-ID from the creator
        m_iMainThread = SDL_ThreadID();
    }
}


// ****************************************************************
/* destructor */
coreLog::~coreLog()
{
    // append final line
    this->__Write(false, "<hr>");

    // close log file
    if(m_pFile) std::fclose(m_pFile);
}


// ****************************************************************
/* write OpenGL debug message */
void GL_APIENTRY WriteOpenGL(GLenum iSource, GLenum iType, GLuint iID, GLenum iSeverity, GLsizei iLength, const GLchar* pcMessage, const void* pUserParam)
{
    coreLog* pLog = s_cast<coreLog*>(c_cast<void*>(pUserParam));

    // write debug message
    pLog->ListStartWarning("OpenGL Debug Message");
    {
        pLog->ListAdd("<span class=\"gl\">" CORE_LOG_BOLD("ID:")           " %d</span>", iID);
        pLog->ListAdd("<span class=\"gl\">" CORE_LOG_BOLD("Source:")   " 0x%04X</span>", iSource);
        pLog->ListAdd("<span class=\"gl\">" CORE_LOG_BOLD("Type:")     " 0x%04X</span>", iType);
        pLog->ListAdd("<span class=\"gl\">" CORE_LOG_BOLD("Severity:") " 0x%04X</span>", iSeverity);
        pLog->ListAdd(pcMessage);
    }
    pLog->ListEnd();

#if defined(_CORE_DEBUG_)

    // also show message box
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenGL Error", pcMessage, NULL);
    WARN_IF(true) {}

#endif
}


// ****************************************************************
/* enable OpenGL debug output */
void coreLog::DebugOpenGL()
{
    if(!Core::Config->GetBool(CORE_CONFIG_BASE_DEBUGMODE) && !DEFINED(_CORE_DEBUG_)) return;

    if(CORE_GL_SUPPORT(KHR_debug))
    {
        // enable synchronous debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // set callback function and filter
        glDebugMessageCallback(&WriteOpenGL, this);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

        // disable all shader compiler messages
        glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, NULL, false);

        // 1: Shader Stats (SGPRs, VGPRs, Code Size, LDS, Scratch, Max Waves, Spilled SGPRs, Spilled VGPRs, PrivMem VGPRs)
        // 2: LLVM Diagnostics (# instructions in function)
        // #: extension # unsupported in # shader

        // disable certain API messages
        constexpr GLuint aiID[] = {131169u, 131185u, 131204u, 131222u};
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER,              GL_DONT_CARE, ARRAY_SIZE(aiID), aiID, false);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DONT_CARE, ARRAY_SIZE(aiID), aiID, false);

        // 131169: Framebuffer detailed info: The driver allocated multisample storage for renderbuffer #.
        // 131185: Buffer detailed info: Buffer object # (bound to #, usage hint is #) will use # memory as the source for buffer object operations.
        // 131204: Texture state usage warning: Texture # is base level inconsistent. Check texture size.
        // 131222: Program undefined behavior warning: Sampler object # is bound to non-depth texture #, yet it is used with a program that uses a shadow sampler. This is undefined behavior.
    }
}


// ****************************************************************
/* write text to the log file */
void coreLog::__Write(const coreBool bTimeStamp, coreString sText)
{
    coreSpinLocker oLocker(&m_Lock);

#if defined(_CORE_DEBUG_)

    // also write text to the standard output
    SDL_Log("%s", sText.c_str());

#endif

    // check for valid log file
    if(m_pFile)
    {
        // convert new lines and color brackets
        sText.replace("\n", "<br>");
        sText.replace("(",  "<span class=\"data\">(");
        sText.replace(")",  ")</span>");

        if(bTimeStamp)
        {
            // get time-value and thread-ID
            const coreUint32   iTime   = SDL_GetTicks();
            const SDL_threadID iThread = SDL_ThreadID();

            // write time-value and thread-ID
            std::fprintf(m_pFile, "<span class=\"time\">[%02u:%02u.%03u - %3u]</span> <span class=\"%s\">[%04lX]</span> ",
                         (iTime / 1000u) / 60u, (iTime / 1000u) % 60u, (iTime % 1000u), coreUint32(MIN(iTime - m_iLastTime, 999u)),
                         (iThread == m_iMainThread) ? "thread1" : "thread2", iThread);

            // save time-value (for duration approximations)
            m_iLastTime = iTime;
        }

        // write text
        std::fputs(sText.c_str(), m_pFile);
        std::fputs("\n",          m_pFile);

        // flush log file
        std::fflush(m_pFile);
    }
}