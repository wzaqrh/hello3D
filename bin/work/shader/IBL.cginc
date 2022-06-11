#ifndef IBL_H
#define IBL_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "ToneMapping.cginc"
#include "CommonFunction.cginc"

float3 GetEnvDiffuseIrradiance(float3 n)
{
#if RIGHT_HANDNESS_RESOURCE
    n.z = -n.z;
#endif  
	return MIR_SAMPLE_TEXCUBE(_EnvDiffuseMap, n).rgb;
}

float3 GetEnvSpecularIrradiance(float3 reflUVW, float perceptualRoughness)
{
#if RIGHT_HANDNESS_RESOURCE
	reflUVW.z = -reflUVW.z;
#endif
    float lod = perceptualRoughness * (EnvSpecColorMip.w - 1.0);
    return MIR_SAMPLE_TEXCUBE_LOD(_EnvSpecMap, reflUVW, lod).rgb;
}

float3 GetEnvSheenIrradiance(float3 reflUVW, float sheenPerceptualRoughness)
{
    float lod = sheenPerceptualRoughness * float(EnvSheenColorMip.w - 1);
#if RIGHT_HANDNESS_RESOURCE
	reflUVW.z = -reflUVW.z;
#endif
    return MIR_SAMPLE_TEXCUBE_LOD(_EnvSheenMap, reflUVW, lod).rgb;
}

struct IBLInput 
{
    float3 FssEss;
    float3 reflUVW;
};
inline IBLInput GetIBLInput(float3 n, float3 v, float nv, float perceptualRoughness, float2 f_ab, float3 f0, float specularWeight)
{
    IBLInput ibl_i;
    float3 smoothness = 1.0 - perceptualRoughness;
    float3 Fr = max(smoothness, f0) - f0;
    float3 k_S = f0 + Fr * Pow5(1.0 - nv);
    ibl_i.FssEss = specularWeight * k_S * f_ab.x + f_ab.y; // <--- GGX / specular light contribution (scale it down if the specularWeight is low)
    ibl_i.reflUVW = normalize(reflect(-v, n));
    return ibl_i;
}

//https://bruop.github.io/ibl/#single_scattering_results from Fdez-Aguera
float3 GetIBLRadianceLambertian(IBLInput i, float3 n, float2 f_ab, float3 f0, float3 diffuseColor, float specularWeight)
{
    float3 irradiance = GetEnvDiffuseIrradiance(n);
#if DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE_PREFILTER_ENV
	return irradiance;
#endif

    // Multiple scattering, from Fdez-Aguera
    float Ems = (1.0 - (f_ab.x + f_ab.y));
    float3 F_avg = specularWeight * (f0 + (1.0 - f0) / 21.0);
    float3 FmsEms = Ems * i.FssEss * F_avg / (1.0 - F_avg * Ems);
    
    float3 k_D = diffuseColor * (1.0 - i.FssEss + FmsEms); // we use +FmsEms as indicated by the formula in the blog post (might be a typo in the implementation)

	return (FmsEms + k_D) * irradiance;
}

float3 GetIBLRadianceGGX(IBLInput i, float perceptualRoughness, float specularWeight)
{
    float3 irradiance = GetEnvSpecularIrradiance(i.reflUVW, perceptualRoughness);
    
    float3 fcolor = specularWeight * irradiance * i.FssEss;
    
#if DEBUG_CHANNEL == DEBUG_CHANNEL_MIP_LEVEL
    float mip = (EnvSpecColorMip.w - 1.0);
	fcolor = float3(mip / 32.0, perceptualRoughness, mip * perceptualRoughness);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV_UV
    float3 reflUVW = i.reflUVW;
    #if RIGHT_HANDNESS_RESOURCE
	    reflUVW.z = -reflUVW.z;
    #endif	
	fcolor = reflUVW * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV
	fcolor = specularLight;
#endif
	return fcolor;
}

float3 GetIBLRadianceCharlie(IBLInput i, float nv, float4 sheenColorRoughness)
{
    float2 lut_uv = saturate(float2(nv, sheenColorRoughness.w));
    float brdf = MIR_SAMPLE_TEX2D(_LUT, lut_uv).b;

    float3 irradiance = GetEnvSheenIrradiance(i.reflUVW, sheenColorRoughness.w);
    float3 fcolor = sheenColorRoughness.rgb * irradiance * brdf;
#if DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_IBL_LUT
    fcolor = brdf;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_IBL_LUT_UV
    fcolor = float3(lut_uv, 0.0);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_IBL_LOD
    fcolor = float3(sheenColorRoughness * float(EnvSheenColorMip.w - 1), sheenColorRoughness, float(EnvSheenColorMip.w - 1));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_IBL_IRRADIANCE
    fcolor = irradiance;
#endif
    return fcolor;
}

#endif