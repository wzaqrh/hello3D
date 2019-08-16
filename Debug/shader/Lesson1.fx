/********** Load Model FBX **********/
#include "Standard.h"
#include "Skeleton.h"

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Norm : NORMAL;
	float3 Tangent : NORMAL1;
	float2 Tex  : TEXCOORD0;
    float4 BlendWeights : BLENDWEIGHT;
    uint4  BlendIndices : BLENDINDICES;
	float3 BiTangent : NORMAL2;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
	float2 Tex  : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT i)
{
	PS_INPUT output = (PS_INPUT)0;
	matrix WVP = mul(Projection,mul(View, World));
#if 1
	matrix MWVP = mul(WVP, transpose(Model));
	
	float4 skinPos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0));
	//float4 skinPos = float4(i.Pos.xyz, 1.0);
	output.Pos = mul(MWVP, skinPos);
#else
	output.Pos = mul(WVP, float4(i.Pos,1.0));
#endif
		
	output.Tex = i.Tex;
    return output;
}

float4 PS( PS_INPUT input) : SV_Target
{
	float4 finalColor = GetTextureMain(input.Tex);
	return finalColor;
}