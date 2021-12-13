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
	float3 ToLight : TEXCOORD2;//world space
#if ENABLE_SHADOW_MAP
	float4 PosInLight : TEXCOORD3;//light's ndc space
#endif
};

PixelInput VS(vbSurface surf, vbWeightedSkin skin)
{
	PixelInput output;
	matrix MW = mul(World, transpose(Model));
	
	//Normal
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal = normalize(mul(mul(View, MW), skinNormal).xyz);
	
	//ToLight
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);
	output.ToLight = unity_LightPosition.xyz - output.Pos.xyz * unity_LightPosition.w;
	
#if ENABLE_SHADOW_MAP
	//PosInLight
	output.PosInLight = mul(LightView, output.Pos);
	output.PosInLight = mul(LightProjection, output.PosInLight);
	float bias = max(0.05 * (1.0 - dot(output.Normal, output.ToLight)), 0.005);
	output.PosInLight.z -= bias * output.PosInLight.w;
#endif
	
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
	finalColor.a = 1.0;
#if ENABLE_SHADOW_MAP
	finalColor.rgb *= CalcShadowFactor(input.PosInLight);
#endif
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

/************ PrepassBase ************/
struct PSPrepassBaseInput
{
    float4 Pos : POSITION;//world space
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL0;
};
PSPrepassBaseInput VSPrepassBase(vbSurface surf, vbWeightedSkin skin)
{
	PSPrepassBaseInput output;
	matrix MW = mul(World, transpose(Model));
	
	//Normal
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal = mul(MW, skinNormal).xyz;
	
	//Pos
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);
	
	//Tex
	output.Tex = surf.Tex;
    return output;
}

struct PSPrepassBaseOutput
{
	float4 Pos : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Albedo : SV_Target2;
};
PSPrepassBaseOutput PSPrepassBase(PSPrepassBaseInput input)
{
	PSPrepassBaseOutput output;
	output.Pos = input.Pos / input.Pos.w;

	output.Normal.rgb = normalize(input.Normal);
	output.Normal.w = 1.0;
	
	output.Albedo.rgb = GetAlbedo(input.Tex);
	output.Albedo.w = 1.0;
	return output;
}

/************ PrepassFinal ************/
struct PSPrepassFinalInput
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

PSPrepassFinalInput VSPrepassFinal(vbSurface input)
{
	PSPrepassFinalInput output;
	matrix WVP = mul(Projection, mul(View, World));
    output.Pos = mul(WVP, float4(input.Pos, 1.0));
	output.Tex = input.Tex;
    return output;
}

float4 PSPrepassFinal(PSPrepassFinalInput input) : SV_Target
{
	float4 finalColor;
	
	float4 position = MIR_SAMPLE_TEX2D(_GBufferPos, input.Tex);
	float3 normal = normalize(MIR_SAMPLE_TEX2D(_GBufferNormal, input.Tex).rgb);
	float3 albedo = MIR_SAMPLE_TEX2D(_GBufferAlbedo, input.Tex).rgb;
	
	float3 toLight_ = unity_LightPosition.xyz - position.xyz * unity_LightPosition.w;
	float3 toEye = normalize(-mul(View, position.xyz));
	finalColor.rgb = MirBlinnPhongLight(toLight_, normal, toEye, albedo, IsSpotLight);
	finalColor.a = 1.0;
	
#if ENABLE_SHADOW_MAP
	//PosInLight
	float4 posInLight = mul(LightView, position);
	posInLight = mul(LightProjection, posInLight);
	float bias = max(0.05 * (1.0 - dot(normal, normalize(toLight_))), 0.005);
	posInLight.z -= bias * posInLight.w;
	
	finalColor.rgb *= CalcShadowFactor(posInLight);
#endif
	return finalColor;
}