
// ****************************************************************
# Shader Interface #

*varying* and *attribute* instead of *in* and *out* in vertex and fragment shader for max compatibility
qualifiers *highp*, *mediump*, *lowp*, *flat*, *noperspective* and *smooth* are safe to use
shader main functions are named *VertexMain()* and *FragmentMain()*


// ****************************************************************
# Vertex Shader #


## Functions ##

// default model transformation (position, size, rotation, without camera and projection)
vec3 coreObject3DTransform(in vec3)
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

// transform lighting properties into tangent-space (default, vertex shader only)
void coreLightingTransform(in vec3 v3Position)


## In ##

// object3D attributes
vec3 a_v3RawPosition     // raw vertex position
vec2 a_v2RawTexCoord     // raw texture coordinate
vec3 a_v3RawNormal       // normal vector 
vec4 a_v4RawTangent      // tangent vector (xyz = tangent, w = binormal sign)
vec3 u_v3Position        // position offset
vec3 u_v3Size            // non-uniform size factor
vec4 u_v4Rotation        // rotation quaternion (can also be used for normal rotation)
vec2 u_v2TexSize         // texture size
vec2 u_v2TexOffset       // texture offset
vec2 (a_v2LowPosition)   // raw vertex position    (only with low-memory model, recommended)
vec2 (a_v2LowTexCoord)   // raw texture coordinate (only with low-memory model, recommended)

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


## Out ##

vec4 v_v4VarColor                          // custom color value forward (don't use together with u_v4Color in fragment shader)
vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D]   // predefined texture coordinate forward (to next shader stage)
vec4 v_av4LightPos[CORE_NUM_LIGHTS]        // tangent-space light position forward
vec4 v_av4LightDir[CORE_NUM_LIGHTS]        // tangent-space light direction forward
vec3 v_v3TangentPos                        // tangent-space vertex position forward (used for view direction)
vec3 v_v3TangentCam                        // tangent-space camera position forward (used for view direction)


// ****************************************************************
# Fragment Shader #


## In ##

vec4 v_v4VarColor
vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D]
vec4 v_av4LightPos[CORE_NUM_LIGHTS]
vec4 v_av4LightDir[CORE_NUM_LIGHTS]
vec3 v_v3TangentPos
vec3 v_v3TangentCam

vec3 v_v3ViewDir       // pre-calculated view direction (not normalized)

// not with particles or on instancing
vec3 (u_v3Position)    // position offset
vec3 (u_v3Size)        // non-uniform size factor
vec4 (u_v4Rotation)    // rotation quaternion (can also be used for normal rotation)
vec2 (u_v2TexSize)     // texture size
vec2 (u_v2TexOffset)   // texture offset


// ****************************************************************
# General #


## Functions ##

bool  coreAnyInvocation       (in bool)            // condition across group of shader invocations
bool  coreAllInvocations      (in bool)
T     coreMin3                (in T, in T, in T)   // trinary minimum
T     coreMax3                (in T, in T, in T)   // trinary maximum
T     coreMed3                (in T, in T, in T)   // trinary median
int   coreMod                 (in int, in int)     // modulo operator
vec3  coreHSVtoRGB            (in vec3)            // HSV to RGB conversion
vec3  coreRGBtoHSV            (in vec3)            // RGB to HSV conversion
vec3  coreYIQtoRGB            (in vec3)            // YIQ to RGB conversion
vec3  coreRGBtoYIQ            (in vec3)            // RGB to YIQ conversion
float coreLengthSq            (in vec2)            // vector square length
float coreLengthSq            (in vec3)
vec2  corePackNormalSphere    (in vec3)            // vector normal pack   (with Lambert azimuthal equal-area projection)
vec3  coreUnpackNormalSphere  (in vec2)            // vector normal unpack (with Lambert azimuthal equal-area projection)
vec3  coreUnpackNormalMap     (in vec2)            // vector normal unpack (with z-reconstruction)
vec3  coreUnpackNormalMapDeriv(in vec2)            // vector normal unpack (with partial-derivative)
vec3  coreQuatApply           (in vec4, in vec3)   // quaternion transformation
mat3  coreTranspose           (in mat3)            // matrix transpose
mat4  coreTranspose           (in mat4)
mat3  coreInvert              (in mat3)            // matrix invert
mat4  coreInvert              (in mat4)
mat3  coreMat4to3             (in mat4)            // matrix convert
mat2  coreMat3to2             (in mat3)
uint  corePackUnorm4x8        (in vec4)            // value pack   (4x 8bit float -> 1x 32bit uint)
vec4  coreUnpackUnorm4x8      (in uint)            // value unpack (1x 32bit uint -> 4x 8bit float)

vec4  coreTexture2D    (in int v1Unit, in vec2 v2TexCoord)    // normal texture lookup
float coreTextureShadow(in int v1Unit, in vec4 v4ProjCoord)   // PCF depth-compare for shadow textures

#define PI (3.1415926535897932384626433832795)
#define EU (2.7182818284590452353602874713527)


## In ##

mat4 u_m4ViewProj      // view-projection matrix (u_m4Perspective * u_m4Camera)
mat4 u_m4Camera        // camera matrix
mat4 u_m4Perspective   // perspective projection matrix
mat4 u_m4Ortho         // orthographic projection matrix
vec4 u_v4Resolution    // current viewport resolution (xy = normal, zw = reciprocal)
vec3 u_v3CamPosition   // camera position

vec4 u_v4Color         // color value

struct
{
    vec4 v4Position
    vec4 v4Direction
    vec4 v4Value
} u_aLight[CORE_NUM_LIGHTS]   // light data directly from the application
