#ifndef LIGHTING_INPUT_H
#define LIGHTING_INPUT_H
#include "Macros.cginc"

#if COLORSPACE == COLORSPACE_GAMMA
    #define DielectricSpec float4(0.220916301, 0.220916301, 0.220916301, 1.0 - 0.220916301)
#else
    #define DielectricSpec float4(0.04, 0.04, 0.04, 1.0 - 0.04)
#endif

struct LightingInput
{
	float3 albedo; 
	float ao;
	float3 emissive;
	float transmission_factor;
	float3 world_pos;
	float percertual_roughness;
	float4 sheen_color_roughness;
	float metallic;
	float2 uv;
#if DEBUG_CHANNEL
    float2 uv1;
	float3 tangent_normal;
    float3 normal_basis;
	float3 tangent_basis;
	float3 bitangent_basis;
	float3 window_pos;
#endif
};

#endif