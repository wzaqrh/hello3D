#ifndef LIGHTING_PBR_H
#define LIGHTING_PBR_H
#include "IBL.cginc"
#include "ToneMapping.cginc"
#include "Debug.cginc"

#define MIR_EPS 1e-7f
#define MIR_PI  3.141592f
#define MIR_INV_PI 0.31830988618f

inline float3 MakeDummyColor(float3 toEye)
{
	float3 fcolor = float3(1, 0, 1);
    fcolor.r = fcolor.b = float(max(2.0 * sin(0.1 * (toEye.x + toEye.y) * 1024), 0.0) + 0.3); 
	return fcolor;
}

inline float3 DisneyDiffuse(float NdotV, float NdotL, float LdotH, float perceptualRoughness, float3 baseColor)
{
    float fd90 = 0.5 + 2 * LdotH * LdotH * perceptualRoughness;
    float lightScatter = (1 + (fd90 - 1) * pow(1 - NdotL,5));
    float viewScatter  = (1 + (fd90 - 1) * pow(1 - NdotV,5));
    return baseColor / MIR_PI * lightScatter * viewScatter;
}
inline float3 LambertDiffuse(float3 baseColor)
{
    return baseColor / MIR_PI;
}

inline float GGXTRDistribution(float NdotH, float roughness) 
{
	float a2 = roughness * roughness;
    float denom = NdotH * NdotH * (a2 - 1) + 1.0f;
    return a2 / (MIR_PI * denom * denom + MIR_EPS);
}

inline float SmithJointGGXVisibility(float NdotL, float NdotV, float roughness)
{
    float lambdaV = NdotL * (NdotV * (1 - roughness) + roughness);
    float lambdaL = NdotV * (NdotL * (1 - roughness) + roughness);
    return 0.5 / (lambdaV + lambdaL + MIR_EPS);
}
inline float SmithJointGGXFilamentVisibility(float NdotL, float NdotV, float roughness)
{
    float a2 = roughness * roughness;
    float lambdaV = NdotL * sqrt((1 - a2) * NdotV * NdotV + a2);
    float lambdaL = NdotV * sqrt((1 - a2) * NdotL * NdotL + a2);
	if (lambdaV + lambdaL <= 0.0) return 0.0;
    else return 0.5 / (lambdaV + lambdaL);
}

inline float3 SchlickFresnel(float3 F0, float3 F90, float VdotH)
{
    return F0 + (F90 - F0) * pow(1 - VdotH, 5);
}

#define VectorIdentity float3(1,1,1)
#define DielectricSpec float4(0.04, 0.04, 0.04, 1.0 - 0.04)
float3 UnityPbrLight(float3 toLight_, float3 normal, float3 toEye, float3 albedo, float3 ao_rough_metal)
{
	float lengthSq = max(dot(toLight_, toLight_), 0.000001);
	toLight_ *= rsqrt(lengthSq);
	
    float3 halfView = normalize(toLight_ + toEye);
    float NdotL = saturate(dot(normal, toLight_));
    float LdotH = saturate(dot(toLight_, halfView));
    float NdotH = saturate(dot(normal, halfView));
    float NdotV = saturate(dot(normal, toEye));
	float perceptualRoughness = ao_rough_metal.y;
    float alpha = max(perceptualRoughness * perceptualRoughness, 0.002);
    float3 F0 = lerp(DielectricSpec.rgb, albedo, ao_rough_metal.z);
    float3 F90 = float3(1,1,1);
    
	float3 diffuse = DisneyDiffuse(NdotV, NdotL, LdotH, perceptualRoughness, albedo);
    
    float D = GGXTRDistribution(NdotH, alpha);
    float3 F = SchlickFresnel(F0, F90, LdotH);
    float V = SmithJointGGXVisibility(NdotL, NdotV, alpha);
    float3 specular = D * F * V;
    
    float reflectivity = lerp(DielectricSpec.x, 1, ao_rough_metal.z);
    float kd = 1.0f - reflectivity;
    float ks = 1.0f;
    //kd = 0.0f;
    //ks = 1.0;
    float3 finalColor = (kd * diffuse + ks * specular) * unity_LightColor.rgb * NdotL;
    //finalColor = diffuse;
    //finalColor = VectorIdentity * -normal;
    //finalColor = VectorIdentity * -toEye;
    //finalColor = VectorIdentity * -toLight_;
    //finalColor = VectorIdentity * -halfView;
    //finalColor = VectorIdentity * NdotL;
    //finalColor = VectorIdentity * LdotH;
    //finalColor = VectorIdentity * NdotH;
    //finalColor = VectorIdentity * NdotV;
    return finalColor;
}

float3 gltfPbrLight(float3 toLight, float3 normal, float3 toEye, float3 albedo, float3 ao_rough_metal, float3 emissive)
{
    float3 halfView = normalize(toLight + toEye);
    float NdotL = saturate(dot(normal, toLight));
    float LdotH = saturate(dot(toLight, halfView));
    float NdotH = saturate(dot(normal, halfView));
    float NdotV = saturate(dot(normal, toEye));
    float perceptualRoughness = ao_rough_metal.y;
    float roughness = perceptualRoughness * perceptualRoughness;
    float3 F0 = lerp(DielectricSpec.rgb, albedo, ao_rough_metal.z);
    float3 F90 = float3(1, 1, 1);
    float specularWeight = 1.0;
    
    float3 fcolor = float3(0, 0, 0);
#if USE_PUNCTUAL   
    float3 diffuse = LambertDiffuse(albedo * (1.0 - ao_rough_metal.z));
    
    float D = GGXTRDistribution(NdotH, roughness);
    float V = SmithJointGGXFilamentVisibility(NdotL, NdotV, roughness);
    float3 F = SchlickFresnel(F0, F90, LdotH);
    float3 specular = D * V * F;
    
    float3 kd = float3(1.0, 1.0, 1.0) - specularWeight * F;
    float ks = specularWeight;
	diffuse  = kd * diffuse * unity_LightColor.rgb * NdotL;
	specular = ks * specular * unity_LightColor.rgb * NdotL;
    fcolor += diffuse + specular;
#endif    
    
#if USE_IBL
    float3 ibl_diff = GetIBLRadianceLambertian(normal, toEye, roughness, albedo * (1.0 - ao_rough_metal.z), F0, specularWeight);
    float3 ibl_spec = GetIBLRadianceGGX(normal, toEye, perceptualRoughness, F0, specularWeight);    
	fcolor += (ibl_diff + ibl_spec) * ao_rough_metal.x;
#endif
	fcolor += emissive;
#if TONEMAP_MODE
	fcolor = toneMap(fcolor);
#endif
	
#if DEBUG_CHANNEL == DEBUG_CHANNEL_OCCLUSION
	fcolor = linearTosRGB(float3(ao_rough_metal.x, ao_rough_metal.x, ao_rough_metal.x));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_EMISSIVE
	fcolor = linearTosRGB(emissive);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_VECTOR_L
	fcolor = toLight;
	fcolor.z = -fcolor.z;
	fcolor = (fcolor + 1.0) / 2.0;	
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_VECTOR_V
	fcolor = toEye;
	fcolor.z = -fcolor.z;
	fcolor = (fcolor + 1.0) / 2.0;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_INTENSITY_NDOTL
	fcolor = float3(NdotL, NdotL, NdotL);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_VECTOR_R
	fcolor = normalize(reflect(-toEye, normal));
	fcolor.z = -fcolor.z;
	fcolor = (fcolor + 1.0) / 2.0;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_DIFFUSE
	fcolor = kd * diffuse;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR
	fcolor = ks * specular;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_D
	fcolor = float3(D, D, D);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_V
	fcolor = float3(V, V, V);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_F
	fcolor = F;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE_PREFILTER_ENV
    fcolor = GetIBLRadianceLambertian(normal, toEye, roughness, albedo * (1.0 - ao_rough_metal.z), F0, specularWeight); 
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_LUT || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV_UV || DEBUG_CHANNEL == DEBUG_CHANNEL_MIP_LEVEL	
	fcolor = GetIBLRadianceGGX(normal, toEye, perceptualRoughness, F0, specularWeight); 
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_METTALIC_ROUGHNESS 
    fcolor = float3(0, 0, 0);
	#if USE_PUNCTUAL
	fcolor += diffuse + specular;
	#endif
	#if USE_IBL
	fcolor += (ibl_diff + ibl_spec) * ao_rough_metal.x;
	#endif
	fcolor = linearTosRGB(fcolor);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BASECOLOR
    fcolor = linearTosRGB(albedo);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_METTALIC
    fcolor = linearTosRGB(float3(ao_rough_metal.z, ao_rough_metal.z, ao_rough_metal.z));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_PERCEPTUAL_ROUGHNESS
    fcolor = (float3(perceptualRoughness,perceptualRoughness,perceptualRoughness));
#elif DEBUG_CHANNEL != 0
    fcolor = MakeDummyColor(toEye);
#endif
    return fcolor;
}

#endif