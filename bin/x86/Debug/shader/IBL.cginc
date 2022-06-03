#ifndef IBL_H
#define IBL_H
#include "Standard.cginc"
#include "ToneMapping.cginc"
#include "Macros.cginc"

float3 GetEnvDiffuseColor(float3 n)
{
#if RIGHT_HANDNESS_RESOURCE
    n.z = -n.z;
#endif  
	return MIR_SAMPLE_TEXCUBE(_DiffuseCube, n).rgb;
}

float3 GetEnvSpecularColor(float3 n, float3 v, float perceptualRoughness)
{
    float3 reflUVW = normalize(reflect(-v, n));
#if RIGHT_HANDNESS_RESOURCE
	reflUVW.z = -reflUVW.z;
#endif	
    float lod = perceptualRoughness * (EnvSpecColorMip.z - 1);
    return MIR_SAMPLE_TEXCUBE_LOD(_SpecCube, reflUVW, lod).rgb;
}

struct IBLInput 
{
    float3 FssEss;
    float specularWeight;
};
inline IBLInput GetIBLInput(float perceptualRoughness, float nv, float2 f_ab, float3 f0, float specularWeight)
{
    IBLInput ibl_i;
    float3 smoothness = 1.0 - perceptualRoughness;
    float3 Fr = max(smoothness, f0) - f0;
    float3 k_S = f0 + Fr * pow(1.0 - nv, 5.0);
    ibl_i.FssEss = specularWeight * k_S * f_ab.x + f_ab.y; // <--- GGX / specular light contribution (scale it down if the specularWeight is low)
    ibl_i.specularWeight = specularWeight;
    return ibl_i;
}

//https://bruop.github.io/ibl/#single_scattering_results from Fdez-Aguera
float3 GetIBLRadianceLambertian(IBLInput i, float3 n, float2 f_ab, float3 f0, float3 diffuseColor)
{
    float3 irradiance = GetEnvDiffuseColor(n);
#if DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE_PREFILTER_ENV
	return irradiance;
#endif

    // Multiple scattering, from Fdez-Aguera
    float Ems = (1.0 - (f_ab.x + f_ab.y));
    float3 F_avg = i.specularWeight * (f0 + (1.0 - f0) / 21.0);
    float3 FmsEms = Ems * i.FssEss * F_avg / (1.0 - F_avg * Ems);
    
    float3 k_D = diffuseColor * (1.0 - i.FssEss + FmsEms); // we use +FmsEms as indicated by the formula in the blog post (might be a typo in the implementation)

	return (FmsEms + k_D) * irradiance;
}

float3 GetIBLRadianceGGX(IBLInput i, float3 n, float3 v, float perceptualRoughness)
{
    float3 specularLight = GetEnvSpecularColor(n, v, perceptualRoughness);
    
    float3 fcolor = i.specularWeight * specularLight * i.FssEss;
    
#if DEBUG_CHANNEL == DEBUG_CHANNEL_MIP_LEVEL
	fcolor = float3(mip / 32.0, perceptualRoughness, mip * perceptualRoughness);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV_UV
    float3 reflUVW = normalize(reflect(-v, n));
    #if RIGHT_HANDNESS_RESOURCE
	    reflUVW.z = -reflUVW.z;
    #endif	
	fcolor = reflUVW * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV
	fcolor = specularLight;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_LUT
	fcolor = float3(f_ab, 0.0);
#endif
	return fcolor;
}

#endif