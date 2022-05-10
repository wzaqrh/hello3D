#ifndef UNITY_LIGHTING_H
#define UNITY_LIGHTING_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "BRDFCommonFunction.cginc"
#include "ToneMapping.cginc"

float3 UnityPbrLight(float3 l, float3 n, float3 v, float3 albedo, float3 ao_rough_metal)
{
	l = normalize(l);
    float3 h = normalize(l + v);

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
    metalness = 0.0;
    perceptualRoughness = 1.0;
#endif
    
    //diffuse color
    float3 fd_disney = DisneyDiffuse(nv, nl, lh, perceptualRoughness, albedo);
    float reflectivity = lerp(DielectricSpec.x, 1, metalness);
    float kd = 1.0f - reflectivity;
	float3 diffuse_color = MIR_PI * fd_disney * kd * unity_LightColor.rgb * nl; //π，kd，Li，fd_disney，nl

    //D, V
    float roughness = max(perceptualRoughness * perceptualRoughness, 0.002);
    float D = GGXTRDistribution(nh, roughness);
    float V = SmithJointGGXVisibility(nl, nv, roughness);

    //F
    float3 f0  = lerp(DielectricSpec.rgb, albedo, metalness);
    float3 f90 = float3(1,1,1);
    float3 F = SchlickFresnel(f0, f90, lh);

    //specular color
    float3 fs_cook_torrance = max(0.0, D * V) * F;
    float ks = any(f0) ? 1.0 : 0.0; 
    #if _SPECULARHIGHLIGHTS_OFF
	    ks = 0.0;
    #endif
    float3 specular_color = MIR_PI * ks * unity_LightColor.rgb * fs_cook_torrance * nl;
    
    //grazing color
    float surfaceReduction = 1.0 / (roughness * roughness + 1.0);
    float3 f90_ = saturate(1.0 - perceptualRoughness + reflectivity);
    float3 grazing_color = SchlickFresnel(f0, f90_, nv);
     
    float3 finalColor = diffuse_color + specular_color;
    //finalColor = float3(1.0,0.0,0.0);
    //finalColor = albedo;
    finalColor = n * 0.5 + 0.5;
    //finalColor = VectorIdentity * -n;
    //finalColor = VectorIdentity * -v;
    //finalColor = VectorIdentity * -l;
    //finalColor = VectorIdentity * -h;
    //finalColor = VectorIdentity * nl;
    //finalColor = VectorIdentity * lh;
    //finalColor = VectorIdentity * nh;
    //finalColor = VectorIdentity * nv;
#if TONEMAP_MODE
	//finalColor = toneMap(finalColor);
#endif   
    return finalColor;
}

#endif