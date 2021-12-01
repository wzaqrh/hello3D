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
	float4 PosInLight : TEXCOORD1;//light's ndc space
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	matrix WVP = mul(Projection,mul(View, World));
	
    output.Pos = mul(WVP, float4(input.Pos,1.0));
	output.Color = input.Color;
	output.Tex = input.Tex;
	
	output.PosInLight = mul(World, float4(input.Pos,1.0));
	//output.PosInLight = mul(LightView, output.PosInLight);
	//output.PosInLight = mul(LightProjection, output.PosInLight);
	output.PosInLight = mul(View, output.PosInLight);
	output.PosInLight = mul(Projection, output.PosInLight);
	
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{	
#if 0
	float4 diffuseColor = GetTextureMain(input.Tex);
	float4 finalColor = diffuseColor * input.Color;
	//return diffuseColor;
	return float4(1.0,0.0,0.0,1.0);
#elif 0
	float4 finalColor = float4(0,0,0,1);

	float4 shadowPosH = input.Pos;
	shadowPosH.xyz /= shadowPosH.w;
	//shadowPosH.xy = shadowPosH.xy * 0.5 + 0.5;
	
	//finalColor.x = shadowPosH.x;
	
	finalColor.r = txDepthMap.Sample(samLinear, shadowPosH.xy).r;
	
	return finalColor;
#else
	float4 finalColor = float4(0,0,0,1);

	float4 shadowPosH = input.PosInLight;
	shadowPosH.xyz /= shadowPosH.w;
	shadowPosH.xy = shadowPosH.xy * 0.5 + 0.5;
	shadowPosH.y = 1.0 - shadowPosH.y;
	
	//finalColor.x = shadowPosH.x;
	finalColor.r = txDepthMap.Sample(samLinear, shadowPosH.xy).r;

	return finalColor;
#endif
}



