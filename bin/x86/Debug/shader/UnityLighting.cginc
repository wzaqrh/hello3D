#ifndef UNITY_LIGHTING_H
#define UNITY_LIGHTING_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "LightingInput.cginc"
#include "CommonFunction.cginc"
#include "BRDFCommonFunction.cginc"
#include "SphericalHarmonics.cginc"
#include "ToneMapping.cginc"

float3 GetLightMap(float2 uv) 
{
    float4 color = MIR_SAMPLE_TEX2D(_LightMap, GetUV(uv, LightMapUV));
#if 0   
    return DecodeLightmapRGBM(color, float4(5,1,0,1));
#else    
    return color.rgb;
#endif    
}

float3 UnityPbrLight(LightingInput i, float3 l, float3 n, float3 v)
{
    float3 h = SafeNormalize(l + v);

    float nv = abs(dot(n, v));

    float nl = saturate(dot(n, l));
    float nh = saturate(dot(n, h));
    
    float lh = saturate(dot(l, h));
    float lv = saturate(dot(l, v));
    
    float ao = i.ao_rough_metal_tx.x;
    float perceptualRoughness = i.ao_rough_metal_tx.y;
    float metalness = i.ao_rough_metal_tx.z;
    
    //diffuse color
    float3 fd_disney = DisneyDiffuse(nv, nl, lh, perceptualRoughness, i.albedo.rgb);
    float reflectivity = lerp(DielectricSpec.r, 1, metalness);
    float kd = 1.0f - reflectivity;
	float3 diffuse_color = MIR_PI * kd * LightColor.rgb * fd_disney * nl; //π，kd，Li，fd_disney，nl
	
    //env diffuse color
#if ENABLE_LIGHT_MAP  
    float3 sh_diffuse_color = kd * i.albedo.rgb * GetLightMap(i.uv);
#else
    float3 sh = GetSphericalHarmonics012(float4(n, 1.0), SHC0C1, SHC2, SHC2_2);
    float3 sh_diffuse_color = kd * i.albedo.rgb * sh;
#endif
    
    //D, V
    float roughness = max(perceptualRoughness * perceptualRoughness, 0.002);
    float D = GGXTRDistribution(nh, roughness);
    float V = SmithJointGGXVisibility(nl, nv, roughness);
    
    float specularTerm = V * D * MIR_PI;
	#if COLORSPACE == COLORSPACE_GAMMA
        specularTerm = sqrt(max(1e-4h, specularTerm));
	#endif
    specularTerm = max(0.0, specularTerm * nl);
    
    //F
    float3 f0  = lerp(DielectricSpec.rgb, i.albedo.rgb, metalness);
    float3 f90 = float3(1,1,1);
    float3 F = SchlickFresnel(f0, f90, lh);

    //specular color
    float3 fs_cook_torrance = specularTerm * F;
    float ks = any(f0) ? 1.0 : 0.0; 
    float3 specular_color =  ks * LightColor.rgb * fs_cook_torrance;
    
    //grazing color
    #if COLORSPACE == COLORSPACE_GAMMA
        float surfaceReduction = 1.0-0.28*roughness*perceptualRoughness;
	#else
	    float surfaceReduction = 1.0 / (roughness * roughness + 1.0);
	#endif
    float3 f90_ = saturate(1.0 - perceptualRoughness + reflectivity);
    float3 grazing_color = SchlickFresnel(f0, f90_, nv);
     
    //float3 finalColor = diffuse_color + sh_diffuse_color + specular_color;
    float3 finalColor = sh_diffuse_color;
#if TONEMAP_MODE
	finalColor = toneMap(finalColor, CameraPositionExposure.w);
#endif

#if DEBUG_CHANNEL == DEBUG_CHANNEL_BASECOLOR
    finalColor = i.albedo.rgb * (1.0 - reflectivity);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHADING_NORMAL
    finalColor = n * 0.5 + 0.5;
#endif

    #define VectorIdentity float3(1,1,1)   
    //finalColor = float3(1.0,0.0,0.0);
    //finalColor = i.albedo.rgb * (1.0 - reflectivity);
    //finalColor = n * 0.5 + 0.5;
    //finalColor = VectorIdentity * perceptualRoughness * (1.0 - reflectivity);
    //finalColor = VectorIdentity * v * 0.5 + 0.5;
    //finalColor = VectorIdentity * l;
    //finalColor = VectorIdentity * h;
    //finalColor = VectorIdentity * lh;
    //finalColor = VectorIdentity * nl;
    //finalColor = VectorIdentity * (1-0.5*Pow5(1-nl));
    //finalColor = VectorIdentity * nv;
    //finalColor = VectorIdentity * (1-0.5*Pow5(1-nv));
    //finalColor = VectorIdentity * lh;
    //finalColor = VectorIdentity * nh;
    return finalColor;
}

#endif