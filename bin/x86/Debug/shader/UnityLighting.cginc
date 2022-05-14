#ifndef UNITY_LIGHTING_H
#define UNITY_LIGHTING_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "BRDFCommonFunction.cginc"
#include "ToneMapping.cginc"


#define VectorIdentity float3(1,1,1)
#define DielectricSpec float4(0.220916301, 0.220916301, 0.220916301, 1.0 - 0.220916301)
float3 UnityPbrLight(float3 l, float3 n, float3 v, float3 albedo, float3 ao_rough_metal)
{
	l = normalize(l);
    float3 h = SafeNormalize(l + v);

    float nv = abs(dot(n, v));

    float nl = saturate(dot(n, l));
    float nh = saturate(dot(n, h));
    
    float lh = saturate(dot(l, h));
    float lv = saturate(dot(l, v));
    
    float ao = ao_rough_metal.x;
    float perceptualRoughness = ao_rough_metal.y;
    float metalness = ao_rough_metal.z;
    
#if 1
    #define _SPECULARHIGHLIGHTS_OFF 1
    #define PBS3 1
    metalness = 0.0;
    perceptualRoughness = 0.0;
#endif
    
    //diffuse color
    float3 fd_disney = DisneyDiffuse(nv, nl, lh, perceptualRoughness, float3(1,1,1));
    float reflectivity = lerp(DielectricSpec.x, 1, metalness);
    float kd = 1.0f - reflectivity;
	float3 diffuse_color = kd * albedo * unity_LightColor.rgb * fd_disney * nl; //π，kd，Li，fd_disney，nl
	//diffuse_color = kd * albedo * unity_LightColor.rgb * nl;
	
    #if PBS3
        //diffuse_color = kd * albedo * unity_LightColor.rgb * nl;
    #endif
    //D, V
    float roughness = max(perceptualRoughness * perceptualRoughness, 0.002);
    float D = GGXTRDistribution(nh, roughness);
    float V = SmithJointGGXVisibility(nl, nv, roughness);
    
    float specularTerm = V * D * MIR_PI;
	#if COLORSPACE_GAMMA
        specularTerm = sqrt(max(1e-4h, specularTerm));
	#endif
    specularTerm = max(0.0, specularTerm * nl);
    
    //F
    float3 f0  = lerp(DielectricSpec.rgb, albedo, metalness);
    float3 f90 = float3(1,1,1);
    float3 F = SchlickFresnel(f0, f90, lh);

    //specular color
    float3 fs_cook_torrance = specularTerm * F;
    float ks = any(f0) ? 1.0 : 0.0; 
    #if _SPECULARHIGHLIGHTS_OFF
	    ks = 0.0;
    #endif
    float3 specular_color =  ks * unity_LightColor.rgb * fs_cook_torrance;
    
    //grazing color
    #if COLORSPACE_GAMMA
        float surfaceReduction = 1.0-0.28*roughness*perceptualRoughness;
	#else
	    float surfaceReduction = 1.0 / (roughness * roughness + 1.0);
	#endif
    float3 f90_ = saturate(1.0 - perceptualRoughness + reflectivity);
    float3 grazing_color = SchlickFresnel(f0, f90_, nv);
     
    float3 finalColor = diffuse_color;// + specular_color;
#if TONEMAP_MODE
	finalColor = toneMap(finalColor);
#endif

#if DEBUG_CHANNEL == DEBUG_CHANNEL_BASECOLOR
    finalColor = albedo * (1.0 - reflectivity);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHADING_NORMAL
    finalColor = n * 0.5 + 0.5;
#endif
	//finalColor = albedo * (1.0 - reflectivity);
    //finalColor = n * 0.5 + 0.5;

    //finalColor = float3(1.0,0.0,0.0);
    //finalColor = albedo;
    //finalColor = n * 0.5 + 0.5;
    //finalColor = VectorIdentity * -n;
    //finalColor = VectorIdentity * v * 0.5 + 0.5;
    //finalColor = VectorIdentity * l;
    //finalColor = VectorIdentity * -h;
    //finalColor = VectorIdentity * nl;
    //finalColor = VectorIdentity * (1-0.5*Pow5(1-nl));
    //finalColor = VectorIdentity * nv;
    //finalColor = VectorIdentity * (1-0.5*Pow5(1-nv));
    //finalColor = VectorIdentity * lh;
    //finalColor = VectorIdentity * nh;
    return finalColor;
}

#endif