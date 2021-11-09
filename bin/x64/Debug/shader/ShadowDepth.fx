/********** ShadowCaster **********/
#include "Standard.h"
#include "Skeleton.h"

struct SHADOW_VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
	float3 Tangent : NORMAL1;
	float2 Tex  : TEXCOORD0;
    float4 BlendWeights : BLENDWEIGHT;
    uint4  BlendIndices : BLENDINDICES;
};

struct SHADOW_PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 Depth : TEXCOORD0;
};

SHADOW_PS_INPUT VS( SHADOW_VS_INPUT i)
{
	SHADOW_PS_INPUT output;
	
	float4 skinPos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0));
	
	matrix MW = mul(World, transpose(Model));
	matrix MWVP = mul(Projection,mul(View, MW));
	output.Pos = mul(MWVP, skinPos);
	output.Depth = output.Pos;	
	
	return output;
}

float4 PS(SHADOW_PS_INPUT i) : SV_Target
{
	float depthValue = i.Depth.z / i.Depth.w;
	float4 finalColor = depthValue;
	finalColor = pow(depthValue,64);
	return finalColor;
}