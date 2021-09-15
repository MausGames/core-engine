///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

coreString   coreShader ::s_asGlobalCode[2] = {"", ""};
coreSpinLock coreShader ::s_GlobalLock      = coreSpinLock();
coreProgram* coreProgram::s_pCurrent        = NULL;


// ****************************************************************
/* create static string lists */
template <const coreChar* pcString, coreUintW iLength, coreUintW iNum> struct coreStringList final
{
    coreChar       aacCharArray[iNum][iLength];
    coreHashString asHashString[iNum];

    coreStringList()noexcept {for(coreUintW i = 0u; i < iNum; ++i) {WARN_IF(coreUintW(coreData::PrintBase(aacCharArray[i], iLength, pcString, i)) >= iLength) {} asHashString[i] = aacCharArray[i];}}
    inline const coreHashString& operator [] (const coreUintW iIndex)const {ASSERT(iIndex < iNum) return asHashString[iIndex];}
};

#define __STRING_LIST(s,n,v)              \
    extern const coreChar v ## __a[] = s; \
    static const coreStringList<v ## __a, ARRAY_SIZE(v ## __a), n> v;

__STRING_LIST(CORE_SHADER_UNIFORM_LIGHT_POSITION,  CORE_GRAPHICS_LIGHTS,      s_asLightPosition)
__STRING_LIST(CORE_SHADER_UNIFORM_LIGHT_DIRECTION, CORE_GRAPHICS_LIGHTS,      s_asLightDirection)
__STRING_LIST(CORE_SHADER_UNIFORM_LIGHT_VALUE,     CORE_GRAPHICS_LIGHTS,      s_asLightValue)
__STRING_LIST(CORE_SHADER_UNIFORM_TEXTURE_2D,      CORE_TEXTURE_UNITS_2D,     s_asTexture2D)
__STRING_LIST(CORE_SHADER_UNIFORM_TEXTURE_SHADOW,  CORE_TEXTURE_UNITS_SHADOW, s_asTextureShadow)
__STRING_LIST(CORE_SHADER_OUTPUT_COLOR,            CORE_SHADER_OUTPUT_COLORS, s_asOutColor)


// ****************************************************************
/* constructor */
coreShader::coreShader(const coreChar* pcCustomCode)noexcept
: coreResource  ()
, m_iIdentifier (0u)
, m_iType       (0u)
, m_sCustomCode (pcCustomCode)
{
}


// ****************************************************************
/* destructor */
coreShader::~coreShader()
{
    this->Unload();
}


// ****************************************************************
/* load shader resource data */
coreStatus coreShader::Load(coreFile* pFile)
{
    coreFileScope oUnloader(pFile);

    WARN_IF(m_iIdentifier) return CORE_INVALID_CALL;
    if(!pFile)             return CORE_INVALID_INPUT;
    if(!pFile->GetData())  return CORE_ERROR_FILE;

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
    coreString sMainCode(r_cast<const coreChar*>(pFile->GetData()), pFile->GetSize());
    coreShader::__ReduceCodeSize(&sMainCode);

    // assemble the shader
    const coreChar* apcData[] = {s_asGlobalCode[0].c_str(),             pcTypeDef,                         pcQualityDef,                         m_sCustomCode.c_str(),             s_asGlobalCode[1].c_str(),             sMainCode.c_str()};
    const coreInt32 aiSize [] = {coreInt32(s_asGlobalCode[0].length()), coreInt32(std::strlen(pcTypeDef)), coreInt32(std::strlen(pcQualityDef)), coreInt32(m_sCustomCode.length()), coreInt32(s_asGlobalCode[1].length()), coreInt32(sMainCode.length())};
    STATIC_ASSERT(ARRAY_SIZE(apcData) == ARRAY_SIZE(aiSize))

    // create and compile the shader
    m_iIdentifier = glCreateShader(m_iType);
    glShaderSource (m_iIdentifier, ARRAY_SIZE(apcData), apcData, aiSize);
    glCompileShader(m_iIdentifier);

    // save properties
    m_sPath = pFile->GetPath();

    // check for errors
    GLint iStatus;
    glGetShaderiv(m_iIdentifier, GL_COMPILE_STATUS, &iStatus);
    WARN_IF(!iStatus)
    {
        // get length of info-log
        GLint iLength;
        glGetShaderiv(m_iIdentifier, GL_INFO_LOG_LENGTH, &iLength);

        if(iLength)
        {
            // get info-log
            coreChar* pcLog = new coreChar[iLength];
            glGetShaderInfoLog(m_iIdentifier, iLength, NULL, pcLog);

            // write info-log
            Core::Log->Warning("Shader (%s) could not be compiled", pFile->GetPath());
            Core::Log->ListStartWarning("Shader Log");
            Core::Log->ListAdd(pcLog);
            Core::Log->ListEnd();

            SAFE_DELETE_ARRAY(pcLog)
        }
        return CORE_INVALID_DATA;
    }

    Core::Log->Info("Shader (%s:%u) loaded", pFile->GetPath(), m_iIdentifier);
    return CORE_OK;
}


// ****************************************************************
/* unload shader resource data */
coreStatus coreShader::Unload()
{
    if(!m_iIdentifier) return CORE_INVALID_CALL;

    // delete shader
    glDeleteShader(m_iIdentifier);
    if(!m_sPath.empty()) Core::Log->Info("Shader (%s) unloaded", m_sPath.c_str());

    // reset properties
    m_sPath       = "";
    m_iIdentifier = 0u;
    m_iType       = 0u;

    return CORE_OK;
}


// ****************************************************************
/* load global shader code */
void coreShader::__LoadGlobalCode()
{
    coreSpinLocker oLocker(&s_GlobalLock);

    if(!s_asGlobalCode[0].empty()) return;

    // set global shader definitions
    s_asGlobalCode[0].assign(PRINT("#version %.0f \n", CORE_GL_SUPPORT(ARB_uniform_buffer_object) ? Core::Graphics->GetVersionGLSL()*100.0f : (DEFINED(_CORE_GLES_) ? 100.0f : 110.0f)));
    s_asGlobalCode[1].assign(PRINT("#define CORE_NUM_TEXTURES_2D     (%u) \n", CORE_TEXTURE_UNITS_2D));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_TEXTURES_SHADOW (%u) \n", CORE_TEXTURE_UNITS_SHADOW));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_LIGHTS          (%u) \n", CORE_GRAPHICS_LIGHTS));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_OUTPUTS         (%u) \n", CORE_SHADER_OUTPUT_COLORS));

    const auto nRetrieveFunc = [](const coreChar* pcPath)
    {
        // retrieve shader file
        coreFileScope pFile = Core::Manager::Resource->RetrieveFile(pcPath);
        WARN_IF(!pFile->GetData()) return;

        // copy data
        s_asGlobalCode[1].append(r_cast<const coreChar*>(pFile->GetData()), pFile->GetSize());
        s_asGlobalCode[1].append(1u, '\n');
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
/* reduce shader code size */
void coreShader::__ReduceCodeSize(coreString* OUTPUT psCode)
{
    // remove code comments
    for(coreUintW i = 0u; (i = psCode->find("//", i)) != coreString::npos; )
        psCode->erase(i, psCode->find_first_of('\n', i) - i);

    // remove redundant whitespaces
    psCode->replace("    ", " ");
}


// ****************************************************************
/* constructor */
coreProgram::coreProgram()noexcept
: coreResource     ()
, m_iIdentifier    (0u)
, m_apShader       {}
, m_apShaderHandle {}
, m_eStatus        (CORE_PROGRAM_NEW)
, m_aiUniform      {}
, m_aiAttribute    {}
, m_avCache        {}
, m_Sync           ()
{
}


// ****************************************************************
/* destructor */
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
/* load shader-program */
coreStatus coreProgram::Load(coreFile* pFile)
{
    ASSERT(!pFile)

    // check for sync object status
    const coreStatus eCheck = m_Sync.Check(0u, CORE_SYNC_CHECK_FLUSHED);
    if(eCheck == CORE_OK) m_eStatus = CORE_PROGRAM_FINISHED;
    if(eCheck >= CORE_OK) return eCheck;

    // check for shader-program status
    if(m_eStatus < CORE_PROGRAM_DEFINED) return CORE_BUSY;
    if(m_eStatus > CORE_PROGRAM_DEFINED) return CORE_INVALID_CALL;
    WARN_IF(m_iIdentifier)               return CORE_INVALID_CALL;

    // load all required shader objects
    if(m_apShader.empty())
    {
        FOR_EACH(it, m_apShaderHandle)
            m_apShader.emplace_back(*it);
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
            ASSERT(((*it) != (*et)) && ((*it) >= 0))

#endif

    // create shader-program
    m_iIdentifier = glCreateProgram();

    // attach shader objects
    FOR_EACH(it, m_apShader)
    {
        if((*it)->GetIdentifier())
            glAttachShader(m_iIdentifier, (*it)->GetIdentifier());
    }

    // bind default attribute locations
    glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_POSITION_NUM, CORE_SHADER_ATTRIBUTE_POSITION);
    glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_TEXCOORD_NUM, CORE_SHADER_ATTRIBUTE_TEXCOORD);
    glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   CORE_SHADER_ATTRIBUTE_NORMAL);
    glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  CORE_SHADER_ATTRIBUTE_TANGENT);

    // bind instancing attribute locations
    if(CORE_GL_SUPPORT(ARB_instanced_arrays) && CORE_GL_SUPPORT(ARB_uniform_buffer_object) && CORE_GL_SUPPORT(ARB_vertex_array_object) && CORE_GL_SUPPORT(ARB_half_float_vertex))
    {
        glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM,  CORE_SHADER_ATTRIBUTE_DIV_POSITION);
        glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,      CORE_SHADER_ATTRIBUTE_DIV_SIZE);
        glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM,  CORE_SHADER_ATTRIBUTE_DIV_ROTATION);
        glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM,      CORE_SHADER_ATTRIBUTE_DIV_DATA);
        glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,     CORE_SHADER_ATTRIBUTE_DIV_COLOR);
        glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM,  CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM);
    }

    // bind custom attribute locations
    FOR_EACH(it, m_aiAttribute)
    {
        if((*it) >= 0)
            glBindAttribLocation(m_iIdentifier, (*it), m_aiAttribute.get_string(it));
    }

    // bind output locations
    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        for(coreUintW i = 0u; i < CORE_SHADER_OUTPUT_COLORS; ++i)
            glBindFragDataLocation(m_iIdentifier, i, s_asOutColor[i].GetString());
    }

    // link shader-program
    glLinkProgram(m_iIdentifier);
    glUseProgram (m_iIdentifier);

    // bind texture units
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS_2D;     ++i) glUniform1i(glGetUniformLocation(m_iIdentifier, s_asTexture2D    [i].GetString()), i);
    for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS_SHADOW; ++i) glUniform1i(glGetUniformLocation(m_iIdentifier, s_asTextureShadow[i].GetString()), i + CORE_TEXTURE_SHADOW);

    // bind uniform buffer objects
    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        const GLuint iTransformBlock = glGetUniformBlockIndex(m_iIdentifier, CORE_SHADER_BUFFER_TRANSFORM);
        const GLuint iAmbientBlock   = glGetUniformBlockIndex(m_iIdentifier, CORE_SHADER_BUFFER_AMBIENT);
        if(iTransformBlock != GL_INVALID_INDEX) glUniformBlockBinding(m_iIdentifier, iTransformBlock, CORE_SHADER_BUFFER_TRANSFORM_NUM);
        if(iAmbientBlock   != GL_INVALID_INDEX) glUniformBlockBinding(m_iIdentifier, iAmbientBlock,   CORE_SHADER_BUFFER_AMBIENT_NUM);
    }

    // save properties
    FOR_EACH(it, m_apShader) m_sPath += PRINT("%s%s:%u", m_sPath.empty() ? "" : ", ", (*it).GetHandle()->GetName(), (*it)->GetIdentifier());

    // check for errors
    GLint iStatus;
    glGetProgramiv(m_iIdentifier, GL_LINK_STATUS, &iStatus);
    WARN_IF(!iStatus)
    {
        this->__WriteLog();
        return CORE_INVALID_DATA;
    }

    // create sync object
    const coreBool bSync = m_Sync.Create();
    if(!bSync) m_eStatus = CORE_PROGRAM_FINISHED;

    Core::Log->Info("Program (%s) loaded", m_sPath.c_str());
    this->__WriteInterface();
    return bSync ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
/* unload shader-program */
coreStatus coreProgram::Unload()
{
    if(!m_iIdentifier) return CORE_INVALID_CALL;

    // disable still active shader-program
    if(s_pCurrent == this) coreProgram::Disable(true);

    // disable shader objects
    m_apShader.clear();

    // delete shader-program (with implicit shader object detachment)
    glDeleteProgram(m_iIdentifier);
    if(!m_sPath.empty()) Core::Log->Info("Program (%s) unloaded", m_sPath.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_sPath       = "";
    m_iIdentifier = 0u;
    m_eStatus     = CORE_PROGRAM_DEFINED;

    // clear uniform locations and cache
    m_aiUniform.clear();
    m_avCache  .clear();

    return CORE_OK;
}


// ****************************************************************
/* enable the shader-program */
coreBool coreProgram::Enable()
{
    ASSERT(m_eStatus)

    // try to update global uniform data
    Core::Graphics->UpdateTransformation();
    Core::Graphics->UpdateAmbient();

    // check current shader-program
    if(s_pCurrent == this)                  return true;
    if(m_eStatus  != CORE_PROGRAM_FINISHED) return false;

    // set current shader-program
    s_pCurrent = this;
    glUseProgram(m_iIdentifier);

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
            this->SendUniform(s_asLightPosition [i], Core::Graphics->GetLight(i).vPosition);
            this->SendUniform(s_asLightDirection[i], Core::Graphics->GetLight(i).vDirection);
            this->SendUniform(s_asLightValue    [i], Core::Graphics->GetLight(i).vValue);
        }
    }

#if defined(_CORE_DEBUG_)   // # never in release

    // validate shader-program
    glValidateProgram(m_iIdentifier);

    // check for errors
    GLint iStatus;
    glGetProgramiv(m_iIdentifier, GL_VALIDATE_STATUS, &iStatus);
    WARN_IF(!iStatus)
    {
        this->__WriteLog();
        return false;
    }

#endif

    return true;
}


// ****************************************************************
/* disable the shader-program */
void coreProgram::Disable(const coreBool bFull)
{
    // reset current shader-program
    s_pCurrent = NULL;
    if(bFull) glUseProgram(0u);
}


// ****************************************************************
/* execute a compute shader-program */
coreStatus coreProgram::DispatchCompute(const coreUint32 iGroupsX, const coreUint32 iGroupsY, const coreUint32 iGroupsZ)
{
    ASSERT((m_eStatus >= CORE_PROGRAM_FINISHED) && (s_pCurrent == this))

    if(CORE_GL_SUPPORT(ARB_compute_shader))
    {
        // launch one or more compute work groups
        glDispatchCompute(iGroupsX, iGroupsY, iGroupsZ);

        return CORE_OK;
    }

    return CORE_ERROR_SUPPORT;
}


// ****************************************************************
/* send new uniform 2x2-matrix */
void coreProgram::SendUniform(const coreHashString& sName, const coreMatrix2& mMatrix, const coreBool bTranspose)
{
    // retrieve uniform location
    const coreInt8 iLocation = this->RetrieveUniform(sName);
    if(iLocation >= 0)
    {
        if(this->CheckCache(iLocation, coreVector4(mMatrix.arr(0u), mMatrix.arr(1u), mMatrix.arr(2u), mMatrix.arr(3u))))
        {
            // send new value
#if defined(_CORE_GLES_)
            glUniformMatrix2fv(iLocation, 1, false, bTranspose ? mMatrix.Transposed().ptr() : mMatrix.ptr());
#else
            glUniformMatrix2fv(iLocation, 1, bTranspose, mMatrix.ptr());
#endif
        }
    }
}


// ****************************************************************
/* send new uniform 3x3-matrix */
void coreProgram::SendUniform(const coreHashString& sName, const coreMatrix3& mMatrix, const coreBool bTranspose)
{
    // retrieve uniform location
    const coreInt8 iLocation = this->RetrieveUniform(sName);
    if(iLocation >= 0)
    {
        // send new value
#if defined(_CORE_GLES_)
        glUniformMatrix3fv(iLocation, 1, false, bTranspose ? mMatrix.Transposed().ptr() : mMatrix.ptr());
#else
        glUniformMatrix3fv(iLocation, 1, bTranspose, mMatrix.ptr());
#endif
    }
}


// ****************************************************************
/* send new uniform 4x4-matrix */
void coreProgram::SendUniform(const coreHashString& sName, const coreMatrix4& mMatrix, const coreBool bTranspose)
{
    // retrieve uniform location
    const coreInt8 iLocation = this->RetrieveUniform(sName);
    if(iLocation >= 0)
    {
        // send new value
#if defined(_CORE_GLES_)
        glUniformMatrix4fv(iLocation, 1, false, bTranspose ? mMatrix.Transposed().ptr() : mMatrix.ptr());
#else
        glUniformMatrix4fv(iLocation, 1, bTranspose, mMatrix.ptr());
#endif
    }
}


// ****************************************************************
/* write info-log to log file */
void coreProgram::__WriteLog()const
{
    // get length of info-log
    GLint iLength;
    glGetProgramiv(m_iIdentifier, GL_INFO_LOG_LENGTH, &iLength);

    if(iLength)
    {
        // get info-log
        coreChar* pcLog = new coreChar[iLength];
        glGetProgramInfoLog(m_iIdentifier, iLength, NULL, pcLog);

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
/* write interface to log file */
void coreProgram::__WriteInterface()const
{
    if(!Core::Config->GetBool(CORE_CONFIG_BASE_DEBUGMODE) && !DEFINED(_CORE_DEBUG_)) return;

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
            glGetProgramInterfaceiv(m_iIdentifier, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &iNumInput);
            glGetProgramInterfaceiv(m_iIdentifier, GL_UNIFORM,       GL_ACTIVE_RESOURCES, &iNumUniform);

            // write active vertex attributes (name, location)
            Core::Log->ListDeeper(CORE_LOG_BOLD("Attributes:") " %d", iNumInput);
            for(coreUintW i = 0u, ie = iNumInput; i < ie; ++i)
            {
                glGetProgramResourceName(m_iIdentifier, GL_PROGRAM_INPUT, i, ARRAY_SIZE(acName), NULL, acName);
                glGetProgramResourceiv  (m_iIdentifier, GL_PROGRAM_INPUT, i, 1, aiProperty, 1,   NULL, aiValue);

                Core::Log->ListAdd("%s: %i", acName, aiValue[0]);
            }
            Core::Log->ListEnd();

            // write active uniforms (name, location, block offset)
            Core::Log->ListDeeper(CORE_LOG_BOLD("Uniforms:") " %d", iNumUniform);
            for(coreUintW i = 0u, ie = iNumUniform; i < ie; ++i)
            {
                glGetProgramResourceName(m_iIdentifier, GL_UNIFORM, i, ARRAY_SIZE(acName), NULL, acName);
                glGetProgramResourceiv  (m_iIdentifier, GL_UNIFORM, i, 2, aiProperty, 2,   NULL, aiValue);

                Core::Log->ListAdd("%s: %i/%i", acName, aiValue[0], aiValue[1]);
            }
            Core::Log->ListEnd();
        }
        else
        {
            // get number of active shader-program resources
            glGetProgramiv(m_iIdentifier, GL_ACTIVE_ATTRIBUTES, &iNumInput);
            glGetProgramiv(m_iIdentifier, GL_ACTIVE_UNIFORMS,   &iNumUniform);

            // write only numbers
            Core::Log->ListAdd(CORE_LOG_BOLD("Attributes:") " %d", iNumInput);
            Core::Log->ListAdd(CORE_LOG_BOLD("Uniforms:")   " %d", iNumUniform);
        }
    }
    Core::Log->ListEnd();
}