/********** Skybox **********/
#include "Standard.h"

#if SHADER_MODEL > 30000
TextureCube tcubeSkybox : register(t0);
#else
texture  textureCubeSkybox : register(t0);
samplerCUBE tcubeSkybox : register(s0) =
sampler_state
{
    Texture = <textureCubeSkybox>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};
#endif

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
	finalColor.rgb = GetTextureCube(tcubeSkybox, samLinear, input.Tex);
	return finalColor;
}


