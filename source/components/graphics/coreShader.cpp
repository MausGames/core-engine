//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

std::string  coreShader ::s_asGlobalCode[2] = {"", ""};
coreProgram* coreProgram::s_pCurrent        = NULL;


// ****************************************************************
// create static string lists
template <const coreChar* pcString, coreUintW iLength, coreUintW iNum> struct sStringList final
{
    coreChar aacEntry[iNum][iLength];

    sStringList()noexcept {for(coreUintW i = 0u; i < iNum; ++i) std::sprintf(aacEntry[i], pcString, i);}
    inline const coreChar* operator [] (const coreUintW iIndex)const {return aacEntry[iIndex];}
};

#define STRING_ARRAY(s,n,v)               \
    extern const coreChar v ## __a[] = s; \
    const sStringList<v ## __a, ARRAY_SIZE(v ## __a), n> v;

STRING_ARRAY(CORE_SHADER_UNIFORM_LIGHT_POSITION,  CORE_GRAPHICS_LIGHTS,      avLightPosition)
STRING_ARRAY(CORE_SHADER_UNIFORM_LIGHT_DIRECTION, CORE_GRAPHICS_LIGHTS,      avLightDirection)
STRING_ARRAY(CORE_SHADER_UNIFORM_LIGHT_VALUE,     CORE_GRAPHICS_LIGHTS,      avLightValue)
STRING_ARRAY(CORE_SHADER_UNIFORM_TEXTURE_2D,      CORE_TEXTURE_UNITS_2D,     avTexture2D)
STRING_ARRAY(CORE_SHADER_UNIFORM_TEXTURE_SHADOW,  CORE_TEXTURE_UNITS_SHADOW, avTextureShadow)
STRING_ARRAY(CORE_SHADER_OUTPUT_COLOR,            CORE_SHADER_OUTPUT_COLORS, avOutColor)


// ****************************************************************
// constructor
coreShader::coreShader()noexcept
: coreShader ("")
{
}

coreShader::coreShader(const coreChar* pcCustomCode)noexcept
: coreResource  ()
, m_iShader     (0u)
, m_iType       (0u)
, m_sCustomCode (pcCustomCode)
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
coreStatus coreShader::Load(coreFile* pFile)
{
    coreFileUnload oUnload(pFile);

    WARN_IF(m_iShader)    return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_ERROR_FILE;

    // extract file extension
    const coreChar* pcExtension = coreData::StrLower(coreData::StrExtension(pFile->GetPath()));

    // set shader type
    const coreChar* pcTypeDef;
         if(!std::strcmp(pcExtension, "vs")  || !std::strcmp(pcExtension, "vert")) {m_iType = GL_VERTEX_SHADER;          pcTypeDef = "#define _CORE_VERTEX_SHADER_          (1) \n";}
    else if(!std::strcmp(pcExtension, "tcs") || !std::strcmp(pcExtension, "tesc")) {m_iType = GL_TESS_CONTROL_SHADER;    pcTypeDef = "#define _CORE_TESS_CONTROL_SHADER_    (1) \n";}
    else if(!std::strcmp(pcExtension, "tes") || !std::strcmp(pcExtension, "tese")) {m_iType = GL_TESS_EVALUATION_SHADER; pcTypeDef = "#define _CORE_TESS_EVALUATION_SHADER_ (1) \n";}
    else if(!std::strcmp(pcExtension, "gs")  || !std::strcmp(pcExtension, "geom")) {m_iType = GL_GEOMETRY_SHADER;        pcTypeDef = "#define _CORE_GEOMETRY_SHADER_        (1) \n";}
    else if(!std::strcmp(pcExtension, "fs")  || !std::strcmp(pcExtension, "frag")) {m_iType = GL_FRAGMENT_SHADER;        pcTypeDef = "#define _CORE_FRAGMENT_SHADER_        (1) \n";}
    else if(!std::strcmp(pcExtension, "cs")  || !std::strcmp(pcExtension, "comp")) {m_iType = GL_COMPUTE_SHADER;         pcTypeDef = "#define _CORE_COMPUTE_SHADER_         (1) \n";}
    else
    {
        Core::Log->Warning("Shader (%s) could not be identified (valid extensions: vs, vert, tcs, tesc, tes, tese, gs, geom, fs, frag, cs, comp)", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // check for OpenGL extensions
    if((m_iType == GL_TESS_CONTROL_SHADER || m_iType == GL_TESS_EVALUATION_SHADER) && !CORE_GL_SUPPORT(ARB_tessellation_shader)) return CORE_OK;
    if((m_iType == GL_GEOMETRY_SHADER)                                             && !CORE_GL_SUPPORT(ARB_geometry_shader4))    return CORE_OK;
    if((m_iType == GL_COMPUTE_SHADER)                                              && !CORE_GL_SUPPORT(ARB_compute_shader))      return CORE_OK;

    // load quality level and global shader data
    const coreChar* pcQualityDef = PRINT("#define _CORE_QUALITY_ (%d) \n", Core::Config->GetInt(CORE_CONFIG_GRAPHICS_QUALITY));
    coreShader::__LoadGlobalCode();

    // reduce shader code size
    std::string sMainCode(r_cast<const coreChar*>(pFile->GetData()), pFile->GetSize());
    coreShader::__ReduceCodeSize(&sMainCode);

    // assemble the shader
    const coreInt32 iSize  = 6 + s_asGlobalCode[0].length() + std::strlen(pcTypeDef) + std::strlen(pcQualityDef) + m_sCustomCode.length() + s_asGlobalCode[1].length() + sMainCode.length();
    coreChar*       pcData = new coreChar[iSize];
    std::snprintf(pcData, iSize, "%s\n%s\n%s\n%s\n%s\n%s", s_asGlobalCode[0].c_str(), pcTypeDef, pcQualityDef, m_sCustomCode.c_str(), s_asGlobalCode[1].c_str(), sMainCode.c_str());

    // create and compile the shader
    m_iShader = glCreateShader(m_iType);
    glShaderSource (m_iShader, 1, &pcData, &iSize);
    glCompileShader(m_iShader);

    // free required shader memory
    SAFE_DELETE_ARRAY(pcData)

    // save properties
    m_sPath = pFile->GetPath();

    // check for errors
    GLint iStatus;
    glGetShaderiv(m_iShader, GL_COMPILE_STATUS, &iStatus);
    if(!iStatus)
    {
        // get length of info-log
        GLint iLength;
        glGetShaderiv(m_iShader, GL_INFO_LOG_LENGTH, &iLength);

        if(iLength)
        {
            // get info-log
            coreChar* pcLog = new coreChar[iLength];
            glGetShaderInfoLog(m_iShader, iLength, NULL, pcLog);

            // write info-log
            Core::Log->Warning("Shader (%s) could not be compiled", pFile->GetPath());
            Core::Log->ListStartWarning("Shader Log");
            Core::Log->ListAdd(pcLog);
            Core::Log->ListEnd();

            SAFE_DELETE_ARRAY(pcLog)
        }
        return CORE_INVALID_DATA;
    }

    Core::Log->Info("Shader (%s:%u) loaded", pFile->GetPath(), m_iShader);
    return CORE_OK;
}


// ****************************************************************
// unload shader resource data
coreStatus coreShader::Unload()
{
    if(!m_iShader) return CORE_INVALID_CALL;

    // delete shader
    glDeleteShader(m_iShader);
    if(!m_sPath.empty()) Core::Log->Info("Shader (%s) unloaded", m_sPath.c_str());

    // reset properties
    m_sPath   = "";
    m_iShader = 0u;
    m_iType   = 0u;

    return CORE_OK;
}


// ****************************************************************
// load global shader code
void coreShader::__LoadGlobalCode()
{
    if(!s_asGlobalCode[0].empty()) return;

    // set global shader definitions
    s_asGlobalCode[0].assign(PRINT("#version %.0f \n", CORE_GL_SUPPORT(ARB_uniform_buffer_object) ? Core::Graphics->VersionGLSL()*100.0f : (DEFINED(_CORE_GLES_) ? 100.0f : 110.0f)));
    s_asGlobalCode[1].assign(PRINT("#define CORE_NUM_TEXTURES_2D     (%d) \n", CORE_TEXTURE_UNITS_2D));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_TEXTURES_SHADOW (%d) \n", CORE_TEXTURE_UNITS_SHADOW));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_LIGHTS          (%d) \n", CORE_GRAPHICS_LIGHTS));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_OUTPUTS         (%d) \n", CORE_SHADER_OUTPUT_COLORS));

    auto nRetrieveFunc = [&](const coreChar* pcPath)
    {
        // retrieve shader file
        coreFile* pFile = Core::Manager::Resource->RetrieveFile(pcPath);
        WARN_IF(!pFile->GetData()) return;

        // copy and unload data
        s_asGlobalCode[1].append(r_cast<const coreChar*>(pFile->GetData()), pFile->GetSize());
        s_asGlobalCode[1].append(1u, '\n');
        pFile->UnloadData();
    };
    nRetrieveFunc("data/shaders/global.glsl");
    nRetrieveFunc("data/shaders/custom.glsl");

    // reduce shader code size
    coreShader::__ReduceCodeSize(&s_asGlobalCode[1]);

    // reduce memory consumption
    s_asGlobalCode[0].shrink_to_fit();
    s_asGlobalCode[1].shrink_to_fit();
}


// ****************************************************************
// reduce shader code size
void coreShader::__ReduceCodeSize(std::string* OUTPUT psCode)
{
    // remove code comments
    for(coreUintW i = 0u; (i = psCode->find("//", i)) != std::string::npos; )
        psCode->erase(i, psCode->find_first_of('\n', i) - i);

    // remove redundant whitespaces
    coreData::StrReplace(psCode, "    ", " ");
}


// ****************************************************************
// constructor
coreProgram::coreProgram()noexcept
: coreResource     ()
, m_iProgram       (0u)
, m_apShader       {}
, m_apShaderHandle {}
, m_iStatus        (CORE_PROGRAM_NEW)
, m_aiUniform      {}
, m_aiAttribute    {}
, m_avCache        {}
, m_Sync           ()
{
}


// ****************************************************************
// destructor
coreProgram::~coreProgram()
{
    // unload shader-program
    this->Unload();

    // remove all shader objects and attribute locations
    m_apShader      .clear();
    m_apShaderHandle.clear();
    m_aiAttribute   .clear();
}


// ****************************************************************
// load shader-program
coreStatus coreProgram::Load(coreFile* pFile)
{
    // check for sync object status
    const coreStatus iCheck = m_Sync.Check(0u, CORE_SYNC_CHECK_FLUSHED);
    if(iCheck == CORE_OK) m_iStatus = CORE_PROGRAM_FINISHED;
    if(iCheck >= CORE_OK) return iCheck;

    // check for shader-program status
    if(m_iStatus < CORE_PROGRAM_DEFINED) return CORE_BUSY;
    if(m_iStatus > CORE_PROGRAM_DEFINED) return CORE_INVALID_CALL;
    WARN_IF(m_iProgram)                  return CORE_INVALID_CALL;

    // load all required shader objects
    if(m_apShader.empty())
    {
        FOR_EACH(it, m_apShaderHandle)
            m_apShader.push_back(*it);
    }
    FOR_EACH(it, m_apShader)
    {
        it->GetHandle()->Update();
        if(!it->IsUsable()) return CORE_BUSY;
    }

#if defined(_CORE_DEBUG_)

    // check for duplicate shader objects
    FOR_EACH(it, m_apShader)
        FOR_EACH_SET(et, it+1u, m_apShader)
            ASSERT(std::strcmp((*it)->GetPath(), (*et)->GetPath()))

    // check for duplicate attribute locations
    FOR_EACH(it, m_aiAttribute)
        FOR_EACH_SET(et, it+1u, m_aiAttribute)
            ASSERT((*it) != (*et) && (*it) >= 0)

#endif

    // create shader-program
    m_iProgram = glCreateProgram();

    // attach shader objects
    FOR_EACH(it, m_apShader)
    {
        if((*it)->GetShader())
            glAttachShader(m_iProgram, (*it)->GetShader());
    }

    // bind default attribute locations
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_POSITION_NUM, CORE_SHADER_ATTRIBUTE_POSITION);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TEXCOORD_NUM, CORE_SHADER_ATTRIBUTE_TEXCOORD);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   CORE_SHADER_ATTRIBUTE_NORMAL);
    glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  CORE_SHADER_ATTRIBUTE_TANGENT);

    // bind instancing attribute locations
    if(CORE_GL_SUPPORT(ARB_instanced_arrays) && CORE_GL_SUPPORT(ARB_uniform_buffer_object) && CORE_GL_SUPPORT(ARB_vertex_array_object))
    {
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM,  CORE_SHADER_ATTRIBUTE_DIV_POSITION);
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,      CORE_SHADER_ATTRIBUTE_DIV_SIZE);
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM,  CORE_SHADER_ATTRIBUTE_DIV_ROTATION);
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM,      CORE_SHADER_ATTRIBUTE_DIV_DATA);
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,     CORE_SHADER_ATTRIBUTE_DIV_COLOR);
        glBindAttribLocation(m_iProgram, CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM,  CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM);
    }

    // bind custom attribute locations
    FOR_EACH(it, m_aiAttribute)
    {
        if((*it) >= 0)
            glBindAttribLocation(m_iProgram, (*it), *m_aiAttribute.get_key(it));
    }

    // bind output locations
    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        for(coreUintW i = 0u; i < CORE_SHADER_OUTPUT_COLORS; ++i)
            glBindFragDataLocation(m_iProgram, i, avOutColor[i]);
    }

    // link shader-program
    glLinkProgram(m_iProgram);
    glUseProgram (m_iProgram);

    // bind texture units
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS_2D;     ++i) glUniform1i(glGetUniformLocation(m_iProgram, avTexture2D    [i]), i);
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS_SHADOW; ++i) glUniform1i(glGetUniformLocation(m_iProgram, avTextureShadow[i]), i + CORE_TEXTURE_SHADOW);

    // bind uniform buffer objects
    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        const GLuint iTransformBlock = glGetUniformBlockIndex(m_iProgram, CORE_SHADER_BUFFER_TRANSFORM);
        const GLuint iAmbientBlock   = glGetUniformBlockIndex(m_iProgram, CORE_SHADER_BUFFER_AMBIENT);
        if(iTransformBlock != GL_INVALID_INDEX) glUniformBlockBinding(m_iProgram, iTransformBlock, CORE_SHADER_BUFFER_TRANSFORM_NUM);
        if(iAmbientBlock   != GL_INVALID_INDEX) glUniformBlockBinding(m_iProgram, iAmbientBlock,   CORE_SHADER_BUFFER_AMBIENT_NUM);
    }

    // save properties
    FOR_EACH(it, m_apShader)
    {
        m_sPath += (*it).GetHandle()->GetName();
        m_sPath += ":";
    }
    m_sPath.pop_back();

    // check for errors
    GLint iStatus;
    glGetProgramiv(m_iProgram, GL_LINK_STATUS, &iStatus);
    if(!iStatus)
    {
        this->__WriteLog();
        return CORE_INVALID_DATA;
    }

    // create sync object
    const coreBool bSync = m_Sync.Create();
    if(!bSync) m_iStatus = CORE_PROGRAM_FINISHED;

    Core::Log->Info("Program (%s:%u) loaded", m_sPath.c_str(), m_iProgram);
    this->__WriteInterface();
    return bSync ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
// unload shader-program
coreStatus coreProgram::Unload()
{
    if(!m_iProgram) return CORE_INVALID_CALL;

    // disable still active shader-program
    if(s_pCurrent == this) coreProgram::Disable(true);

    // disable shader objects
    m_apShader.clear();

    // delete shader-program (with implicit shader object detachment)
    glDeleteProgram(m_iProgram);
    if(!m_sPath.empty()) Core::Log->Info("Program (%s) unloaded", m_sPath.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_sPath    = "";
    m_iProgram = 0u;
    m_iStatus  = CORE_PROGRAM_DEFINED;

    // clear uniform locations and cache
    m_aiUniform.clear();
    m_avCache  .clear();

    return CORE_OK;
}


// ****************************************************************
// enable the shader-program
coreBool coreProgram::Enable()
{
    ASSERT(m_iStatus)

    // try to update global uniform data
    Core::Graphics->UpdateTransformation();
    Core::Graphics->UpdateAmbient();

    // check current shader-program
    if(s_pCurrent == this)                  return true;
    if(m_iStatus  != CORE_PROGRAM_FINISHED) return false;

    // set current shader-program
    s_pCurrent = this;
    glUseProgram(m_iProgram);

    // forward global uniform data without UBOs
    if(!CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        const coreMatrix4 mViewProj = Core::Graphics->GetCamera() * Core::Graphics->GetPerspective();

        // forward transformation data
        this->SendUniform(CORE_SHADER_UNIFORM_VIEWPROJ,    mViewProj,                        false);
        this->SendUniform(CORE_SHADER_UNIFORM_CAMERA,      Core::Graphics->GetCamera(),      false);
        this->SendUniform(CORE_SHADER_UNIFORM_PERSPECTIVE, Core::Graphics->GetPerspective(), false);
        this->SendUniform(CORE_SHADER_UNIFORM_ORTHO,       Core::Graphics->GetOrtho(),       false);
        this->SendUniform(CORE_SHADER_UNIFORM_RESOLUTION,  Core::Graphics->GetViewResolution());
        this->SendUniform(CORE_SHADER_UNIFORM_CAMPOSITION, Core::Graphics->GetCamPosition());

        // forward ambient data
        for(coreUintW i = 0u; i < CORE_GRAPHICS_LIGHTS; ++i)
        {
            this->SendUniform(avLightPosition [i], Core::Graphics->GetLight(i).vPosition);
            this->SendUniform(avLightDirection[i], Core::Graphics->GetLight(i).vDirection);
            this->SendUniform(avLightValue    [i], Core::Graphics->GetLight(i).vValue);
        }
    }

#if defined(_CORE_DEBUG_)   // # never in release

    // validate shader-program
    glValidateProgram(m_iProgram);

    // check for errors
    GLint iStatus;
    glGetProgramiv(m_iProgram, GL_VALIDATE_STATUS, &iStatus);
    if(!iStatus)
    {
        this->__WriteLog();
        return false;
    }

#endif

    return true;
}


// ****************************************************************
// disable the shader-program
void coreProgram::Disable(const coreBool bFull)
{
    // reset current shader-program
    s_pCurrent = NULL;
    if(bFull) glUseProgram(0u);
}


// ****************************************************************
// execute a compute shader-program
void coreProgram::DispatchCompute(const coreUint32 iGroupsX, const coreUint32 iGroupsY, const coreUint32 iGroupsZ)
{
    ASSERT(m_iStatus >= CORE_PROGRAM_FINISHED && s_pCurrent == this)

    if(CORE_GL_SUPPORT(ARB_compute_shader))
    {
        // launch one or more compute work groups
        glDispatchCompute(iGroupsX, iGroupsY, iGroupsZ);
    }
}


// ****************************************************************
// send new uniform 2x2-matrix
void coreProgram::SendUniform(const coreChar* pcName, const coreMatrix2& mMatrix, const coreBool bTranspose)
{
    // retrieve uniform location
    const coreInt8 iLocation = this->RetrieveUniform(pcName);
    if(iLocation >= 0)
    {
        if(this->CheckCache(iLocation, coreVector4(mMatrix[0], mMatrix[1], mMatrix[2], mMatrix[3])))
        {
            // send new value
#if defined(_CORE_GLES_)
            glUniformMatrix2fv(iLocation, 1, false, bTranspose ? mMatrix.Transposed() : mMatrix);
#else
            glUniformMatrix2fv(iLocation, 1, bTranspose, mMatrix);
#endif
        }
    }
}


// ****************************************************************
// send new uniform 3x3-matrix
void coreProgram::SendUniform(const coreChar* pcName, const coreMatrix3& mMatrix, const coreBool bTranspose)
{
    // retrieve uniform location
    const coreInt8 iLocation = this->RetrieveUniform(pcName);
    if(iLocation >= 0)
    {
        // send new value
#if defined(_CORE_GLES_)
        glUniformMatrix3fv(iLocation, 1, false, bTranspose ? mMatrix.Transposed() : mMatrix);
#else
        glUniformMatrix3fv(iLocation, 1, bTranspose, mMatrix);
#endif
    }
}


// ****************************************************************
// send new uniform 4x4-matrix
void coreProgram::SendUniform(const coreChar* pcName, const coreMatrix4& mMatrix, const coreBool bTranspose)
{
    // retrieve uniform location
    const coreInt8 iLocation = this->RetrieveUniform(pcName);
    if(iLocation >= 0)
    {
        // send new value
#if defined(_CORE_GLES_)
        glUniformMatrix4fv(iLocation, 1, false, bTranspose ? mMatrix.Transposed() : mMatrix);
#else
        glUniformMatrix4fv(iLocation, 1, bTranspose, mMatrix);
#endif
    }
}


// ****************************************************************
// write info-log to log file
void coreProgram::__WriteLog()const
{
    // get length of info-log
    GLint iLength;
    glGetProgramiv(m_iProgram, GL_INFO_LOG_LENGTH, &iLength);

    if(iLength)
    {
        // get info-log
        coreChar* pcLog = new coreChar[iLength];
        glGetProgramInfoLog(m_iProgram, iLength, NULL, pcLog);

        // write info-log
        Core::Log->Warning("Program (%s) could not be linked or validated", m_sPath.c_str());
        Core::Log->ListStartWarning("Program Log");
        {
            FOR_EACH(it, m_apShader)
                Core::Log->ListAdd("%s (%s)", it->GetHandle()->GetName(), (*it)->GetPath());
            Core::Log->ListAdd(pcLog);
        }
        Core::Log->ListEnd();

        SAFE_DELETE_ARRAY(pcLog)
    }
}


// ****************************************************************
// write interface to log file
void coreProgram::__WriteInterface()const
{
    if(!Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) && !DEFINED(_CORE_DEBUG_)) return;

    Core::Log->ListStartInfo("Program Interface");
    {
        GLint iNumInput;
        GLint iNumUniform;

        if(CORE_GL_SUPPORT(ARB_program_interface_query))
        {
            coreChar acName[64];
            GLint    aiValue[2];

            constexpr GLenum aiProperty[] = {GL_LOCATION, GL_OFFSET};

            // get number of active shader-program resources
            glGetProgramInterfaceiv(m_iProgram, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &iNumInput);
            glGetProgramInterfaceiv(m_iProgram, GL_UNIFORM,       GL_ACTIVE_RESOURCES, &iNumUniform);

            // write active vertex attributes (name, location)
            Core::Log->ListDeeper(CORE_LOG_BOLD("Attributes:") " %d", iNumInput);
            for(coreUintW i = 0u, ie = iNumInput; i < ie; ++i)
            {
                glGetProgramResourceName(m_iProgram, GL_PROGRAM_INPUT, i, ARRAY_SIZE(acName), NULL, acName);
                glGetProgramResourceiv  (m_iProgram, GL_PROGRAM_INPUT, i, 1, aiProperty, 1,   NULL, aiValue);

                Core::Log->ListAdd("%s: %i", acName, aiValue[0]);
            }
            Core::Log->ListEnd();

            // write active uniforms (name, location, block offset)
            Core::Log->ListDeeper(CORE_LOG_BOLD("Uniforms:") " %d", iNumUniform);
            for(coreUintW i = 0u, ie = iNumUniform; i < ie; ++i)
            {
                glGetProgramResourceName(m_iProgram, GL_UNIFORM, i, ARRAY_SIZE(acName), NULL, acName);
                glGetProgramResourceiv  (m_iProgram, GL_UNIFORM, i, 2, aiProperty, 2,   NULL, aiValue);

                Core::Log->ListAdd("%s: %i/%i", acName, aiValue[0], aiValue[1]);
            }
            Core::Log->ListEnd();
        }
        else
        {
            // get number of active shader-program resources
            glGetProgramiv(m_iProgram, GL_ACTIVE_ATTRIBUTES, &iNumInput);
            glGetProgramiv(m_iProgram, GL_ACTIVE_UNIFORMS,   &iNumUniform);

            // write only numbers
            Core::Log->ListAdd(CORE_LOG_BOLD("Attributes:") " %d", iNumInput);
            Core::Log->ListAdd(CORE_LOG_BOLD("Uniforms:")   " %d", iNumUniform);
        }
    }
    Core::Log->ListEnd();
}