#ifndef TONE_MAPPING_H
#define TONE_MAPPING_H

#define GAMMA 2.2
#define INV_GAMMA (1.0 / GAMMA)

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

#endif