//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreProgram* coreProgram::s_pCurrent = NULL;


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
    const char* pcExtension = coreUtils::StrExtension(pFile->GetPath());

    // set shader type
    GLenum iType;
         if(!strcmp(pcExtension, "vs") || !strcmp(pcExtension, "vert")) iType = GL_VERTEX_SHADER;
    else if(!strcmp(pcExtension, "fs") || !strcmp(pcExtension, "frag")) iType = GL_FRAGMENT_SHADER;
    else
    {
        Core::Log->Error(0, coreUtils::Print("Shader (%s) could not be identified (valid extension: vs, vert, fs, frag)", pFile->GetPath()));
        return CORE_INVALID_DATA;
    }

    const char* apcData[1] = {reinterpret_cast<const char*>(pFile->GetData())};
    const GLint aiSize[1]  = {(GLint)pFile->GetSize()};

    // create and compile the shader
    m_iShader = glCreateShader(iType);
    glShaderSource(m_iShader, 1, apcData, aiSize);
    glCompileShader(m_iShader);

    // check for errors
    int iStatus;
    glGetShaderiv(m_iShader, GL_COMPILE_STATUS, &iStatus);
    if(!iStatus)
    {
        // get length of error-log
        int iLength;
        glGetShaderiv(m_iShader, GL_INFO_LOG_LENGTH, &iLength);

        if(iLength)
        {
            // get error-log
            char* pcLog = new char[iLength];
            glGetShaderInfoLog(m_iShader, iLength, NULL, pcLog);

            // write error-log
            Core::Log->Error(0, coreUtils::Print("Shader (%s) could not be compiled", pFile->GetPath()));
            Core::Log->ListStart("Error Log");
            Core::Log->ListEntry(pcLog);
            Core::Log->ListEnd();

            SAFE_DELETE_ARRAY(pcLog)
        }
        return CORE_INVALID_DATA;
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
// init the shader-program
coreError coreProgram::Init()
{
    if(m_iStatus != 1) return CORE_INVALID_CALL;

    // check if all requested shaders are loaded
    for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
        if(!(*it).IsLoaded()) return CORE_BUSY;

#if defined(_CORE_DEBUG_)

    // check for duplicate shader objects
    for(coreUint i = 0; i < m_apShader.size(); ++i)
        for(coreUint j = i+1; j < m_apShader.size(); ++j)
            SDL_assert(strcmp(m_apShader[i]->GetPath(), m_apShader[j]->GetPath()));

#endif

    // create shader-program
    SDL_assert(!m_iProgram);
    m_iProgram = glCreateProgram();

    // attach shader objects
    for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
        glAttachShader(m_iProgram, (*it)->GetShader());

    // set input and output attribute locations
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_POSITION_NUM, CORE_SHADER_ATTRIBUTE_POSITION);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  CORE_SHADER_ATTRIBUTE_TEXTURE);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   CORE_SHADER_ATTRIBUTE_NORMAL);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  CORE_SHADER_ATTRIBUTE_TANGENT);
    if(Core::Graphics->SupportOpenGL() >= 3.0f) glBindFragDataLocation(m_iProgram, CORE_SHADER_OUT_COLOR0_NUM, CORE_SHADER_OUT_COLOR0);

    // link shader-program
    glLinkProgram(m_iProgram);
    m_iStatus = 2;

    // check for errors
    int iStatus;
    glGetProgramiv(m_iProgram, GL_LINK_STATUS, &iStatus);

#if defined(_CORE_DEBUG_)

    // validate shader-program
    glValidateProgram(m_iProgram);

    // check for errors
    int iStatusVal;
    glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &iStatusVal);
    iStatus &= iStatusVal;

#endif

    if(!iStatus)
    {
        // get length of error-log
        int iLength;
        glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &iLength);

        if(iLength)
        {
            // get error-log
            char* pcLog = new char[iLength];
            glGetProgramInfoLog(m_iProgram, iLength, NULL, pcLog);

            // write error-log
            Core::Log->Error(0, "Shader-Program could not be linked/validated");
            Core::Log->ListStart("Error Log");
            for(auto it = m_apShader.begin(); it != m_apShader.end(); ++it)
                Core::Log->ListEntry(coreUtils::Print("(%s)", (*it)->GetPath()));
            Core::Log->ListEntry(pcLog);
            Core::Log->ListEnd();

            SAFE_DELETE_ARRAY(pcLog)
        }
        return CORE_INVALID_DATA;
    }
    return CORE_OK;
}


 // ****************************************************************
// exit the shader-program
coreError coreProgram::Exit()
{
    if(!m_iProgram) return CORE_INVALID_CALL;

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

    return CORE_OK;
}


// ****************************************************************
// enable the shader-program
// TODO: use UBOs for OpenGL 3.1+
// TODO: remove/move linking part somehow
void coreProgram::Enable()
{
    SDL_assert(m_iStatus);
    SDL_assert(!s_pCurrent);

    // save current shader-program
    s_pCurrent = this;

    // link the shader-program
    if(!this->IsLinked())
    {
        if(this->Init() != CORE_OK)
            return;
    }

    // set current shader-program
    glUseProgram(m_iProgram);

    // forward transformation matrices
    this->SetUniform(CORE_SHADER_UNIFORM_PERSPECTIVE, Core::Graphics->GetPerspective(), false);
    this->SetUniform(CORE_SHADER_UNIFORM_ORTHO,       Core::Graphics->GetOrtho(),       false);
    this->SetUniform(CORE_SHADER_UNIFORM_CAMERA,      Core::Graphics->GetCamera(),      false);
    this->SetUniform(CORE_SHADER_UNIFORM_TRANSFORM,   coreMatrix::Identity(),           false);
}


// ****************************************************************
// disable the shader-program
void coreProgram::Disable()
{
    SDL_assert(s_pCurrent == this);
    if(!s_pCurrent) return;

    // reset current shader-program
    s_pCurrent = NULL;
    glUseProgram(0);
}


// ****************************************************************
// add shader object for later attachment
coreProgram* coreProgram::AttachShader(const char* pcPath)
{
    if(!m_iStatus) m_apShader.push_back(Core::Manager::Resource->Load<coreShader>(pcPath));
    return this;
}