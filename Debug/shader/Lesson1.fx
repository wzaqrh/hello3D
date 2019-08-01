/********** Load Model FBX **********/
#include "Standard.h"
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

static const int MAX_MATRICES = 256;
cbuffer cbWeightedSkin : register(b1)
{
	matrix Model;
	matrix Models[MAX_MATRICES] : WORLDMATRIXARRAY;	
}

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

float4 Skinning(float4 iPos, float4 iBlendWeights, uint4 iBlendIndices)
{
    float BlendWeights[4] = (float[4])iBlendWeights;
	BlendWeights[3] = 1.0 - BlendWeights[0] - BlendWeights[1] - BlendWeights[2];
    uint  BlendIndices[4] = (uint[4])iBlendIndices;	
	
    float4	Pos = float4(0.0,0.0,0.0,1.0);   
	const int NumBones = 4;
    for (int iBone = 0; iBone < NumBones; iBone++) {
		uint Indice = BlendIndices[iBone];
		float Weight = BlendWeights[iBone];
			
		float4 bonePos = mul(iPos, Models[Indice]);
        Pos.xyz += bonePos.xyz * Weight;
    }
	return Pos;
}

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