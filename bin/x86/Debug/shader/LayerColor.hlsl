/********** Multi Light(Direct Point Spot) (eye space) (SpecularMap NormalMapping) **********/
#include "Standard.cginc"
#include "Lighting.cginc"

/************ ForwardBase ************/
struct PixelInput
{
    float4 Pos : SV_POSITION;
	float4 Color : COLOR;
#if ENABLE_SHADOW_MAP
	float4 PosInLight : TEXCOORD0;//light's ndc space
#endif
};

PixelInput VS(vbSurface input)
{
	PixelInput output;
	matrix WVP = mul(Projection, mul(View, World));
    output.Pos = mul(WVP, float4(input.Pos,1.0));
	output.Color = input.Color;
#if ENABLE_SHADOW_MAP
	matrix LightWVP = mul(LightProjection, mul(LightView, World));
	output.PosInLight = mul(LightWVP, float4(input.Pos, 1.0));
	
	float bias = 0.005;
	output.PosInLight.z -= bias * output.PosInLight.w;
#endif
    return output;
}

float4 PS(PixelInput input) : SV_Target
{	
	float4 finalColor = input.Color;
#if DEBUG_SHADOW_MAP
	finalColor.xyz = float3(0, 0, CalcShadowFactor(input.PosInLight));
#elif ENABLE_SHADOW_MAP
	finalColor.xyz *= CalcShadowFactor(input.PosInLight);
#endif
	return finalColor;
}

/************ ShadowCaster ************/
struct PSShadowCasterInput 
{
	float4 Pos  : SV_POSITION;
	float4 Pos0 : POSITION0;
};

PSShadowCasterInput VSShadowCaster(vbSurface input)
{
	PSShadowCasterInput output;
	matrix WVP = mul(Projection, mul(View, World));
    output.Pos = mul(WVP, float4(input.Pos, 1.0));
	output.Pos0 = output.Pos;
	return output;
}

float4 PSShadowCasterDebug(PSShadowCasterInput input) : SV_Target
{
	float4 finalColor = float4(0,0,0,1);
	finalColor.xy = input.Pos0.xy / input.Pos0.w;
	finalColor.xy = finalColor.xy * 0.5 + 0.5;
	finalColor.y = 0;
	return finalColor;
}