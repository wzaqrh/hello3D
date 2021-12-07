/********** Skybox **********/
#include "Standard.h"

MIR_DECLARE_TEXCUBE(_MainTex, 0);

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
	finalColor.rgb = MIR_SAMPLE_TEXCUBE(_MainTex, input.Tex).rgb;
	finalColor.a = 1.0;
	return finalColor;
}


