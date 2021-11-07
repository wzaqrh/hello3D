static const int MAX_MATRICES = 56;
cbuffer cbWeightedSkin : register(b1)
{
	matrix Model;
	matrix Models[MAX_MATRICES] : WORLDMATRIXARRAY;	
	int hasNormal;
	int hasMetalness;
	int hasRoughness;
	int hasAO;
}

float4 Skinning(float4 iBlendWeights, int4 iBlendIndices, float4 iPos)
{
    float4 Pos = float4(0.0,0.0,0.0,iPos.w); 	
	Pos.xyz += mul(iPos, Models[iBlendIndices.x]).xyz * iBlendWeights.x;
	Pos.xyz += mul(iPos, Models[iBlendIndices.y]).xyz * iBlendWeights.y;
	Pos.xyz += mul(iPos, Models[iBlendIndices.z]).xyz * iBlendWeights.z;
	Pos.xyz += mul(iPos, Models[iBlendIndices.w]).xyz * (1.0 - iBlendWeights.x - iBlendWeights.y - iBlendWeights.z);
	return Pos;
}