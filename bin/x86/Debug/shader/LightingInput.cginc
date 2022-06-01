#ifndef LIGHTING_INPUT_H
#define LIGHTING_INPUT_H
#include "Macros.cginc"

#if COLORSPACE_GAMMA
    #define DielectricSpec float4(0.220916301, 0.220916301, 0.220916301, 1.0 - 0.220916301)
#else
    #define DielectricSpec float4(0.04, 0.04, 0.04, 1.0 - 0.04)
#endif

struct LightingInput
{
	float4 albedo; 
	float4 ao_rough_metal_tx; 
	float4 emissive; 
	float2 uv;
	float3 world_pos;
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