///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


// ****************************************************************
// convert between RGB-color and HSV-color
vec3 coreRgbToHsv(const in vec3 v3Rgb)
{
    float R = v3Rgb.r;
    float G = v3Rgb.g;
    float B = v3Rgb.b;

    float v = coreMax3(R, G, B);
    float d = v - coreMin3(R, G, B);

    if(d == 0.0) return vec3(0.0, 0.0, v);

    float s = d / v;

    if(R == v) return vec3((0.0 + (G - B) / d) / 6.0, s, v);
    if(G == v) return vec3((2.0 + (B - R) / d) / 6.0, s, v);
               return vec3((4.0 + (R - G) / d) / 6.0, s, v);
}

vec3 coreHsvToRgb(const in vec3 v3Hsv)
{
    float H = v3Hsv.x * 6.0;
    float S = v3Hsv.y;
    float V = v3Hsv.z;

    float h = floor(H);

    float s = V * S;
    float t = s * (H - h);
    float p = V - s;

    if(h == 1.0) return vec3(V - t, V,     p);
    if(h == 2.0) return vec3(p,     V,     p + t);
    if(h == 3.0) return vec3(p,     V - t, V);
    if(h == 4.0) return vec3(p + t, p,     V);
    if(h == 5.0) return vec3(V,     p,     V - t);
                 return vec3(V,     p + t, p);
}


// ****************************************************************
// convert between RGB-color and YIQ-color
vec3 coreRgbToYiq(const in vec3 v3Rgb)
{
    return mat3(0.299,  0.587,  0.114,
                0.596, -0.275, -0.321,
                0.212, -0.523,  0.311) * v3Rgb;
}

vec3 coreYiqToRgb(const in vec3 v3Yiq)
{
    return mat3(1.000,  0.956,  0.620,
                1.000, -0.272, -0.647,
                1.000, -1.108,  1.705) * v3Yiq;
}


// ****************************************************************
// convert between RGB-color and YUV-color
vec3 coreRgbToYuv(const in vec3 v3Rgb)
{
    return mat3( 0.21260,  0.71520,  0.07220,
                -0.09991, -0.33609,  0.43600,
                 0.61500, -0.55861, -0.05639) * v3Rgb;
}

vec3 coreYuvToRgb(const in vec3 v3Yuv)
{
    return mat3(1.00000,  0.00000,  1.28033,
                1.00000, -0.21482, -0.38059,
                1.00000,  2.12798,  0.00000) * v3Yuv;
}


// ****************************************************************
// convert between RGB-color and YCbCr-color
vec3 coreRgbToYcbcr(const in vec3 v3Rgb)
{
    return mat3( 0.299000,  0.587000,  0.114000,
                -0.168736, -0.331264,  0.500000,
                 0.500000, -0.418688, -0.081312) * v3Rgb + vec3(0.0, 0.5, 0.5);
}

vec3 coreYcbcrToRgb(const in vec3 v3Ycbcr)
{
    return mat3(1.00000,  0.00000,  1.40200,
                1.00000, -0.34414, -0.71414,
                1.00000,  1.77200,  0.00000) * (v3Ycbcr - vec3(0.0, 0.5, 0.5));
}


// ****************************************************************
// convert to luminance value
float coreLuminance(const in vec3 v3Rgb)
{
    return dot(v3Rgb, vec3(0.2126, 0.7152, 0.0722));
}