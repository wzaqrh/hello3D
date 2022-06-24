#ifndef UNITY_LIGHTING_H
#define UNITY_LIGHTING_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "LightingInput.cginc"
#include "CommonFunction.cginc"
#include "BRDFCommonFunction.cginc"
#include "SphericalHarmonics.cginc"
#include "ToneMapping.cginc"

#if ENABLE_LIGHT_MAP
float3 GetLightMap(float2 uv) 
{
    float4 color = MIR_SAMPLE_TEX2D(_LightMap, GetUV(uv, LightMapUV));
#if 0   
    return DecodeLightmapRGBM(color, float4(5,1,0,1));
#else    
    return color.rgb;
#endif    
}
#endif

float3 UnityLightAdditive(LightingInput i, float3 l, float3 n, float3 v)
{
    float3 h = SafeNormalize(l + v);

    float nv = abs(dot(n, v));

    float nl = saturate(dot(n, l));
    float nh = saturate(dot(n, h));
    
    float lh = saturate(dot(l, h));
    float lv = saturate(dot(l, v));
    
    //diffuse color
    float3 fd_disney = DisneyDiffuse(nv, nl, lh, i.percertual_roughness, i.albedo.rgb);
    float reflectivity = lerp(DielectricSpec.r, 1, i.metallic);
    float kd = 1.0f - reflectivity;
	float3 diffuse_color = C_PI * kd * i.light_color.rgb * fd_disney * nl; //π，kd，Li，fd_disney，nl
	
    //D, V
    float roughness = max(i.percertual_roughness * i.percertual_roughness, 0.002);
    float D = GGXTRDistribution(nh, roughness);
    float V = SmithJointGGXVisibility(nl, nv, roughness);
    
    float specularTerm = V * D * C_PI;
	#if COLORSPACE == COLORSPACE_GAMMA
        specularTerm = sqrt(max(1e-4h, specularTerm));
	#endif
    specularTerm = max(0.0, specularTerm * nl);
    
    //F
    float3 f0  = lerp(DielectricSpec.rgb, i.albedo.rgb, i.metallic);
    float3 f90 = float3(1,1,1);
    float3 F = SchlickFresnel(f0, f90, lh);

    //specular color
    float3 fs_cook_torrance = specularTerm * F;
    float ks = any(f0) ? 1.0 : 0.0; 
    float3 specular_color =  ks * i.light_color.rgb * fs_cook_torrance;
    
    //grazing color
    #if COLORSPACE == COLORSPACE_GAMMA
        float surfaceReduction = 1.0-0.28*roughness*i.percertual_roughness;
	#else
	    float surfaceReduction = 1.0 / (roughness * roughness + 1.0);
	#endif
    float3 f90_ = saturate(1.0 - i.percertual_roughness + reflectivity);
    float3 grazing_color = SchlickFresnel(f0, f90_, nv);
     
    float3 finalColor = diffuse_color + specular_color;

#if DEBUG_CHANNEL == DEBUG_CHANNEL_BASECOLOR
    finalColor = i.albedo.rgb * (1.0 - reflectivity);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHADING_NORMAL
    finalColor = n * 0.5 + 0.5;
#elif DEBUG_CHANNEL
    finalColor = 0.0;
#endif
    //finalColor = float3(1.0,0.0,0.0);
    //finalColor = i.albedo.rgb * (1.0 - reflectivity);
    //finalColor = n * 0.5 + 0.5;
    //finalColor = i.percertual_roughness * (1.0 - reflectivity);
    //finalColor = v * 0.5 + 0.5;
    //finalColor = l;
    //finalColor = h;
    //finalColor = lh;
    //finalColor = nl;
    //finalColor = (1-0.5*Pow5(1-nl));
    //finalColor = nv;
    //finalColor = (1-0.5*Pow5(1-nv));
    //finalColor = lh;
    //finalColor = nh;
    return finalColor;
}

float3 UnityLightBase(LightingInput i, float3 l, float3 n, float3 v)
{
    float reflectivity = lerp(DielectricSpec.r, 1, i.metallic);
    float kd = 1.0f - reflectivity;
    
    //env diffuse color
#if ENABLE_LIGHT_MAP
    float3 sh_diffuse_color = kd * i.albedo.rgb * GetLightMap(i.uv);
#else
    float3 sh = GetSphericalHarmonics012(float4(n, 1.0), SHC0C1, SHC2, SHC2_2);
    float3 sh_diffuse_color = kd * i.albedo.rgb * sh;
#endif

    float3 finalColor = sh_diffuse_color;   

#if DEBUG_CHANNEL
    finalColor = 0.0;
#endif
    return finalColor;
}

#endif