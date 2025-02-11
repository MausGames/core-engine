///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

coreString                 coreShader ::s_asGlobalCode[8] = {};
coreSpinLock               coreShader ::s_GlobalLock      = coreSpinLock();
coreMapStr<coreString>     coreShader ::s_asIncludeCode   = {};
coreRecursiveLock          coreShader ::s_IncludeLock     = coreRecursiveLock();
coreProgram*               coreProgram::s_pCurrent        = NULL;
coreProgram::coreBinaryMap coreProgram::s_aBinaryMap      = {};
coreUint32                 coreProgram::s_iBinarySize     = 0u;
coreSpinLock               coreProgram::s_BinaryLock      = coreSpinLock();


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
    static const coreChar v ## __a[] = s; \
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
, m_iHash       (0u)
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
    const coreChar* pcExtension = coreData::StrToLower(coreData::StrExtension(pFile->GetPath()));

    // set shader type
    const coreChar* pcTypeDef;
    coreUintW       iTypeIndex;
         if(!std::memcmp(pcExtension, "vert", 4u)) {m_iType = GL_VERTEX_SHADER;          pcTypeDef = "#define _CORE_VERTEX_SHADER_"          " (1) \n"; iTypeIndex = 2u;}
    else if(!std::memcmp(pcExtension, "tesc", 4u)) {m_iType = GL_TESS_CONTROL_SHADER;    pcTypeDef = "#define _CORE_TESS_CONTROL_SHADER_"    " (1) \n"; iTypeIndex = 3u;}
    else if(!std::memcmp(pcExtension, "tese", 4u)) {m_iType = GL_TESS_EVALUATION_SHADER; pcTypeDef = "#define _CORE_TESS_EVALUATION_SHADER_" " (1) \n"; iTypeIndex = 4u;}
    else if(!std::memcmp(pcExtension, "geom", 4u)) {m_iType = GL_GEOMETRY_SHADER;        pcTypeDef = "#define _CORE_GEOMETRY_SHADER_"        " (1) \n"; iTypeIndex = 5u;}
    else if(!std::memcmp(pcExtension, "frag", 4u)) {m_iType = GL_FRAGMENT_SHADER;        pcTypeDef = "#define _CORE_FRAGMENT_SHADER_"        " (1) \n"; iTypeIndex = 6u;}
    else if(!std::memcmp(pcExtension, "comp", 4u)) {m_iType = GL_COMPUTE_SHADER;         pcTypeDef = "#define _CORE_COMPUTE_SHADER_"         " (1) \n"; iTypeIndex = 7u;}
    else
    {
        Core::Log->Warning("Shader (%s) could not be identified (valid extensions: vert, tesc, tese, geom, frag, comp)", m_sName.c_str());
        return CORE_INVALID_DATA;
    }

    // check for OpenGL extensions
    if((m_iType == GL_TESS_CONTROL_SHADER)    && !CORE_GL_SUPPORT(ARB_tessellation_shader)) return CORE_OK;
    if((m_iType == GL_TESS_EVALUATION_SHADER) && !CORE_GL_SUPPORT(ARB_tessellation_shader)) return CORE_OK;
    if((m_iType == GL_GEOMETRY_SHADER)        && !CORE_GL_SUPPORT(ARB_geometry_shader4))    return CORE_OK;
    if((m_iType == GL_COMPUTE_SHADER)         && !CORE_GL_SUPPORT(ARB_compute_shader))      return CORE_OK;

    // load quality level and global shader data
    const coreChar* pcQualityDef = PRINT("#define _CORE_QUALITY_ (%d) \n", Core::Config->GetInt(CORE_CONFIG_GRAPHICS_QUALITY));
    coreShader::__LoadGlobalCode();

    // define separate entry-point (main-function needs to be at the bottom)
    constexpr coreChar acEntryPoint[] = "\n void main() {ShaderMain();}";

    // parse and adapt shader code
    coreString sMainCode(r_cast<const coreChar*>(pFile->GetData()), pFile->GetSize());
    coreShader::__ReduceSize     (&sMainCode);
    coreShader::__ResolveIncludes(&sMainCode, pFile);

    // assemble the shader
    const coreChar* apcData[] = {s_asGlobalCode[0].c_str(),             pcTypeDef,                         pcQualityDef,                         m_sCustomCode.c_str(),             s_asGlobalCode[1].c_str(),             s_asGlobalCode[iTypeIndex].c_str(),             sMainCode.c_str(),             acEntryPoint};
    const coreInt32 aiSize [] = {coreInt32(s_asGlobalCode[0].length()), coreInt32(std::strlen(pcTypeDef)), coreInt32(std::strlen(pcQualityDef)), coreInt32(m_sCustomCode.length()), coreInt32(s_asGlobalCode[1].length()), coreInt32(s_asGlobalCode[iTypeIndex].length()), coreInt32(sMainCode.length()), coreInt32(ARRAY_SIZE(acEntryPoint) - 1u)};
    STATIC_ASSERT(ARRAY_SIZE(apcData) == ARRAY_SIZE(aiSize))

    // create and compile the shader
    m_iIdentifier = glCreateShader(m_iType);
    glShaderSource (m_iIdentifier, ARRAY_SIZE(apcData), apcData, aiSize);
    glCompileShader(m_iIdentifier);

    // generate shader code hash-value
    ASSERT(!m_iHash)
    for(coreUintW i = 0u; i < ARRAY_SIZE(apcData); ++i)
    {
        m_iHash = coreMath::HashCombine64(m_iHash, coreHashXXH64(r_cast<const coreByte*>(apcData[i]), aiSize[i]));
    }

    // add debug label
    Core::Graphics->LabelOpenGL(GL_SHADER, m_iIdentifier, m_sName.c_str());

    Core::Log->Info("Shader (%s) loaded", m_sName.c_str());
    return CORE_OK;
}


// ****************************************************************
/* unload shader resource data */
coreStatus coreShader::Unload()
{
    if(!m_iIdentifier) return CORE_INVALID_CALL;

    // delete shader
    glDeleteShader(m_iIdentifier);
    if(!m_sName.empty()) Core::Log->Info("Shader (%s) unloaded", m_sName.c_str());

    // reset properties
    m_iIdentifier = 0u;
    m_iType       = 0u;
    m_iHash       = 0u;

    return CORE_OK;
}


// ****************************************************************
/* clear global shader code */
void coreShader::ClearGlobalCode()
{
    const coreSpinLocker oLocker1(&s_GlobalLock);
    const coreSpinLocker oLocker2(&s_IncludeLock);

    // delete global shader code
    for(coreUintW i = 0u; i < ARRAY_SIZE(s_asGlobalCode); ++i)
    {
        s_asGlobalCode[i].clear();
    }

    // also delete include shader code
    s_asIncludeCode.clear();
}


// ****************************************************************
/* load global shader code */
void coreShader::__LoadGlobalCode()
{
    const coreSpinLocker oLocker(&s_GlobalLock);

    if(!s_asGlobalCode[0].empty()) return;

    // determine best shader version
    const coreUint16 iVersion = 100u * Core::Graphics->GetVersionGLSL()[0] + 10u * Core::Graphics->GetVersionGLSL()[1];
    const coreChar*  pcType   = (DEFINED(_CORE_GLES_) && (iVersion >= 300u)) ? "es" : "";

    // set global shader definitions
    s_asGlobalCode[0].assign(PRINT("#version %u %s"                         "\n", iVersion, pcType));
    s_asGlobalCode[1].assign(PRINT("#define CORE_NUM_TEXTURES_2D"     " (%u) \n", CORE_TEXTURE_UNITS_2D));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_TEXTURES_SHADOW" " (%u) \n", CORE_TEXTURE_UNITS_SHADOW));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_LIGHTS"          " (%u) \n", CORE_GRAPHICS_LIGHTS));
    s_asGlobalCode[1].append(PRINT("#define CORE_NUM_OUTPUTS"         " (%u) \n", CORE_SHADER_OUTPUT_COLORS));

#if defined(_CORE_EMSCRIPTEN_)

    // add WebGL environment flag (as certain features and extensions behave differently)
    s_asGlobalCode[1].append("#define _CORE_WEBGL_ (1) \n");

#endif

    // prevent instancing if not supported
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_vertex_array_object))
        s_asGlobalCode[1].append("#undef _CORE_OPTION_INSTANCING_ \n");

    const auto nRetrieveFunc = [](const coreChar* pcPath, coreString* OUTPUT pString)
    {
        // retrieve shader file
        coreFileScope pFile = Core::Manager::Resource->RetrieveFile(pcPath);
        WARN_IF(!pFile->GetData()) return;

        // copy data
        pString->append(r_cast<const coreChar*>(pFile->GetData()), pFile->GetSize());
        pString->append("\n #line 1 \n");

        // parse and adapt shader code
        coreShader::__ReduceSize     (pString);
        coreShader::__ResolveIncludes(pString, pFile);

        // reduce memory consumption
        pString->shrink_to_fit();
    };
    nRetrieveFunc("data/shaders/engine/global.glsl",                 &s_asGlobalCode[1]);
    nRetrieveFunc("data/shaders/engine/shader_vertex.glsl",          &s_asGlobalCode[2]);
    nRetrieveFunc("data/shaders/engine/shader_tess_control.glsl",    &s_asGlobalCode[3]);
    nRetrieveFunc("data/shaders/engine/shader_tess_evaluation.glsl", &s_asGlobalCode[4]);
    nRetrieveFunc("data/shaders/engine/shader_geometry.glsl",        &s_asGlobalCode[5]);
    nRetrieveFunc("data/shaders/engine/shader_fragment.glsl",        &s_asGlobalCode[6]);
    nRetrieveFunc("data/shaders/engine/shader_compute.glsl",         &s_asGlobalCode[7]);
}


// ****************************************************************
/* reduce shader code size */
void coreShader::__ReduceSize(coreString* OUTPUT psCode)
{
    if(Core::Debug->IsEnabled()) return;

    // remove code comments
    for(coreUintW i = 0u; (i = psCode->find("//", i)) != coreString::npos; )
    {
        psCode->erase(i, psCode->find_first_of('\n', i) - i);
    }

    // remove redundant whitespaces
    psCode->replace("    ", " ");
}


// ****************************************************************
/* resolve include directives */
void coreShader::__ResolveIncludes(coreString* OUTPUT psCode, const coreFile* pFile)
{
    const coreSpinLocker oLocker(&s_IncludeLock);

    constexpr coreChar  acText[] = "#include \"";
    constexpr coreUintW iTextLen = ARRAY_SIZE(acText) - 1u;

    // get base directory
    const coreChar* pcDirectory = coreData::StrDirectory(pFile->GetPath());

    // find all include directives
    for(coreUintW i = 0u; (i = psCode->find(acText, i)) != coreString::npos; )
    {
        const coreUintW      iLen  = psCode->find_first_of('\"', i + iTextLen) - i;
        const coreHashString sPath = PRINT("%s%.*s", pcDirectory, coreInt32(iLen - iTextLen), psCode->c_str() + i + iTextLen);

        if(!s_asIncludeCode.count_bs(sPath))
        {
            // retrieve shader file
            coreFileScope pIncludeFile = Core::Manager::Resource->RetrieveFile(sPath);
            WARN_IF(!pIncludeFile->GetData()) return;

            // parse and adapt shader code (recursive)
            coreString sIncludeCode(r_cast<const coreChar*>(pIncludeFile->GetData()), pIncludeFile->GetSize());
            coreShader::__ReduceSize     (&sIncludeCode);
            coreShader::__ResolveIncludes(&sIncludeCode, pIncludeFile);

            // insert include-guards
            sIncludeCode.prepend(PRINT("#ifndef x%X \n #define x%X \n", sPath.GetHash(), sPath.GetHash()));
            sIncludeCode.append ("\n #endif \n #line 1 \n");

            // reduce memory consumption
            sIncludeCode.shrink_to_fit();

            // store in container
            s_asIncludeCode.emplace_bs(sPath, std::move(sIncludeCode));
        }

        // replace include directive with shader code
        const coreString& sReplace = s_asIncludeCode.at_bs(sPath);
        psCode->replace(i, iLen + 1u, sReplace);

        i += sReplace.length();
    }
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
, m_aiBuffer       {}
, m_avCache        {}
, m_iHash          (0u)
, m_bBinary        (false)
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
    const coreStatus eCheck = m_Sync.Check(0u);
    if(eCheck == CORE_BUSY) return CORE_BUSY;

    if(m_eStatus == CORE_PROGRAM_DEFINED)
    {
        // load all required shader objects
        if(m_apShader.empty()) FOR_EACH(it, m_apShaderHandle) m_apShader.emplace_back(*it);
        FOR_EACH(it, m_apShader)
        {
            it->GetHandle()->Update();
            if(!it->GetHandle()->IsSuccessful())
            {
                m_apShader.clear();
                return CORE_INVALID_DATA;
            }
        }

        // create shader-program
        m_iIdentifier = glCreateProgram();

        // combine all shader code hash-values
        ASSERT(!m_iHash)
        FOR_EACH(it, m_apShader)
        {
            m_iHash = coreMath::HashCombine64(m_iHash, (*it)->GetHash());
        }
        FOR_EACH(it, m_aiAttribute)
        {
            m_iHash = coreMath::HashCombine64(m_iHash, coreHashXXH64(PRINT("%s %d", m_aiAttribute.get_string(it), (*it))));
        }

        // try to load shader-program binary
        m_bBinary = this->__LoadBinary();

        if(!m_bBinary)
        {
            // attach shader objects
            FOR_EACH(it, m_apShader)
            {
                if((*it)->GetIdentifier()) glAttachShader(m_iIdentifier, (*it)->GetIdentifier());
            }

            // bind default attribute locations
            glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_POSITION_NUM, CORE_SHADER_ATTRIBUTE_POSITION);
            glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_TEXCOORD_NUM, CORE_SHADER_ATTRIBUTE_TEXCOORD);
            glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_NORMAL_NUM,   CORE_SHADER_ATTRIBUTE_NORMAL);
            glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_TANGENT_NUM,  CORE_SHADER_ATTRIBUTE_TANGENT);

            // bind instancing attribute locations
            if(CORE_GL_SUPPORT(ARB_instanced_arrays) && CORE_GL_SUPPORT(ARB_vertex_array_object))
            {
                glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM,    CORE_SHADER_ATTRIBUTE_DIV_POSITION);
                glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_SIZE_NUM,        CORE_SHADER_ATTRIBUTE_DIV_SIZE);
                glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_ROTATION_NUM,    CORE_SHADER_ATTRIBUTE_DIV_ROTATION);
                glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_2D_POSITION_NUM, CORE_SHADER_ATTRIBUTE_DIV_2D_POSITION);
                glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_2D_SIZE_NUM,     CORE_SHADER_ATTRIBUTE_DIV_2D_SIZE);
                glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_2D_ROTATION_NUM, CORE_SHADER_ATTRIBUTE_DIV_2D_ROTATION);
                glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM,        CORE_SHADER_ATTRIBUTE_DIV_DATA);
                glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,       CORE_SHADER_ATTRIBUTE_DIV_COLOR);
                glBindAttribLocation(m_iIdentifier, CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM,    CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM);
            }

            // bind custom attribute locations
            FOR_EACH(it, m_aiAttribute)
            {
                if((*it) >= 0) glBindAttribLocation(m_iIdentifier, (*it), m_aiAttribute.get_string(it));
            }

            // bind output locations
            if(CORE_GL_SUPPORT(EXT_gpu_shader4))
            {
                for(coreUintW i = 0u; i < CORE_SHADER_OUTPUT_COLORS; ++i)
                {
                    glBindFragDataLocation(m_iIdentifier, i, s_asOutColor[i].GetString());
                }
            }

            // link shader-program
            glLinkProgram(m_iIdentifier);
        }

        m_eStatus = CORE_PROGRAM_LINKING;
        m_Sync.Create(CORE_SYNC_CREATE_FLUSHED);
        return CORE_BUSY;
    }
    else if(m_eStatus == CORE_PROGRAM_LINKING)
    {
        if(!m_bBinary)
        {
            GLint iStatus;

            // check for completion status
            if(CORE_GL_SUPPORT(ARB_parallel_shader_compile))
            {
                glGetProgramiv(m_iIdentifier, GL_COMPLETION_STATUS_ARB, &iStatus);
                if(!iStatus) return CORE_BUSY;
            }

            // check for link status
            glGetProgramiv(m_iIdentifier, GL_LINK_STATUS, &iStatus);
            WARN_IF(!iStatus)
            {
                Core::Log->Warning("Program (%s) could not be linked", m_sName.c_str());
                this->__WriteLog();

                m_eStatus = CORE_PROGRAM_FAILED;
                return CORE_INVALID_DATA;
            }
        }

        // set current shader-program
        glUseProgram(m_iIdentifier);
        if(!DEFINED(_CORE_DEBUG_)) s_pCurrent = NULL;

        // bind texture units
        for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS_2D;     ++i) glUniform1i(glGetUniformLocation(m_iIdentifier, s_asTexture2D    [i].GetString()), i);
        for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS_SHADOW; ++i) glUniform1i(glGetUniformLocation(m_iIdentifier, s_asTextureShadow[i].GetString()), i + CORE_TEXTURE_SHADOW);

        if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
        {
            // bind default uniform buffer objects
            const GLuint iTransform3DBlock = glGetUniformBlockIndex(m_iIdentifier, CORE_SHADER_BUFFER_TRANSFORM3D);
            const GLuint iTransform2DBlock = glGetUniformBlockIndex(m_iIdentifier, CORE_SHADER_BUFFER_TRANSFORM2D);
            const GLuint iAmbientBlock     = glGetUniformBlockIndex(m_iIdentifier, CORE_SHADER_BUFFER_AMBIENT);
            if(iTransform3DBlock != GL_INVALID_INDEX) glUniformBlockBinding(m_iIdentifier, iTransform3DBlock, CORE_SHADER_BUFFER_TRANSFORM3D_NUM);
            if(iTransform2DBlock != GL_INVALID_INDEX) glUniformBlockBinding(m_iIdentifier, iTransform2DBlock, CORE_SHADER_BUFFER_TRANSFORM2D_NUM);
            if(iAmbientBlock     != GL_INVALID_INDEX) glUniformBlockBinding(m_iIdentifier, iAmbientBlock,     CORE_SHADER_BUFFER_AMBIENT_NUM);

            // bind custom uniform buffer objects
            FOR_EACH(it, m_aiBuffer)
            {
                const GLuint iBlock = glGetUniformBlockIndex(m_iIdentifier, m_aiBuffer.get_string(it));
                if(iBlock != GL_INVALID_INDEX) glUniformBlockBinding(m_iIdentifier, iBlock, (*it));
            }
        }

        // add debug label
        Core::Graphics->LabelOpenGL(GL_PROGRAM, m_iIdentifier, m_sName.c_str());

        Core::Log->Info("Program (%s, %s) loaded", m_sName.c_str(), m_bBinary ? "cached" : "not cached");
        this->__WriteInterface();

        m_eStatus = CORE_PROGRAM_SUCCESSFUL;
        return m_Sync.Create(CORE_SYNC_CREATE_FLUSHED) ? CORE_BUSY : CORE_OK;
    }

    return eCheck;
}


// ****************************************************************
/* unload shader-program */
coreStatus coreProgram::Unload()
{
    if(!m_iIdentifier) return CORE_INVALID_CALL;

    // disable still active shader-program
    if(s_pCurrent == this) coreProgram::Disable(true);

    // save shader-program binary
    if(m_eStatus == CORE_PROGRAM_SUCCESSFUL) this->__SaveBinary();

    // disable shader objects
    m_apShader.clear();

    // delete shader-program (with implicit shader object detachment)
    glDeleteProgram(m_iIdentifier);
    if(!m_sName.empty()) Core::Log->Info("Program (%s) unloaded", m_sName.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_iIdentifier = 0u;
    m_eStatus     = CORE_PROGRAM_DEFINED;
    m_iHash       = 0u;
    m_bBinary     = false;

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
    if(s_pCurrent == this)                    return true;
    if(m_eStatus  != CORE_PROGRAM_SUCCESSFUL) return false;

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
/* execute compute shader-program */
coreStatus coreProgram::DispatchCompute(const coreUint32 iGroupsX, const coreUint32 iGroupsY, const coreUint32 iGroupsZ)const
{
    ASSERT((m_eStatus >= CORE_PROGRAM_SUCCESSFUL) && (s_pCurrent == this))

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
            if(CORE_GL_SUPPORT(CORE_es2_restriction)) glUniformMatrix2fv(iLocation, 1, false, bTranspose ? mMatrix.Transposed().ptr() : mMatrix.ptr());
                                                 else glUniformMatrix2fv(iLocation, 1, bTranspose, mMatrix.ptr());
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
        if(CORE_GL_SUPPORT(CORE_es2_restriction)) glUniformMatrix3fv(iLocation, 1, false, bTranspose ? mMatrix.Transposed().ptr() : mMatrix.ptr());
                                             else glUniformMatrix3fv(iLocation, 1, bTranspose, mMatrix.ptr());
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
        if(CORE_GL_SUPPORT(CORE_es2_restriction)) glUniformMatrix4fv(iLocation, 1, false, bTranspose ? mMatrix.Transposed().ptr() : mMatrix.ptr());
                                             else glUniformMatrix4fv(iLocation, 1, bTranspose, mMatrix.ptr());
    }
}


// ****************************************************************
/* load shader-cache from file */
coreBool coreProgram::LoadShaderCache()
{
    if(!Core::Config->GetBool(CORE_CONFIG_GRAPHICS_SHADERCACHE) || !CORE_GL_SUPPORT(ARB_get_program_binary) || DEFINED(_CORE_MACOS_) || DEFINED(_CORE_EMSCRIPTEN_) || DEFINED(_CORE_SWITCH_)) return false;

    ASSERT(s_aBinaryMap.empty() && !s_iBinarySize)

    // load and decompress file
    coreFile oFile(coreData::UserFolderShared(CORE_SHADER_CACHE_NAME));
    oFile.Decompress();

    // get file data
    const coreByte* pData   = oFile.GetData();
    const coreByte* pCursor = pData;
    if(!pData)
    {
        Core::Log->Warning("Shader cache could not be loaded");
        return false;
    }

    coreUint32 iMagic;
    coreUint32 iVersion;
    coreUint32 iTotalSize;
    coreUint32 iCheck;
    coreUint16 iNum;

    // read header values
    std::memcpy(&iMagic,     pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
    std::memcpy(&iVersion,   pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
    std::memcpy(&iTotalSize, pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
    std::memcpy(&iCheck,     pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
    std::memcpy(&iNum,       pCursor, sizeof(coreUint16)); pCursor += sizeof(coreUint16);

    // check header values
    if((iMagic != CORE_SHADER_CACHE_MAGIC) || (iVersion != CORE_SHADER_CACHE_VERSION) || (iTotalSize != oFile.GetSize()) || (iCheck != coreProgram::__GetShaderCacheCheck()))
    {
        Core::Log->Warning("Shader cache is not valid");
        return false;
    }

    s_BinaryLock.Lock();
    {
        for(coreUintW i = 0u, ie = iNum; i < ie; ++i)
        {
            coreUint64 iKey;
            coreBinary oEntry;

            // read entry values
            std::memcpy(&iKey,           pCursor, sizeof(coreUint64)); pCursor += sizeof(coreUint64);
            std::memcpy(&oEntry.iSize,   pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
            std::memcpy(&oEntry.iFormat, pCursor, sizeof(coreUint32)); pCursor += sizeof(coreUint32);

            // read shader-program binary data
            oEntry.pData = NULL; DYNAMIC_RESIZE(oEntry.pData, oEntry.iSize)
            std::memcpy(oEntry.pData, pCursor, oEntry.iSize); pCursor += oEntry.iSize;

            // add entry to map
            s_aBinaryMap.emplace_bs(iKey, oEntry);
            s_iBinarySize += oEntry.iSize;
        }
    }
    s_BinaryLock.Unlock();

    ASSERT(iTotalSize == coreUint32(pCursor - pData))

    Core::Log->Info("Shader cache loaded (%u entries, %.1f KB)", iNum, I_TO_F(s_iBinarySize) / 1024.0f);
    return true;
}


// ****************************************************************
/* save shader-cache to file */
void coreProgram::SaveShaderCache()
{
    if(!Core::Config->GetBool(CORE_CONFIG_GRAPHICS_SHADERCACHE) || !CORE_GL_SUPPORT(ARB_get_program_binary) || DEFINED(_CORE_MACOS_) || DEFINED(_CORE_EMSCRIPTEN_) || DEFINED(_CORE_SWITCH_)) return;

    s_BinaryLock.Lock();
    {
        const coreUint32 iTotalSize = 4u * sizeof(coreUint32) + sizeof(coreUint16) + s_aBinaryMap.size() * (sizeof(coreUint64) + 2u * sizeof(coreUint32)) + s_iBinarySize;

        coreByte* pData   = new coreByte[iTotalSize];
        coreByte* pCursor = pData;

        // prepare header values
        const coreUint32 iMagic   = CORE_SHADER_CACHE_MAGIC;
        const coreUint32 iVersion = CORE_SHADER_CACHE_VERSION;
        const coreUint32 iCheck   = coreProgram::__GetShaderCacheCheck();
        const coreUint16 iNum     = s_aBinaryMap.size();

        // write header values
        std::memcpy(pCursor, &iMagic,     sizeof(coreUint32)); pCursor += sizeof(coreUint32);
        std::memcpy(pCursor, &iVersion,   sizeof(coreUint32)); pCursor += sizeof(coreUint32);
        std::memcpy(pCursor, &iTotalSize, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
        std::memcpy(pCursor, &iCheck,     sizeof(coreUint32)); pCursor += sizeof(coreUint32);
        std::memcpy(pCursor, &iNum,       sizeof(coreUint16)); pCursor += sizeof(coreUint16);

        // loop through all shader-program binaries
        FOR_EACH(it, s_aBinaryMap)
        {
            const coreUint64 iKey = (*s_aBinaryMap.get_key(it));

            // write entry values
            std::memcpy(pCursor, &iKey,        sizeof(coreUint64)); pCursor += sizeof(coreUint64);
            std::memcpy(pCursor, &it->iSize,   sizeof(coreUint32)); pCursor += sizeof(coreUint32);
            std::memcpy(pCursor, &it->iFormat, sizeof(coreUint32)); pCursor += sizeof(coreUint32);
            std::memcpy(pCursor, it->pData,    it->iSize);          pCursor += it->iSize;
        }

        // unlock before writing to disk
        s_BinaryLock.Unlock();

        ASSERT(iTotalSize == coreUint32(pCursor - pData))

        // compress and save file
        coreFile oFile(coreData::UserFolderShared(CORE_SHADER_CACHE_NAME), pData, iTotalSize);
        oFile.Compress();
        oFile.Save();

        Core::Log->Info("Shader cache saved (%u entries, %.1f KB)", iNum, I_TO_F(s_iBinarySize) / 1024.0f);
    }
}


// ****************************************************************
/* remove all entries from the shader-cache */
void coreProgram::ClearShaderCache()
{
    const coreSpinLocker oLocker(&s_BinaryLock);

    // delete entries
    FOR_EACH(it, s_aBinaryMap)
    {
        DYNAMIC_DELETE(it->pData)
    }

    // clear memory
    s_aBinaryMap.clear();

    // reset properties
    s_iBinarySize = 0u;
}


// ****************************************************************
/* load shader-program binary */
coreBool coreProgram::__LoadBinary()
{
    if(!Core::Config->GetBool(CORE_CONFIG_GRAPHICS_SHADERCACHE) || !CORE_GL_SUPPORT(ARB_get_program_binary) || DEFINED(_CORE_MACOS_) || DEFINED(_CORE_EMSCRIPTEN_) || DEFINED(_CORE_SWITCH_)) return false;

    ASSERT(m_iIdentifier && m_iHash)

    // indicate the intention to retrieve the shader-program binary
    glProgramParameteri(m_iIdentifier, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, 1);

    const coreSpinLocker oLocker(&s_BinaryLock);

    if(s_aBinaryMap.count_bs(m_iHash))
    {
        GLint iStatus;

        // load existing shader-program binary
        const coreBinary& oEntry = s_aBinaryMap.at_bs(m_iHash);
        glProgramBinary(m_iIdentifier, oEntry.iFormat, oEntry.pData, oEntry.iSize);

        // check for link status (immediately)
        glGetProgramiv(m_iIdentifier, GL_LINK_STATUS, &iStatus);
        if(iStatus) return true;
    }

    return false;
}


// ****************************************************************
/* save shader-program binary */
void coreProgram::__SaveBinary()const
{
    if(!Core::Config->GetBool(CORE_CONFIG_GRAPHICS_SHADERCACHE) || !CORE_GL_SUPPORT(ARB_get_program_binary) || DEFINED(_CORE_MACOS_) || DEFINED(_CORE_EMSCRIPTEN_) || DEFINED(_CORE_SWITCH_)) return;

    ASSERT(m_iIdentifier && m_iHash)

    GLint  iSize;
    GLenum iFormat;

    // get length of shader-program binary
    glGetProgramiv(m_iIdentifier, GL_PROGRAM_BINARY_LENGTH, &iSize);

    if(iSize)
    {
        const coreSpinLocker oLocker(&s_BinaryLock);

        coreBinary& oEntry = s_aBinaryMap.bs(m_iHash);

        // retrieve shader-program binary
        if(oEntry.iSize < coreUint32(iSize)) DYNAMIC_RESIZE(oEntry.pData, iSize)
        glGetProgramBinary(m_iIdentifier, iSize, &iSize, &iFormat, oEntry.pData);

        // adjust total size
        s_iBinarySize -= oEntry.iSize;
        s_iBinarySize += iSize;

        // set properties
        oEntry.iSize   = iSize;
        oEntry.iFormat = iFormat;
    }
}


// ****************************************************************
/* write info-log to log file */
void coreProgram::__WriteLog()const
{
    Core::Log->ListStartWarning("Program Log");
    {
        GLint iLength;

        // get length of shader-program info-log
        glGetProgramiv(m_iIdentifier, GL_INFO_LOG_LENGTH, &iLength);

        if(iLength)
        {
            // get shader-program info-log
            coreChar* pcLog = new coreChar[iLength];
            glGetProgramInfoLog(m_iIdentifier, iLength, NULL, pcLog);

            // write shader-program info-log
            Core::Log->ListAdd(pcLog);
            SAFE_DELETE_ARRAY(pcLog)
        }

        // loop through all attached shader objects
        FOR_EACH(it, m_apShader)
        {
            Core::Log->ListDeeper(CORE_LOG_BOLD("%s"), (*it)->GetName());
            {
                // get length of shader info-log
                glGetShaderiv((*it)->GetIdentifier(), GL_INFO_LOG_LENGTH, &iLength);

                if(iLength)
                {
                    // get shader info-log
                    coreChar* pcLog = new coreChar[iLength];
                    glGetShaderInfoLog((*it)->GetIdentifier(), iLength, NULL, pcLog);

                    // write shader info-log
                    Core::Log->ListAdd(pcLog);
                    SAFE_DELETE_ARRAY(pcLog)
                }
            }
            Core::Log->ListEnd();
        }
    }
    Core::Log->ListEnd();
}


// ****************************************************************
/* write interface to log file */
void coreProgram::__WriteInterface()const
{
    if(!Core::Debug->IsEnabled()) return;

    Core::Log->ListStartInfo("Program Interface");
    {
        GLint iNumInput;
        GLint iNumUniform;

        // write all attached shader objects
        Core::Log->ListDeeper(CORE_LOG_BOLD("Shaders:") " %u", m_apShader.size());
        {
            FOR_EACH(it, m_apShader) Core::Log->ListAdd((*it)->GetName());
        }
        Core::Log->ListEnd();

        if(CORE_GL_SUPPORT(ARB_program_interface_query))
        {
            coreChar acName[64];
            GLint    aiValue[3];

            constexpr GLenum aiProperty[] = {GL_LOCATION, GL_BLOCK_INDEX, GL_OFFSET};

            // get number of active shader-program resources
            glGetProgramInterfaceiv(m_iIdentifier, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &iNumInput);
            glGetProgramInterfaceiv(m_iIdentifier, GL_UNIFORM,       GL_ACTIVE_RESOURCES, &iNumUniform);

            // write active vertex attributes (name, location)
            Core::Log->ListDeeper(CORE_LOG_BOLD("Attributes:") " %d", iNumInput);
            for(coreUintW i = 0u, ie = iNumInput; i < ie; ++i)
            {
                glGetProgramResourceName(m_iIdentifier, GL_PROGRAM_INPUT, i, ARRAY_SIZE(acName), NULL, acName);
                glGetProgramResourceiv  (m_iIdentifier, GL_PROGRAM_INPUT, i, 1, aiProperty, 1,   NULL, aiValue);

                Core::Log->ListAdd("%s: %d", acName, aiValue[0]);
            }
            Core::Log->ListEnd();

            // write active uniforms (name, location, block index, block offset)
            Core::Log->ListDeeper(CORE_LOG_BOLD("Uniforms:") " %d", iNumUniform);
            for(coreUintW i = 0u, ie = iNumUniform; i < ie; ++i)
            {
                glGetProgramResourceName(m_iIdentifier, GL_UNIFORM, i, ARRAY_SIZE(acName), NULL, acName);
                glGetProgramResourceiv  (m_iIdentifier, GL_UNIFORM, i, 3, aiProperty, 3,   NULL, aiValue);

                if(aiValue[0] >= 0) Core::Log->ListAdd("%s: %d",    acName, aiValue[0]);
                               else Core::Log->ListAdd("%s: %d/%d", acName, aiValue[1], aiValue[2]);
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


// ****************************************************************
/* calculate shader-cache verification value */
coreUint32 coreProgram::__GetShaderCacheCheck()
{
    return coreHashXXH32(PRINT("%s %s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION), CoreApp::Settings::Version));
}