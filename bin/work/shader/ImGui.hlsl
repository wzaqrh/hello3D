#include "Standard.cginc"

MIR_DECLARE_TEX2D(_MainTex, 0);

cbuffer cbImGui : register(b3) 
{
	matrix ProjectionMatrix;
}
			
struct vbImGui
{
    float2 Pos : POSITION;
    float2 Tex  : TEXCOORD0;
	float4 Color : COLOR0;
};
			
struct PixelInput
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD0;
};
PixelInput VS(vbImGui input)
{
	PixelInput output = (PixelInput)0;
	output.Pos = mul(ProjectionMatrix, float4(input.Pos,0.0,1.0));
	output.Color = input.Color;
	output.Tex = input.Tex;
	return output;
}

float4 PS(PixelInput input) : SV_Target
{
	float4 finalColor = input.Color;
	finalColor *= MIR_SAMPLE_TEX2D(_MainTex, input.Tex);
	return finalColor;
}



