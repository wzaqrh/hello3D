/********** Multi Light(Direct Point) (eye space) (SpecularMap) **********/
#include "Standard.h"
#include "Skeleton.h"
#include "Lighting.h"

MIR_DECLARE_TEX2D(txAlbedo, 0);

inline float3 GetAlbedo(float2 uv) 
{
	float3 albedo;
	if (hasAlbedo) albedo = MIR_SAMPLE_TEX2D(txAlbedo, uv).rgb;
	else albedo = float3(1.0,1.0,1.0);
	return albedo;
}

/************ ShadowCaster ************/
struct ShadowCasterPixelInput
{
    float4 Pos : SV_POSITION;
};

ShadowCasterPixelInput VSShadowCaster(vbSurface surf, vbWeightedSkin skin)
{
	ShadowCasterPixelInput output;
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos, 1.0));
	matrix MW = mul(World, transpose(Model));
	matrix MWV = mul(View, MW);
	matrix MWVP = mul(Projection, MWV);
	output.Pos = mul(MWVP, skinPos);
	return output;
}

float4 PSShadowCaster(ShadowCasterPixelInput input) : SV_Target
{
	return float4(0.467,0.533,0.6,1);
}

/************ ForwardBase ************/
struct PixelInput
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL0;//eye space
	float3 ToEye  : TEXCOORD1;//eye space
	float3 ToLight : TEXCOORD2;//eye space
	float4 PosInLight : TEXCOORD3;//light's ndc space
};

PixelInput VS(vbSurface surf, vbWeightedSkin skin)
{
	PixelInput output = (PixelInput)0;
	matrix MW = mul(World, transpose(Model));
	
	//Normal
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal = normalize(mul(mul(View, MW), skinNormal).xyz);
	
	//ToLight
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);
	output.ToLight = unity_LightPosition.xyz - output.Pos.xyz * unity_LightPosition.w;
	
	//PosInLight
	output.PosInLight = mul(LightView, output.Pos);
	output.PosInLight = mul(LightProjection, output.PosInLight);
	float bias = max(0.05 * (1.0 - dot(output.Normal, output.ToLight)), 0.005);
	output.PosInLight.z -= bias * output.PosInLight.w;
	
	//ToEye
	output.Pos = mul(View, output.Pos);
	output.ToEye = -output.Pos;
	
	//Pos
    output.Pos = mul(Projection, output.Pos);
	output.Tex = surf.Tex;
    return output;
}

float4 PS(PixelInput input) : SV_Target
{	
	float4 finalColor;
	finalColor.rgb = MirBlinnPhongLight(input.ToLight, normalize(input.Normal), normalize(input.ToEye), GetAlbedo(input.Tex), IsSpotLight);
	//finalColor.rgb *= CalcShadowFactor(input.PosInLight);
	finalColor.a = 1.0;
	return finalColor;
}

/************ ForwardAdd ************/
float4 PSAdd(PixelInput input) : SV_Target
{	
	float4 finalColor;
	finalColor.rgb = MirBlinnPhongLight(input.ToLight, normalize(input.Normal), normalize(input.ToEye), GetAlbedo(input.Tex), IsSpotLight);
	finalColor.a = 1.0;
	return finalColor;
}