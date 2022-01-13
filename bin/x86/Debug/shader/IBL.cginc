#ifndef IBL_H
#define IBL_H
#include "Standard.cginc"
#include "ToneMapping.cginc"
#include "Debug.cginc"

float clampedDot(float3 x, float3 y)
{
    return saturate(dot(x, y));
}

float3 GetDiffuseLight(float3 normal)
{
#if CubeMapIsRightHandness
    normal.z = -normal.z;
#endif  
	return MIR_SAMPLE_TEXCUBE(_DiffuseCube, normal).rgb;
}

float3 GetSpecularLight(float3 normal, float3 toEye, float lod)
{
    float3 reflUVW = normalize(reflect(-toEye, normal));
#if CubeMapIsRightHandness
	reflUVW.z = -reflUVW.z;
#endif	
    return MIR_SAMPLE_TEXCUBE_LOD(_SpecCube, reflUVW, lod).rgb;
}

//https://bruop.github.io/ibl/#single_scattering_results from Fdez-Aguera
float3 GetIBLRadianceLambertian(float3 normal, float3 toEye, float perceptualRoughness, float3 diffuseColor, float3 F0, float specularWeight)
{
    float NdotV = clampedDot(normal, toEye);
    float2 f_ab = MIR_SAMPLE_TEX2D(_LUT, saturate(float2(NdotV, perceptualRoughness))).rg;

    float3 irradiance = GetDiffuseLight(normal);

    float smoothness = 1.0 - perceptualRoughness;
    float3 Fr = max(float3(smoothness,smoothness,smoothness), F0) - F0;
    float3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);
    float3 FssEss = specularWeight * k_S * f_ab.x + f_ab.y; // <--- GGX / specular light contribution (scale it down if the specularWeight is low)

    // Multiple scattering, from Fdez-Aguera
    float Ems = (1.0 - (f_ab.x + f_ab.y));
    float3 F_avg = specularWeight * (F0 + (1.0 - F0) / 21.0);
    float3 FmsEms = Ems * FssEss * F_avg / (1.0 - F_avg * Ems);
    float3 k_D = diffuseColor * (1.0 - FssEss + FmsEms); // we use +FmsEms as indicated by the formula in the blog post (might be a typo in the implementation)

    float3 fcolor = (FmsEms + k_D) * irradiance;
#if DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE_PREFILTER_ENV
	fcolor = irradiance;
#endif
	return fcolor;
}

float3 GetIBLRadianceGGX(float3 normal, float3 toEye, float perceptualRoughness, float3 F0, float specularWeight)
{
    float NdotV = clampedDot(normal, toEye);
	float mip = 4;//float(u_MipCount - 1);
    float lod = saturate(perceptualRoughness) * mip;

	float2 lut_uv = saturate(float2(NdotV, perceptualRoughness));	
    float2 f_ab = MIR_SAMPLE_TEX2D(_LUT, lut_uv).rg;
    float3 specularLight = GetSpecularLight(normal, toEye, lod);

    float smoothness = 1.0 - perceptualRoughness;
    float3 Fr = max(float3(smoothness, smoothness, smoothness), F0) - F0;
    float3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);
    float3 FssEss = k_S * f_ab.x + f_ab.y;

    float3 fcolor = specularWeight * specularLight * FssEss;
#if DEBUG_CHANNEL == DEBUG_CHANNEL_MIP_LEVEL
	fcolor = float3(mip / 32.0, perceptualRoughness, mip * perceptualRoughness);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV_UV
    float3 reflUVW = normalize(reflect(-toEye, normal));
#if CubeMapIsRightHandness
	reflUVW.z = -reflUVW.z;
#endif	
	fcolor = reflUVW;
	fcolor = (fcolor + 1.0) / 2.0;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV
	fcolor = GetSpecularLight(normal, toEye, lod);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_LUT
	fcolor = float3(f_ab, 0.0);
#endif
	return fcolor;
}

#endif