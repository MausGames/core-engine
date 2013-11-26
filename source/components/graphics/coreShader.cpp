//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

std::string  coreShader::s_asGlobal[3]; // = "";
coreProgram* coreProgram::s_pCurrent       = NULL;


// ****************************************************************
// constructor
coreShader::coreShader()noexcept
: m_iShader (0)
{
}

coreShader::coreShader(const char* pcPath)noexcept
: m_iShader (0)
{
    // load from path
    this->coreResource::Load(pcPath);
}

coreShader::coreShader(coreFile* pFile)noexcept
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
    coreFileLock Lock(pFile, true);

    ASSERT_IF(m_iShader)  return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_FILE_ERROR;

    // extract file extension
    const char* pcExtension = coreData::StrExtension(pFile->GetPath());

    // set shader type
    GLenum iType;
         if(!std::strcmp(pcExtension, "vs") || !std::strcmp(pcExtension, "vert")) iType = GL_VERTEX_SHADER;
    else if(!std::strcmp(pcExtension, "fs") || !std::strcmp(pcExtension, "frag")) iType = GL_FRAGMENT_SHADER;
    else
    {
        Core::Log->Error(0, coreData::Print("Shader (%s) could not be identified (valid extension: vs, vert, fs, frag)", pFile->GetPath()));
        return CORE_INVALID_DATA;
    }

    // init global shader data
    __InitGlobal();

    // assemble the shader
    const std::string& sGlobal = s_asGlobal[(iType == GL_VERTEX_SHADER) ? 1 : 2];
    const char* apcData[3]     = {s_asGlobal[0].c_str(),         sGlobal.c_str(),         r_cast<const char*>(pFile->GetData())};
    const GLint aiSize[3]      = {(GLint)s_asGlobal[0].length(), (GLint)sGlobal.length(), (GLint)pFile->GetSize()};

    // create and compile the shader
    m_iShader = glCreateShader(iType);
    glShaderSource(m_iShader, 3, apcData, aiSize);
    glCompileShader(m_iShader);

    // unlock file
    Lock.Release();

    // save attributes
    m_sPath = pFile->GetPath();
    m_iSize = pFile->GetSize() + sGlobal.length();

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
            Core::Log->Error(0, coreData::Print("Shader (%s) could not be compiled", pFile->GetPath()));
            Core::Log->ListStart("Shader Error Log");
            Core::Log->ListEntry(pcLog);
            Core::Log->ListEnd();

            SAFE_DELETE_ARRAY(pcLog)
        }
        return CORE_INVALID_DATA;
    }

    Core::Log->Info(coreData::Print("Shader (%s) loaded", pFile->GetPath()));
    return CORE_OK;
}


// ****************************************************************
// unload shader resource data
coreError coreShader::Unload()
{
    if(!m_iShader) return CORE_INVALID_CALL;

    // delete shader
    glDeleteShader(m_iShader);
    Core::Log->Info(coreData::Print("Shader (%s) unloaded", m_sPath.c_str()));

    // reset attributes
    m_sPath   = "";
    m_iSize   = 0;
    m_iShader = 0;

    return CORE_OK;
}


// ****************************************************************
// init global shader data
void coreShader::__InitGlobal()
{
    if(!s_asGlobal[0].empty()) return;

    // retrieve global shader files
    coreFile* pGlobalVertex   = Core::Manager::Resource->RetrieveFile("data/shaders/global.vs");
    coreFile* pGlobalFragment = Core::Manager::Resource->RetrieveFile("data/shaders/global.fs");

    // create global shader data
    s_asGlobal[0].assign(coreData::Print("#version %.0f", Core::Graphics->GetUniformBuffer() ? Core::Graphics->SupportGLSL()*100.0f : 110.0f));
    s_asGlobal[0].append(coreData::Print("\n#define CORE_TEXTURE_UNITS   (%d)", CORE_TEXTURE_UNITS));
    s_asGlobal[0].append(coreData::Print("\n#define CORE_GRAPHICS_LIGHTS (%d)", CORE_GRAPHICS_LIGHTS));
    s_asGlobal[1].assign(r_cast<const char*>(pGlobalVertex->GetData()),   pGlobalVertex->GetSize());
    s_asGlobal[2].assign(r_cast<const char*>(pGlobalFragment->GetData()), pGlobalFragment->GetSize());

    pGlobalVertex->UnloadData();
    pGlobalFragment->UnloadData();
}


// ****************************************************************
// constructor
coreProgram::coreProgram()noexcept
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
    // exit the shader-program
    this->Exit();

    // remove all shader objects
    m_apShader.clear();
}


// ****************************************************************
// init the shader-program
coreError coreProgram::Init()
{
    if(m_iStatus != 1) return CORE_INVALID_CALL;

    // check if all requested shaders are loaded
    FOR_EACH(it, m_apShader)
        if(!it->IsLoaded()) return CORE_BUSY;

#if defined(_CORE_DEBUG_)

    // check for duplicate shader objects
    for(coreUint i = 0; i < m_apShader.size(); ++i)
        for(coreUint j = i+1; j < m_apShader.size(); ++j)
            SDL_assert(std::strcmp(m_apShader[i]->GetPath(), m_apShader[j]->GetPath()));

#endif

    // create shader-program
    ASSERT_IF(m_iProgram) return CORE_INVALID_CALL;
    m_iProgram = glCreateProgram();

    // attach shader objects
    FOR_EACH(it, m_apShader)
        glAttachShader(m_iProgram, (*it)->GetShader());

    // set attribute and output locations
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_POSITION_NUM, CORE_SHADER_ATTRIBUTE_POSITION);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  CORE_SHADER_ATTRIBUTE_TEXTURE);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   CORE_SHADER_ATTRIBUTE_NORMAL);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  CORE_SHADER_ATTRIBUTE_TANGENT);
    if(Core::Graphics->GetUniformBuffer())
    {
        for(int i = 0; i < CORE_SHADER_OUTPUT_COLORS; ++i)
            glBindFragDataLocation(m_iProgram, i, CORE_SHADER_OUTPUT_COLOR(i));
    }

    // link shader-program
    glLinkProgram(m_iProgram);
    m_iStatus = 2;

    // bind global uniform buffer object
    if(Core::Graphics->GetUniformBuffer())
    {
        const int iBlock = glGetUniformBlockIndex(m_iProgram, CORE_SHADER_BUFFER_GLOBAL);
        if(iBlock >= 0) glUniformBlockBinding(m_iProgram, iBlock, CORE_SHADER_BUFFER_GLOBAL_NUM);
    }

    // check for errors
    int iLinked;
    glGetProgramiv(m_iProgram, GL_LINK_STATUS, &iLinked);

#if defined(_CORE_DEBUG_)

    // validate shader-program
    glValidateProgram(m_iProgram);

    // check for errors
    int iValid;
    glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &iValid);
    iLinked += iValid;

#endif

    ASSERT_IF(!iLinked)
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
            Core::Log->ListStart("Shader-Program Error Log");
            FOR_EACH(it, m_apShader)
                Core::Log->ListEntry(coreData::Print("(%s)", (*it)->GetPath()));
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
    FOR_EACH(it, m_apShader)
        glDetachShader(m_iProgram, (*it)->GetShader());

    // clear identifiers and cache
    m_aiUniform.clear();
    m_aiAttribute.clear();
    m_avCache.clear();

    // delete shader-program
    glDeleteProgram(m_iProgram);
    m_iProgram = 0;
    m_iStatus  = 1;

    return CORE_OK;
}


// ****************************************************************
// enable the shader-program
// TODO: remove/move linking part somehow
bool coreProgram::Enable()
{
    SDL_assert(m_iStatus);

    // check current shader-program
    if(s_pCurrent == this) return true;

    // link shader-program
    if(!this->IsFinished())
    {
        if(this->Init() != CORE_OK)
            return false;
    }

    // set current shader-program
    s_pCurrent = this;
    glUseProgram(m_iProgram);

    // bind all texture units
    if(m_iStatus == 2)
    {
        for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
            glUniform1i(glGetUniformLocation(m_iProgram, CORE_SHADER_UNIFORM_TEXTURE(i)), i);

        m_iStatus = 3;
    }

    // forward global uniform data without UBO
    if(!Core::Graphics->GetUniformBuffer())
    {
        for(int i = 0; i < CORE_GRAPHICS_LIGHTS; ++i)
        {
            this->SetUniform(CORE_SHADER_UNIFORM_LIGHT_POSITION(i),  Core::Graphics->GetLight(i).vPosition);
            this->SetUniform(CORE_SHADER_UNIFORM_LIGHT_DIRECTION(i), Core::Graphics->GetLight(i).vDirection);
            this->SetUniform(CORE_SHADER_UNIFORM_LIGHT_RANGE(i),     Core::Graphics->GetLight(i).fRange);
            this->SetUniform(CORE_SHADER_UNIFORM_LIGHT_VALUE(i),     Core::Graphics->GetLight(i).vValue);
        }
    }

    return true;
}


// ****************************************************************
// disable the shader-program
void coreProgram::Disable()
{
    if(!s_pCurrent) return;

    // reset current shader-program
    s_pCurrent = NULL;
    glUseProgram(0);
}


// ****************************************************************
// add shader object for later attachment
coreProgram* coreProgram::AttachShaderFile(const char* pcPath)
{
    if(!m_iStatus) m_apShader.push_back(Core::Manager::Resource->LoadFile<coreShader>(pcPath));
    return this;
}

coreProgram* coreProgram::AttachShaderLink(const char* pcName)
{
    if(!m_iStatus) m_apShader.push_back(Core::Manager::Resource->LoadLink<coreShader>(pcName));
    return this;
}


// ****************************************************************
// reset with the resource manager
void coreProgram::__Reset(const bool& bInit)
{
    if(bInit) this->Init();
         else this->Exit();
}