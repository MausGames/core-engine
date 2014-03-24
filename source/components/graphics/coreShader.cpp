//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

std::string  coreShader::s_asGlobal[6]; // = "";
coreProgram* coreProgram::s_pCurrent       = NULL;


// ****************************************************************
// constructor
coreShader::coreShader()noexcept
: m_iShader (0)
{
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
    coreFileUnload Unload(pFile);

    ASSERT_IF(m_iShader)  return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_FILE_ERROR;

    // extract file extension
    const char* pcExtension = coreData::StrExtension(pFile->GetPath());

    // set shader type
    GLenum iType;
    coreUint iGlobal;
         if(!std::strcmp(pcExtension, "vs")  || !std::strcmp(pcExtension, "vert")) {iType = GL_VERTEX_SHADER;          iGlobal = 1;}
    else if(!std::strcmp(pcExtension, "tcs") || !std::strcmp(pcExtension, "tesc")) {iType = GL_TESS_CONTROL_SHADER;    iGlobal = 2;}
    else if(!std::strcmp(pcExtension, "tes") || !std::strcmp(pcExtension, "tese")) {iType = GL_TESS_EVALUATION_SHADER; iGlobal = 3;}
    else if(!std::strcmp(pcExtension, "gs")  || !std::strcmp(pcExtension, "geom")) {iType = GL_GEOMETRY_SHADER;        iGlobal = 4;}
    else if(!std::strcmp(pcExtension, "fs")  || !std::strcmp(pcExtension, "frag")) {iType = GL_FRAGMENT_SHADER;        iGlobal = 5;}
    else
    {
        Core::Log->Error(false, "Shader (%s) could not be identified (valid extensions: vs, vert, tcs, tesc, tes, tese, gs, geom, fs, frag)", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // check for OpenGL extension and version
    if((iType == GL_TESS_CONTROL_SHADER || iType == GL_TESS_EVALUATION_SHADER) && !GLEW_ARB_tessellation_shader) return CORE_OK;
    if((iType == GL_GEOMETRY_SHADER)                                           && !GLEW_VERSION_3_2)             return CORE_OK;
    
    // load global shader data
    coreShader::__LoadGlobal();

    // assemble the shader
    const char* apcData[3] = {s_asGlobal[0].c_str(),         s_asGlobal[iGlobal].c_str(),         r_cast<const char*>(pFile->GetData())};
    const GLint aiSize[3]  = {(GLint)s_asGlobal[0].length(), (GLint)s_asGlobal[iGlobal].length(), (GLint)pFile->GetSize()};

    // create and compile the shader
    m_iShader = glCreateShader(iType);
    glShaderSource(m_iShader, 3, apcData, aiSize);
    glCompileShader(m_iShader);

    // save properties
    m_sPath = pFile->GetPath();
    m_iSize = aiSize[0] + aiSize[1] + aiSize[2];

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
            // set logging level
            const int iLevel = Core::Log->GetLevel();
            Core::Log->SetLevel(0);

            // get error-log
            char* pcLog = new char[iLength];
            glGetShaderInfoLog(m_iShader, iLength, NULL, pcLog);

            // write error-log
            Core::Log->Error(false, "Shader (%s) could not be compiled", pFile->GetPath());
            Core::Log->ListStart("Shader Error Log");
            Core::Log->ListEntry(pcLog);
            Core::Log->ListEnd();

            // reset logging level
            Core::Log->SetLevel(iLevel);

            SAFE_DELETE_ARRAY(pcLog)
        }
        return CORE_INVALID_DATA;
    }

    Core::Log->Info("Shader (%s) loaded", pFile->GetPath());
    return CORE_OK;
}


// ****************************************************************
// unload shader resource data
coreError coreShader::Unload()
{
    if(!m_iShader) return CORE_INVALID_CALL;

    // delete shader
    glDeleteShader(m_iShader);
    if(!m_sPath.empty()) Core::Log->Info("Shader (%s) unloaded", m_sPath.c_str());

    // reset properties
    m_sPath   = "";
    m_iSize   = 0;
    m_iShader = 0;

    return CORE_OK;
}


// ****************************************************************
// load global shader data
void coreShader::__LoadGlobal()
{
    if(!s_asGlobal[0].empty()) return;

    // set global shader definitions
    s_asGlobal[0].assign(coreData::Print("#version %.0f", Core::Graphics->GetUniformBuffer() ? Core::Graphics->VersionGLSL()*100.0f : 110.0f));
    s_asGlobal[0].append(coreData::Print("\n#define CORE_TEXTURE_UNITS        (%d)", CORE_TEXTURE_UNITS));
    s_asGlobal[0].append(coreData::Print("\n#define CORE_GRAPHICS_LIGHTS      (%d)", CORE_GRAPHICS_LIGHTS));
    s_asGlobal[0].append(coreData::Print("\n#define CORE_SHADER_OUTPUT_COLORS (%d)", CORE_SHADER_OUTPUT_COLORS));

    // loop through list with global shader files
    const char aacPath[][64] = {"data/shaders/global.glsl",
                                "data/shaders/global_vertex.vs",
                                "data/shaders/global_control.tcs",
                                "data/shaders/global_evaluation.tes",
                                "data/shaders/global_geometry.gs",
                                "data/shaders/global_fragment.fs"};

    for(int i = 0; i < 6; ++i)
    {
        // retrieve file
        coreFile* pFile = Core::Manager::Resource->RetrieveFile(aacPath[i]);
        ASSERT_IF(!pFile->GetData()) continue;

        // copy and unload data
        s_asGlobal[i].append(r_cast<const char*>(pFile->GetData()), pFile->GetSize());
        pFile->UnloadData();
    }
}


// ****************************************************************
// constructor
coreProgram::coreProgram()noexcept
: m_iProgram (0)
, m_iStatus  (CORE_SHADER_NEW)
{
    // reserve memory for shader objects
    m_apShader.reserve(4);
}


// ****************************************************************
// destructor
coreProgram::~coreProgram()
{
    // exit the shader-program
    this->__Exit();

    // remove all shader objects and attribute locations
    m_apShader.clear();
    m_aiAttribute.clear();
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
    if(m_iStatus < CORE_SHADER_LINKED)
    {
        if(this->__Init() != CORE_OK)
            return false;
    }

    // set current shader-program
    s_pCurrent = this;
    glUseProgram(m_iProgram);

    // bind all texture units
    if(m_iStatus == CORE_SHADER_LINKED)
    {
        for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
            glUniform1i(glGetUniformLocation(m_iProgram, CORE_SHADER_UNIFORM_TEXTURE(i)), i);

        m_iStatus = CORE_SHADER_FINISHED;
    }

    // forward global uniform data without UBO
    if(!Core::Graphics->GetUniformBuffer())
    {
        const coreMatrix4 mViewProj = Core::Graphics->GetCamera() * Core::Graphics->GetPerspective();

        // forward transformation data
        this->SendUniform(CORE_SHADER_UNIFORM_VIEWPROJ,    mViewProj,                        false);
        this->SendUniform(CORE_SHADER_UNIFORM_CAMERA,      Core::Graphics->GetCamera(),      false);
        this->SendUniform(CORE_SHADER_UNIFORM_PERSPECTIVE, Core::Graphics->GetPerspective(), false);
        this->SendUniform(CORE_SHADER_UNIFORM_ORTHO,       Core::Graphics->GetOrtho(),       false);
        this->SendUniform(CORE_SHADER_UNIFORM_RESOLUTION,  Core::Graphics->GetResolution());

        // forward ambient data
        for(int i = 0; i < CORE_GRAPHICS_LIGHTS; ++i)
        {
            this->SendUniform(CORE_SHADER_UNIFORM_LIGHT_POSITION(i),  Core::Graphics->GetLight(i).vPosition);
            this->SendUniform(CORE_SHADER_UNIFORM_LIGHT_DIRECTION(i), Core::Graphics->GetLight(i).vDirection);
            this->SendUniform(CORE_SHADER_UNIFORM_LIGHT_VALUE(i),     Core::Graphics->GetLight(i).vValue);
        }
    }

#if defined(_CORE_DEBUG_)

    // validate shader-program
    glValidateProgram(m_iProgram);

    // check for errors
    int iStatus;
    glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &iStatus);
    ASSERT_IF(!iStatus)
    {
        this->__WriteLog();
        return false;
    }

#endif

    return true;
}


// ****************************************************************
// disable the shader-program
void coreProgram::Disable(const bool& bFull)
{
    if(!s_pCurrent) return;

    // reset current shader-program
    s_pCurrent = NULL;
    if(bFull) glUseProgram(0);
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
// add attribute location for later binding
coreProgram* coreProgram::BindAttribute(const char* pcName, const int& iLocation)
{
    if(!m_iStatus) m_aiAttribute[pcName] = iLocation;
    return this;
}


// ****************************************************************
// reset with the resource manager
void coreProgram::__Reset(const bool& bInit)
{
    if(bInit) this->__Init();
         else this->__Exit();
}


// ****************************************************************
// init the shader-program
coreError coreProgram::__Init()
{
    if(m_iStatus != CORE_SHADER_DEFINED) return CORE_INVALID_CALL;

    // check if all requested shaders are loaded
    FOR_EACH(it, m_apShader)
        if(!it->IsLoaded() && it->IsManaged()) return CORE_BUSY;

#if defined(_CORE_DEBUG_)

    // check for duplicate shader objects
    for(coreUint i = 0; i < m_apShader.size(); ++i)
        for(coreUint j = i+1; j < m_apShader.size(); ++j)
            SDL_assert(std::strcmp(m_apShader[i]->GetPath(), m_apShader[j]->GetPath()));

    // check for duplicate attribute locations
    for(coreUint i = 0; i < m_aiAttribute.size(); ++i)
        for(coreUint j = i+1; j < m_aiAttribute.size(); ++j)
            SDL_assert(m_aiAttribute[i] != m_aiAttribute[j] && m_aiAttribute[i] >= 0);

#endif

    // create shader-program
    ASSERT_IF(m_iProgram) return CORE_INVALID_CALL;
    m_iProgram = glCreateProgram();

    // attach shader objects
    FOR_EACH(it, m_apShader)
    {
        if((*it)->GetShader())
            glAttachShader(m_iProgram, (*it)->GetShader());
    }

    // bind attribute locations
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_POSITION_NUM, CORE_SHADER_ATTRIBUTE_POSITION);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TEXTURE_NUM,  CORE_SHADER_ATTRIBUTE_TEXTURE);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   CORE_SHADER_ATTRIBUTE_NORMAL);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  CORE_SHADER_ATTRIBUTE_TANGENT);
    FOR_EACH(it, m_aiAttribute)
    {
        if(it->second >= 0) 
            glBindAttribLocation(m_iProgram, it->second, it->first.c_str());
    }

    // bind output locations
    if(Core::Graphics->GetUniformBuffer())
    {
        for(int i = 0; i < CORE_SHADER_OUTPUT_COLORS; ++i)
            glBindFragDataLocation(m_iProgram, i, CORE_SHADER_OUTPUT_COLOR(i));
    }

    // link shader-program
    glLinkProgram(m_iProgram);
    m_iStatus = CORE_SHADER_LINKED;

    // bind global uniform buffer object
    if(Core::Graphics->GetUniformBuffer())
    {
        const int iBlock = glGetUniformBlockIndex(m_iProgram, CORE_SHADER_BUFFER_GLOBAL);
        if(iBlock >= 0) glUniformBlockBinding(m_iProgram, iBlock, CORE_SHADER_BUFFER_GLOBAL_NUM);
    }

    // check for errors
    int iStatus;
    glGetProgramiv(m_iProgram, GL_LINK_STATUS, &iStatus);
    ASSERT_IF(!iStatus)
    {
        this->__WriteLog();
        return CORE_INVALID_DATA;
    }

    return CORE_OK;
}


// ****************************************************************
// exit the shader-program
coreError coreProgram::__Exit()
{
    if(!m_iProgram) return CORE_INVALID_CALL;

    // disable still active shader-program
    if(s_pCurrent == this) coreProgram::Disable(true);

    // detach shader objects
    FOR_EACH(it, m_apShader)
    {
        if((*it)->GetShader())
            glDetachShader(m_iProgram, (*it)->GetShader());
    }

    // delete shader-program
    glDeleteProgram(m_iProgram);
    m_iProgram = 0;
    m_iStatus  = CORE_SHADER_DEFINED;
    
    // clear uniform locations and cache
    m_aiUniform.clear();
    m_avCache.clear();

    return CORE_OK;
}


// ****************************************************************
// write error-log
void coreProgram::__WriteLog()const
{
    // get length of error-log
    int iLength;
    glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &iLength);

    if(iLength)
    {
        // set logging level
        const int iLevel = Core::Log->GetLevel();
        Core::Log->SetLevel(0);

        // get error-log
        char* pcLog = new char[iLength];
        glGetProgramInfoLog(m_iProgram, iLength, NULL, pcLog);

        // write error-log
        Core::Log->Error(false, "Shader-Program could not be linked/validated");
        Core::Log->ListStart("Shader-Program Error Log");
        FOR_EACH(it, m_apShader)
            Core::Log->ListEntry("(%s)", (*it)->GetPath());
        Core::Log->ListEntry(pcLog);
        Core::Log->ListEnd();

        // reset logging level
        Core::Log->SetLevel(iLevel);

        SAFE_DELETE_ARRAY(pcLog)
    }
}