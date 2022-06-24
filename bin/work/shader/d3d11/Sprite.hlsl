#include "Standard.cginc"
#include "Lighting.cginc"

MIR_DECLARE_TEX2D(_MainTex, 0);

struct PixelInput
{
    float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD0;
};

PixelInput VS(vbSurface input)
{
	PixelInput output = (PixelInput)0;
	matrix WVP = mul(Projection,mul(View, World));
	
    output.Pos = mul(WVP, float4(input.Pos,1.0));
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



