/********** Load Model FBX **********/
#include "Standard.h"
#include "Skeleton.h"

Texture2D txDiffuse : register( t0 );

struct VS_INPUT
{
    float4 Pos : POSITION;
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
	output.Pos = Skinning(float4(i.Pos.xyz, 1.0), i.BlendWeights, i.BlendIndices);
	output.Pos = mul(output.Pos, Model);
	output.Pos = mul(World, output.Pos);
    output.Pos = mul(View, output.Pos);
    output.Pos = mul(Projection, output.Pos);
    
	output.Tex = i.Tex;
    return output;
}

float4 PS( PS_INPUT input) : SV_Target
{	
	return txDiffuse.Sample(samLinear, input.Tex);// + float4(0.2,0.2,0.2,1.0);
	//return float4(1.0, 0.0, 0.0, 1.0);
}