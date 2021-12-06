/********** Sprite **********/
#include "Standard.h"
#include "Lighting.h"

struct PixelInput
{
    float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD0;
#if DEBUG_SHADOW_MAP
	float4 PosInLight : TEXCOORD1;//light's ndc space
#endif
};

PixelInput VS(vbSurface input)
{
	PixelInput output = (PixelInput)0;
	matrix WVP = mul(Projection,mul(View, World));
	
    output.Pos = mul(WVP, float4(input.Pos,1.0));
	output.Color = input.Color;
	output.Tex = input.Tex;
#if DEBUG_SHADOW_MAP
	matrix LightWVP = mul(LightProjection,mul(LightView, World));
	output.PosInLight = mul(LightWVP, float4(input.Pos,1.0));
#endif
    return output;
}

float4 PS(PixelInput input) : SV_Target
{	
	float4 finalColor = input.Color;
#if !DEBUG_SHADOW_MAP
	finalColor *= GetTextureMain(input.Tex);
#else
	finalColor.xyz = float3(0, 0, CalcShadowFactor(samShadow, txDepthMap, input.PosInLight));
#endif
	return finalColor;
}



