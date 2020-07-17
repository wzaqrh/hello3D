/********** Sprite **********/
#include "Standard.h"

struct VS_INPUT
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
	float2 Tex  : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	matrix WVP = mul(Projection,mul(View, World));
	
    output.Pos = mul(WVP, float4(input.Pos,1.0));
	output.Color = input.Color;
	output.Tex = input.Tex;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float color = GetTextureMain(input.Tex).r;
	float4 finalColor = input.Color * color;
	return finalColor;
}



