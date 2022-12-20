-------------------------------------------------------------------
Shader Interface

main function names:
- VertexMain
- TessControlMain
- TessEvaluationMain
- GeometryMain
- FragmentMain
- ComputeMain

preferred qualifiers:
- varying/attribute (instead of in/out, for vertex and fragment shader)

safe qualifiers (ignored, if not supported):
- highp, mediump, lowp
- flat, noperspective, smooth
- centroid, sample
- precise (+ fma)
- invariant


-------------------------------------------------------------------
Vertex Shader

Functions

    // default model transformation (position, size, rotation, without camera and projection)
    vec3 coreObject3DTransform(in vec3 v3Vector)
    vec3 coreObject3DTransformRaw()
    vec3 coreObject3DTransformLow()

    // default particle range calculation (transformed rectangle corner incl. camera alignment)
    vec3 coreParticleRange()

    // default position transformation (with camera and projection)
    vec4 coreObject3DPositionRaw()
    vec4 coreObject3DPositionLow()
    vec4 coreObject2DPosition()
    vec4 coreParticlePosition()

    // default texture coordinate transformation
    vec2 coreObject3DTexCoordRaw()
    vec2 coreObject3DTexCoordLow()
    vec2 coreObject2DTexCoord()
    vec2 coreParticleTexCoord()

    // calculate tangent-space matrix (with rotation quaternion)
    mat3 coreTangentSpaceMatrix(in vec4 v4Rotation, in vec3 v3Normal, in vec4 v4Tangent)
    mat3 coreTangentSpaceMatrix()

    // transform lighting properties into tangent-space
    void coreLightingTransform(in vec3 v3Position)
    void coreLightingTransformRaw()
    void coreLightingTransformLow()

Input

    // object3D attributes
    vec3 a_v3RawPosition     // raw vertex position
    vec2 a_v2RawTexCoord     // raw texture coordinate
    vec3 a_v3RawNormal       // normal vector
    vec4 a_v4RawTangent      // tangent vector (xyz = tangent, w = binormal sign)
    vec2 a_v2LowPosition     // raw vertex position    (only with low-memory model, recommended)
    vec2 a_v2LowTexCoord     // raw texture coordinate (only with low-memory model, recommended)
    vec3 u_v3Position        // position offset
    vec3 u_v3Size            // non-uniform size factor
    vec4 u_v4Rotation        // rotation quaternion (can also be used for normal rotation)
    vec2 u_v2TexSize         // texture size
    vec2 u_v2TexOffset       // texture offset

    // object2D attributes
    vec2 a_v2LowPosition     // raw vertex position
    vec2 a_v2LowTexCoord     // raw texture coordinate
    mat3 u_m3ScreenView      // full screen-view matrix
    vec2 u_v2TexSize         // texture size
    vec2 u_v2TexOffset       // texture offset

    // particle attributes
    vec2  a_v2LowPosition    // raw vertex position
    vec2  a_v2LowTexCoord    // raw texture coordinate
    float a_v1DivScale       // uniform size factor
    float a_v1DivAngle       // rotation angle
    float a_v1DivValue       // animation value (from 1.0 to 0.0)

Output

    vec4 v_v4VarColor                          // custom color value forward (don't use together with u_v4Color in fragment shader)
    vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D]   // predefined texture coordinate forward (to next shader stage)
    vec4 v_av4LightPos[CORE_NUM_LIGHTS]        // tangent-space light position forward
    vec4 v_av4LightDir[CORE_NUM_LIGHTS]        // tangent-space light direction forward
    vec3 v_v3TangentPos                        // tangent-space vertex position forward (used for view direction)
    vec3 v_v3TangentCam                        // tangent-space camera position forward (used for view direction)


-------------------------------------------------------------------
Fragment Shader

Definitions

    // evaluate shader per sample (just add into FragmentMain)
    #define CORE_SAMPLE_SHADING

Functions

    float coreGGX   (in float v1Dot, in float v1Rough)   // GGX specular function
    float coreDither(in ivec2 i2PixelCoord)              // ordered dithering function (modified)
    float coreDither()

Input

    vec4 v_v4VarColor
    vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D]
    vec4 v_av4LightPos[CORE_NUM_LIGHTS]
    vec4 v_av4LightDir[CORE_NUM_LIGHTS]
    vec3 v_v3TangentPos
    vec3 v_v3TangentCam

    // pre-calculated view direction (not normalized)
    vec3 v_v3ViewDir

Output

    // multiple output colors (only with GL >= 130, GLES >= 300)
    vec4 o_av4OutColor[CORE_NUM_OUTPUTS]


-------------------------------------------------------------------
General

Definitions

    uint, uvec2, uvec3, uvec4     // unsigned integer types (signed, if not supported)
    half, hvec2, hvec3, hvec4     // 16-bit floating point types (32-bit, if not supported)

    #define PI                    // Archimedes' constant
    #define EU                    // Euler's number
    #define GR                    // golden ratio ((a+b)/a = a/b)
    #define GA                    // golden angle (radians, PI * (3 - SQRT(5)))
    #define SQRT2                 // principal square root of 2
    #define SQRT3                 // principal square root of 3

    // version wrappers
    #define CORE_GL_VERSION
    #define CORE_GL_ES_VERSION

    // extension wrappers
    #define CORE_GL_conservative_depth
    #define CORE_GL_draw_buffers
    #define CORE_GL_gpu_shader4
    #define CORE_GL_sample_shading
    #define CORE_GL_shader_image_load_store
    #define CORE_GL_standard_derivatives

Functions

    T     coreMin3                (in T, in T, in T)                  // trinary minimum
    T     coreMax3                (in T, in T, in T)                  // trinary maximum
    T     coreMed3                (in T, in T, in T)                  // trinary median
    bool  coreAnyInvocation       (in bool)                           // condition across group of shader invocations
    bool  coreAllInvocations      (in bool)
    T     coreSaturate            (in T)                              // clamp values between 0.0 and 1.0
    T     coreLinearStep          (in T, in T, in T)                  // linear interpolation between 0.0 and 1.0
    T     coreSign                (in T)                              // extract the sign without returning 0.0
    S     coreIsNan               (in T)                              // test if the parameter is not a number
    T     coreIntMod              (in T, in S)                        // integer modulo operation
    vec3  coreRgbToHsv            (in vec3)                           // RGB to HSV conversion
    vec3  coreHsvToRgb            (in vec3)                           // HSV to RGB conversion
    vec3  coreRgbToYiq            (in vec3)                           // RGB to YIQ conversion   (BT.601, NTSC)
    vec3  coreYiqToRgb            (in vec3)                           // YIQ to RGB conversion   (BT.601, NTSC)
    vec3  coreRgbToYuv            (in vec3)                           // RGB to YUV conversion   (BT.709)
    vec3  coreYuvToRgb            (in vec3)                           // YUV to RGB conversion   (BT.709)
    vec3  coreRgbToYcbcr          (in vec3)                           // RGB to YCbCr conversion (BT.601, JPEG)
    vec3  coreYcbcrToRgb          (in vec3)                           // YCbCr to RGB conversion (BT.601, JPEG)
    float coreLuminance           (in vec3)                           // luminance value         (BT.709)
    float coreLengthSq            (in vec2)                           // vector square length
    float coreLengthSq            (in vec3)
    vec2  corePackNormalSphere    (in vec3)                           // vector normal pack   (with Lambert azimuthal equal-area projection)
    vec3  coreUnpackNormalSphere  (in vec2)                           // vector normal unpack (with Lambert azimuthal equal-area projection)
    vec3  coreUnpackNormalMap     (in vec2)                           // vector normal unpack (with z-reconstruction)
    vec3  coreUnpackNormalMapDeriv(in vec2)                           // vector normal unpack (with partial-derivative)
    vec2  corePackNormalOcta      (in vec3)                           // vector normal pack   (with octahedron encoding)
    vec3  coreUnpackNormalOcta    (in vec2)                           // vector normal unpack (with octahedron encoding)
    vec3  coreQuatApply           (in vec4, in vec3)                  // quaternion transformation
    mat2  coreTranspose           (in mat2)                           // matrix transpose
    mat3  coreTranspose           (in mat3)
    mat4  coreTranspose           (in mat4)
    mat3  coreInvert              (in mat3)                           // matrix invert
    mat4  coreInvert              (in mat4)
    mat3  coreMat4to3             (in mat4)                           // matrix convert
    mat2  coreMat3to2             (in mat3)
    mat2  coreMat4to2             (in mat4)
    uint  corePackUnorm4x8        (in vec4)                           // value pack   (4x 8bit float -> 1x 32bit uint)
    vec4  coreUnpackUnorm4x8      (in uint)                           // value unpack (1x 32bit uint -> 4x 8bit float)

    vec4  coreTexture2D        (in int v1Unit, in vec2 v2TexCoord)    // normal texture lookup
    vec4  coreTextureProj      (in int v1Unit, in vec4 v4ProjCoord)   // normal texture lookup (with projection)
    float coreTextureShadow    (in int v1Unit, in vec4 v4ProjCoord)   // PCF depth-compare for shadow textures
    vec4  coreTextureBase2D    (in int v1Unit, in vec2 v2TexCoord)    // only use base-level (if possible)
    vec4  coreTextureBaseProj  (in int v1Unit, in vec4 v4ProjCoord)
    float coreTextureBaseShadow(in int v1Unit, in vec4 v4ProjCoord)

Input

    mat4 u_m4ViewProj      // view-projection matrix (u_m4Perspective * u_m4Camera)
    mat4 u_m4Camera        // camera matrix
    mat4 u_m4Perspective   // perspective projection matrix
    mat4 u_m4Ortho         // orthographic projection matrix
    vec4 u_v4Resolution    // current viewport resolution (xy = normal, zw = reciprocal)
    vec3 u_v3CamPosition   // camera position

    vec4 u_v4Color         // color value

    // base light data
    vec4 u_av4LightPos  [CORE_NUM_LIGHTS]
    vec4 u_av4LightDir  [CORE_NUM_LIGHTS]
    vec4 u_av4LightValue[CORE_NUM_LIGHTS]