#ifndef GLTF_LIGHTING_H
#define GLTF_LIGHTING_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "LightingInput.cginc"
#include "BRDFCommonFunction.cginc"
#include "ToneMapping.cginc"
#include "IBL.cginc"

float3 GltfPbrLight(LightingInput i, float3 l, float3 n, float3 v)
{
    float3 h = normalize(l + v);
    float nl = saturate(dot(n, l));
    float lh = saturate(dot(l, h));
    float nh = saturate(dot(n, h));
    float nv = saturate(dot(n, v));
	float ao = i.ao_rough_metal.x;
    float perceptualRoughness = i.ao_rough_metal.y;
	float metallic = i.ao_rough_metal.z;
    float roughness = perceptualRoughness * perceptualRoughness;
    float3 f0 = lerp(DielectricSpec.rgb, i.albedo.rgb, metallic);
    float3 f90 = float3(1, 1, 1);
    float specularWeight = 1.0;
    
    float3 fcolor = float3(0, 0, 0);
#if USE_PUNCTUAL   
    float3 diffuse = LambertDiffuse(i.albedo.rgb * (1.0 - metallic));
    
    float D = GGXTRDistribution(nh, roughness);
    float V = SmithJointGGXFilamentVisibility(nl, nv, roughness);
    float3 F = SchlickFresnel(f0, f90, lh);
    float3 specular = D * V * F;
    
    float3 kd = float3(1.0, 1.0, 1.0) - specularWeight * F;
    float ks = specularWeight;
	diffuse  = kd * diffuse * LightColor.rgb * nl;
	specular = ks * specular * LightColor.rgb * nl;
    fcolor += diffuse + specular;
#endif    
    
#if USE_IBL
    float3 ibl_diff = GetIBLRadianceLambertian(n, v, roughness, i.albedo.rgb * (1.0 - metallic), f0, specularWeight);
    float3 ibl_spec = GetIBLRadianceGGX(n, v, perceptualRoughness, f0, specularWeight);    
	fcolor += (ibl_diff + ibl_spec) * ao;
#endif

	fcolor += i.emissive.rgb;

#if TONEMAP_MODE
	fcolor = toneMap(fcolor, CameraPositionExposure.w);
#endif
    
#if DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_DIFFUSE
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
    fcolor = GetIBLRadianceLambertian(n, v, roughness, i.albedo.rgb * (1.0 - metallic), f0, specularWeight); 
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_LUT || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV_UV || DEBUG_CHANNEL == DEBUG_CHANNEL_MIP_LEVEL	
	fcolor = GetIBLRadianceGGX(n, v, perceptualRoughness, f0, specularWeight); 
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_METTALIC_ROUGHNESS 
    fcolor = float3(0, 0, 0);
	#if USE_PUNCTUAL
		fcolor += diffuse + specular;
	#endif
	#if USE_IBL
		fcolor += (ibl_diff + ibl_spec) * i.ao_rough_metal.x;
	#endif
	fcolor = linearTosRGB(fcolor);
#endif
    return fcolor;
}

#endif