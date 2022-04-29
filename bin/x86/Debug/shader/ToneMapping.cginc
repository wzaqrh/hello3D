#ifndef TONE_MAPPING_H
#define TONE_MAPPING_H
#include "Standard.cginc"
#include "Macros.cginc"

#define GAMMA 2.2
#define INV_GAMMA (1.0 / GAMMA)

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const static float3x3 ACESInputMat = float3x3
(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
);


// ODT_SAT => XYZ => D60_2_D65 => sRGB
const static float3x3 ACESOutputMat = float3x3
(
    1.60475, -0.10208, -0.00327,
    -0.53108,  1.10813, -0.07276,
    -0.07367, -0.00605,  1.07602
);

inline float3 linearTosRGB(float3 color)
{
    return pow(color, float3(INV_GAMMA, INV_GAMMA, INV_GAMMA));
}

inline float3 sRGBToLinear(float3 srgbIn)
{
    return float3(pow(srgbIn.xyz, float3(GAMMA, GAMMA, GAMMA)));
}
inline float4 sRGBToLinear(float4 srgbIn)
{
    return float4(sRGBToLinear(srgbIn.xyz), srgbIn.w);
}

// ACES tone map (faster approximation)
// see: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
inline float3 toneMapACES_Narkowicz(float3 color)
{
    const float A = 2.51;
    const float B = 0.03;
    const float C = 2.43;
    const float D = 0.59;
    const float E = 0.14;
    return saturate((color * (A * color + B)) / (color * (C * color + D) + E));
}

// ACES filmic tone map approximation
// see https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
inline float3 RRTAndODTFit(float3 color)
{
    float3 a = color * (color + 0.0245786) - 0.000090537;
    float3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
    return a / b;
}

// tone mapping 
inline float3 toneMapACES_Hill(float3 color)
{
    color = mul(color, ACESInputMat);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(color, ACESOutputMat);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

float3 toneMap(float3 color)
{
    color *= Exposure;

#if TONEMAP_MODE == TONEMAP_ACES_NARKOWICZ
    color = toneMapACES_Narkowicz(color);
#elif TONEMAP_MODE == TONEMAP_ACES_HILL
    color = toneMapACES_Hill(color);
#elif TONEMAP_MODE == TONEMAP_ACES_HILL_EXPOSURE_BOOST
    // boost exposure as discussed in https://github.com/mrdoob/three.js/pull/19621
    // this factor is based on the exposure correction of Krzysztof Narkowicz in his
    // implemetation of ACES tone mapping
    color /= 0.6;
    color = toneMapACES_Hill(color);
#endif

    return linearTosRGB(color);
}

#endif