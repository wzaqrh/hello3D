/********** Skybox **********/
#include "Standard.h"

#if SHADER_MODEL > 30000
TextureCube _MainTex : register(t0);
#else
texture  textureCubeSkybox : register(t0);
samplerCUBE _MainTex : register(s0);
#endif

struct VertexInput
{
    float4 Pos : POSITION;
};

struct PixelInput
{
    float4 Pos : SV_POSITION;
	float3 Tex : TEXCOORD0;
};

PixelInput VS(VertexInput input)
{
	PixelInput output = (PixelInput)0;
	output.Pos = input.Pos;
	
	matrix WVPInv = mul(WorldInv, mul(ViewInv, ProjectionInv));
	output.Tex = normalize(mul(WVPInv, input.Pos));
    return output;
}

float4 PS(PixelInput input) : SV_Target
{	
	float4 finalColor;
	finalColor.rgb = GetTextureCube(_MainTex, samLinear, input.Tex);
	finalColor.a = 1.0;
	return finalColor;
}


