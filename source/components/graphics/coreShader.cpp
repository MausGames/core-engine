//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

std::vector<coreProgram*> coreProgram::s_apStack; // = NULL;


// ****************************************************************
// constructor
coreShader::coreShader()
: m_iShader (0)
{
}

coreShader::coreShader(const char* pcPath)
: m_iShader (0)
{
    // load from path
    this->coreResource::Load(pcPath);
}

coreShader::coreShader(coreFile* pFile)
: m_iShader (0)
{
    // load from file
    this->Load(pFile);
}


// ****************************************************************
// destructor
coreShader::~coreShader()
{
    this->Unload();
}


// ****************************************************************
// load shader resource data
coreError coreShader::Load(coreFile* pFile)
{
    SDL_assert(!m_iShader);

    if(m_iShader)         return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_FILE_ERROR;

    // extract file extension
    const char* pcExtension = coreUtils::StrExt(pFile->GetPath());

    // set shader type
    GLenum iType;
         if(!strcmp(pcExtension, "vs") || !strcmp(pcExtension, "vert")) iType = GL_VERTEX_SHADER;
    else if(!strcmp(pcExtension, "fs") || !strcmp(pcExtension, "frag")) iType = GL_FRAGMENT_SHADER;
    else
    {
        Core::Log->Error(0, coreUtils::Print("Shader (%s) could not be identified (valid extension: vs, vert, fs, frag)", pFile->GetPath()));
        return CORE_FILE_ERROR;
    }

    // create and compile the shader
    m_iShader = glCreateShader(iType);
    glShaderSource(m_iShader, 1, (const GLchar**)pFile->GetData(), (const GLint*)&pFile->GetSize());
    glCompileShader(m_iShader);

    // check for errors
    int iStatus;
    glGetShaderiv(m_iShader, GL_COMPILE_STATUS, &iStatus);
    if(!iStatus)
    {
        int iLength;
        glGetShaderiv(m_iShader, GL_INFO_LOG_LENGTH, &iLength);

        char* pcLog = new char[iLength];
        glGetShaderInfoLog(m_iShader, iLength, NULL, pcLog);

        // write error-log
        Core::Log->Error(0, coreUtils::Print("Shader (%s) could not be compiled", pFile->GetPath()));
        Core::Log->ListStart("Error Log");
        Core::Log->ListEntry(pcLog);
        Core::Log->ListEnd();

        SAFE_DELETE_ARRAY(pcLog)
        return CORE_FILE_ERROR;
    }

    // save shader attributes
    m_sPath = pFile->GetPath();
    m_iSize = pFile->GetSize();

    Core::Log->Info(coreUtils::Print("Shader (%s) loaded", m_sPath.c_str()));
    return CORE_OK;
}


// ****************************************************************
// unload shader resource data
coreError coreShader::Unload()
{
    if(!m_iShader) return CORE_INVALID_CALL;

    // delete shader
    glDeleteShader(m_iShader);
    Core::Log->Info(coreUtils::Print("Shader (%s) unloaded", m_sPath.c_str()));

    // reset attributes
    m_sPath   = "";
    m_iSize   = 0;
    m_iShader = 0;

    return CORE_OK;
}


// ****************************************************************
// constructor
coreProgram::coreProgram()
: m_iProgram (0)
, m_iStatus  (0)
{
    // reserve memory for shader objects
    m_apShader.reserve(4);
}


// ****************************************************************
// destructor
coreProgram::~coreProgram()
{
    // shut down the shader-program
    this->Reset(false);

    // remove all shader objects
    m_apShader.clear();
}


// ****************************************************************
// reset the object with the resource manager
void coreProgram::Reset(const bool& bInit)
{
    if(bInit)
    {
        // check if shader is ready for linking
        if(m_iStatus != 1) return;

        // check if all requested shaders are loaded
        for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
            if(!(*it).IsLoaded()) return;

        // create shader-program
        SDL_assert(m_iProgram == 0);
        m_iProgram = glCreateProgram();

        // attach shader objects
        for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
            glAttachShader(m_iProgram, (*it)->GetShader());

        // link shader-program
        glLinkProgram(m_iProgram);
        m_iStatus = 2;

        // check for errors
        int iStatus;
        glGetProgramiv(m_iProgram, GL_LINK_STATUS, &iStatus);
        if(!iStatus)
        {
            int iLength;
            glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &iLength);

            char* pcLog = new char[iLength];
            glGetProgramInfoLog(m_iProgram, iLength, NULL, pcLog);

            // write error-log
            Core::Log->Error(0, "Shader-Program could not be linked");
            Core::Log->ListStart("Error Log");
            for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it) Core::Log->ListEntry((*it)->GetPath());
            Core::Log->ListEntry(pcLog);
            Core::Log->ListEnd();

            return;
        }

#if defined(_DEBUG)
        // validate shader-program
        glValidateProgram(m_iProgram);

        // check for errors
        glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &iStatus);
        if(!iStatus)
        {
            int iLength;
            glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &iLength);

            char* pcLog = new char[iLength];
            glGetProgramInfoLog(m_iProgram, iLength, NULL, pcLog);

            // write error-log
            Core::Log->Error(0, "Shader-Program could not be validated");
            Core::Log->ListStart("Error Log");
            for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it) Core::Log->ListEntry((*it)->GetPath());
            Core::Log->ListEntry(pcLog);
            Core::Log->ListEnd();
        }
#endif
    }
    else
    {
        if(!m_iProgram) return;

        // detach shader objects
        for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
            glDetachShader(m_iProgram, (*it)->GetShader());

        // clear cached identifiers
        m_aiUniform.clear();
        m_aiAttribute.clear();

        // delete shader-program
        glDeleteProgram(m_iProgram);
        m_iProgram = 0;
        m_iStatus  = 1;
    }
}


// ****************************************************************
// enable the shader-program
void coreProgram::Enable()
{
    SDL_assert(m_iStatus);

    // link the shader-program
    // \todo remove this part somehow
    if(!this->Linked())
    {
        this->Reset(true);
        if(!this->Linked()) return;
    }

    // add shader-program to the stack and enable it
    if(s_apStack.back() != this) s_apStack.push_back(this);
    glUseProgram(m_iProgram);
}


// ****************************************************************
// add shader object for later attachment
void coreProgram::AttachShader(const char* pcPath)
{
    if(m_iStatus) return;

#if defined(_DEBUG)
    // check for already added shader object
    for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
        SDL_assert(strcmp((*it)->GetPath(), pcPath));
#endif

    // load and add new shader object
    m_apShader.push_back(Core::Manager::Resource->Load<coreShader>(pcPath));
}