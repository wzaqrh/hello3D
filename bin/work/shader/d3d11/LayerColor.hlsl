#include "Standard.cginc"
#include "Lighting.cginc"

/************ ForwardBase ************/
struct PixelInput
{
    float4 Pos : SV_POSITION;
	float4 Color : COLOR;
};

PixelInput VS(vbSurface input)
{
	PixelInput output;
	matrix WVP = mul(Projection, mul(View, World));
    output.Pos = mul(WVP, float4(input.Pos,1.0));
	output.Color = input.Color;
    return output;
}

float4 PS(PixelInput input) : SV_Target
{	
	return input.Color;
}