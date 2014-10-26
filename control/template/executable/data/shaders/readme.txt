
# Shader Interface #

*varying* and *attribute* instead of *in* and *out* in vertex and fragment shader for max compatibility
qualifiers *highp*, *mediump*, *lowp*, *flat*, *noperspective* and *smooth* are safe to use
shader main functions are named VertexMain and FragmentMain


# Vertex Shader #

## Functions
vec3 coreObject3DTransform(in vec3)   // default model transformation (position, size, rotation, without camera or projection)
vec3 coreObject3DTransform()
vec3 coreParticleRange()              // default particle range calculation (model boundaries incl. camera alignment)

vec4 coreObject3DPosition()           // default position transformation (with camera and projection)
vec4 coreObject2DPosition()
vec4 coreParticlePosition()

vec2 coreObject3DTexCoord()           // default texture coordinate transformation
vec2 coreObject2DTexCoord()
vec2 coreParticleTexCoord()

vec4 coreObject3DColor()              // recommended color value retrieval
vec4 coreObject2DColor()
vec4 coreParticleColor()

void coreDot3VertexInit(in vec4 v4Rotation, in vec3 v3Normal, in vec4 v4Tangent)   // init dot-3 bump mapping (with quaternion)
void coreDot3VertexInit()
vec3 coreDot3VertexTransform(in vec3)                                              // dot-3 vector transformation

## In
vec3 a_v3RawPosition     // raw vertex position
vec2 a_v2RawTexCoord     // raw texture coordinate (UV)
vec3 a_v3RawNormal       // raw normal vector
vec4 a_v4RawTangent      // raw tangent vector (xyz = tangent, w = binormal sign)

vec2  u_v2TexSize     // texture size (object3D/2D only)
vec2  u_v2TexOffset   // texture offset (object3D/2D only)
float a_v1DivScale    // uniform size factor (particle only)
float a_v1DivAngle    // rotation angle (particle only)
float a_v1DivValue    // animation value (particle only, from 1.0 to 0.0)

## Out
vec4 v_v4VarColor                          // custom color value forward (don't use together with u_v4Color in Fragment Shader)
vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D]   // predefined texture coordinate forward
vec4 v_av4LightDir[CORE_NUM_LIGHTS]        // predefined light direction forward
vec3 v_v3ViewDir                           // predefined view direction forward


# Fragment Shader #

## In
vec4 v_v4VarColor
vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D]
vec4 v_av4LightDir[CORE_NUM_LIGHTS]
vec3 v_v3ViewDir

vec4 u_v4Color       // color value
vec2 u_v2TexSize     // texture size (object3D/2D only, not on instancing)
vec2 u_v2TexOffset   // texture offset (object3D/2D only, not on instancing)


# General #

## Functions
T     coreMin3          (in T, in T, in T)   // trinary minimum
T     coreMax3          (in T, in T, in T)   // trinary maximum
vec3  coreHSVtoRGB      (in vec3)            // HSV to RGB conversion
vec3  coreRGBtoHSV      (in vec3)            // RGB to HSV conversion
float coreLengthSq      (in vec2)            // vector square length
float coreLengthSq      (in vec3)
vec3  coreQuatApply     (in vec4, in vec3)   // quaternion transformation
mat3  coreTranspose     (in mat3)            // matrix transpose
mat4  coreTranspose     (in mat4)
mat3  coreInvert        (in mat3)            // matrix invert
mat4  coreInvert        (in mat4)
uint  corePackUnorm4x8  (in vec4)            // value pack (4x 8bit float -> 1x 32bit uint)
vec4  coreUnpackUnorm4x8(in uint)            // value unpack (1x 32bit uint -> 4x 8bit float)

vec4  coreTexture2D    (in int v1Unit, in vec2 v2TexCoord)    // normal texture lookup
float coreTextureShadow(in int v1Unit, in vec4 v4ProjCoord)   // PCF depth-compare for shadow textures

definitions available for PI (Archimedes' constant) and EU (Euler's number)

## In
mat4 u_m4ViewProj      // view-projection matrix (u_m4Perspective * u_m4Camera)
mat4 u_m4Camera        // camera matrix
mat4 u_m4Perspective   // perspective projection matrix
mat4 u_m4Ortho         // orthographic projection matrix
vec4 u_v4Resolution    // current viewport resolution (xy = normal, zw = reciprocal)

struct
{
    vec4 v4Position
    vec4 v4Direction
    vec4 v4Value
} u_aLight[CORE_NUM_LIGHTS]   // raw light data from the application

vec3 u_v3Position;            // position offset(object3D only)
vec3 u_v3Size;                // non-uniform size factor (object3D only)
vec4 u_v4Rotation;            // rotation quaternion (object3D only, can also be used for normal rotation)
mat3 u_m3ScreenView           // full screen-view matrix (object2D only)
