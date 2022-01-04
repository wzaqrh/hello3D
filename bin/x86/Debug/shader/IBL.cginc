#ifndef IBL_H
#define IBL_H
#include "Standard.cginc"

float clampedDot(float3 x, float3 y)
{
    return saturate(dot(x, y));
}

float3 GetDiffuseLight(float3 normal, float3 toEye)
{
    float3 reflUVW = normalize(reflect(-toEye, normal));
    return MIR_SAMPLE_TEXCUBE(_DiffuseCube, normal).rgb;
}

float3 GetSpecularLight(float3 normal, float3 toEye, float lod)
{
    float3 reflUVW = normalize(reflect(-toEye, normal));
    return MIR_SAMPLE_TEXCUBE_LOD(_SpecCube, reflUVW, lod).rgb;
}

//https://bruop.github.io/ibl/#single_scattering_results from Fdez-Aguera
float3 GetIBLRadianceLambertian(float3 normal, float3 toEye, float perceptualRoughness, float3 diffuseColor, float3 F0, float specularWeight)
{
    float NdotV = clampedDot(normal, toEye);
    float2 f_ab = MIR_SAMPLE_TEX2D(_LUT, saturate(float2(NdotV, perceptualRoughness))).rg;

    float3 irradiance = GetDiffuseLight(normal, toEye);

    float smoothness = 1.0 - perceptualRoughness;
    float3 Fr = max(float3(smoothness,smoothness,smoothness), F0) - F0;
    float3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);
    float3 FssEss = specularWeight * k_S * f_ab.x + f_ab.y; // <--- GGX / specular light contribution (scale it down if the specularWeight is low)

    // Multiple scattering, from Fdez-Aguera
    float Ems = (1.0 - (f_ab.x + f_ab.y));
    float3 F_avg = specularWeight * (F0 + (1.0 - F0) / 21.0);
    float3 FmsEms = Ems * FssEss * F_avg / (1.0 - F_avg * Ems);
    float3 k_D = diffuseColor * (1.0 - FssEss + FmsEms); // we use +FmsEms as indicated by the formula in the blog post (might be a typo in the implementation)

    return (FmsEms + k_D) * irradiance;
}

float3 GetIBLRadianceGGX(float3 normal, float3 toEye, float perceptualRoughness, float3 F0, float specularWeight)
{
    float NdotV = clampedDot(normal, toEye);
    float lod = perceptualRoughness * 1;//float(u_MipCount - 1);

    float2 f_ab = MIR_SAMPLE_TEX2D(_LUT, saturate(float2(NdotV, perceptualRoughness))).rg;
    float3 specularLight = GetSpecularLight(normal, toEye, lod);

    float smoothness = 1.0 - perceptualRoughness;
    float3 Fr = max(float3(smoothness,smoothness,smoothness), F0) - F0;
    float3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);
    float3 FssEss = k_S * f_ab.x + f_ab.y;

    return specularWeight * specularLight * FssEss;
}

#endif