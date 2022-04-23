#ifndef SKELETON_H
#define SKELETON_H

struct vbWeightedSkin
{
	float3 Normal : NORMAL;
	float3 Tangent : NORMAL1;
	float3 BiTangent : NORMAL2;
	float4 BlendWeights : BLENDWEIGHT;
	uint4  BlendIndices : BLENDINDICES;
};

static const int MAX_MATRICES = 56;
cbuffer cbWeightedSkin : register(b2)
{
	matrix Model;
	matrix Models[MAX_MATRICES] : WORLDMATRIXARRAY;	
}

float4 Skinning(float4 iBlendWeights, uint4 iBlendIndices, float4 iPos)
{
    float4 Pos = float4(0.0,0.0,0.0,1.0); 	
	Pos.xyz += mul(iPos, Models[iBlendIndices.x]).xyz * iBlendWeights.x;
	Pos.xyz += mul(iPos, Models[iBlendIndices.y]).xyz * iBlendWeights.y;
	Pos.xyz += mul(iPos, Models[iBlendIndices.z]).xyz * iBlendWeights.z;
	Pos.xyz += mul(iPos, Models[iBlendIndices.w]).xyz * iBlendWeights.w;
	Pos.xyz += iPos * max(0.0, 1.0 - dot(iBlendWeights, float4(1.0, 1.0, 1.0, 1.0)));
	return Pos;
}
#endif