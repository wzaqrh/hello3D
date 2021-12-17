/********** Skybox **********/
#include "Standard.h"

MIR_DECLARE_TEXCUBE(_MainTex, 0);

struct VertexInput
{
    float3 Pos : POSITION;
};

struct PixelInput
{
    float4 Pos : SV_POSITION;
	float3 Tex : TEXCOORD0;
};

#if DEPRECATE_SKYBOX
PixelInput VS(VertexInput input)
{
	PixelInput output;
	output.Pos = float4(input.Pos, 1.0);
	
	matrix WVPInv = mul(ViewInv, ProjectionInv);
	output.Tex = normalize(mul(WVPInv, output.Pos));
    return output;
}
#else
PixelInput VS(VertexInput input)
{
	PixelInput output;
	output.Tex = float3(input.Pos);
	output.Pos = mul(View, float4(input.Pos, 0.0));
	output.Pos = mul(Projection, output.Pos).xyww;
	return output;
}
#endif

float4 PS(PixelInput input) : SV_Target
{	
	float4 finalColor;
	finalColor.rgb = MIR_SAMPLE_TEXCUBE(_MainTex, input.Tex).rgb;
	finalColor.a = 1.0;
	return finalColor;
}