#ifndef LIGHTING_PBR_H
#define LIGHTING_PBR_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "BRDFCommonFunction.cginc"
#include "ToneMapping.cginc"
#include "IBL.cginc"

#define DielectricSpec float4(0.04, 0.04, 0.04, 1.0 - 0.04)

float3 GltfPbrLight(float3 toLight, float3 normal, float3 toEye, float3 albedo, float3 ao_rough_metal, float3 emissive)
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
	diffuse  = kd * diffuse * LightColor.rgb * NdotL;
	specular = ks * specular * LightColor.rgb * NdotL;
    fcolor += diffuse + specular;
#endif    
    
#if USE_IBL
    float3 ibl_diff = GetIBLRadianceLambertian(normal, toEye, roughness, albedo * (1.0 - ao_rough_metal.z), F0, specularWeight);
    float3 ibl_spec = GetIBLRadianceGGX(normal, toEye, perceptualRoughness, F0, specularWeight);    
	fcolor += (ibl_diff + ibl_spec) * ao_rough_metal.x;
#endif
	fcolor += emissive;
#if TONEMAP_MODE
	fcolor = toneMap(fcolor, CameraPositionExposure.w);
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
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHADING_NORMAL
	fcolor.rgb = normal.xyz * float3(1.0,1.0,-1.0) * 0.5 + 0.5;
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