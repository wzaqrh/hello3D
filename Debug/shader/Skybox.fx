/********** Multi Light(Direct Point Spot) (eye space) (SpecularMap NormalMapping) **********/
#include "Standard.h"
SamplerState samLinear : register(s0);
TextureCube  txDiffuse : register(t0);

struct VS_INPUT
{
    float4 Pos : POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float3 Tex : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = input.Pos;
	
	matrix WVPInv = mul(WorldInv,mul(ViewInv,ProjectionInv));
	output.Tex = normalize(mul(WVPInv, input.Pos));
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float4 finalColor = 1.0;
	finalColor.rgb = txDiffuse.Sample(samLinear, input.Tex);
	return finalColor;
}



