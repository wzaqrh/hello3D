/********** Multi Light(Direct Point) (eye space) (SpecularMap) **********/
#include "Standard.h"
#include "Skeleton.h"
#include "Lighting.h"

#if SHADER_MODEL > 30000
Texture2D txSpecular : register(t1);
Texture2D txNormal : register(t2);
#else
texture  textureSpecular : register(t1);
sampler2D txSpecular : register(s1) = sampler_state { Texture = <textureSpecular>; };

texture  textureNormal : register(t2);
sampler2D txNormal : register(s2) = sampler_state { Texture = <textureNormal>; };
#endif

struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : NORMAL1;
	float2 Tex  : TEXCOORD0;
	float4 BlendWeights : BLENDWEIGHT;
	uint4  BlendIndices : BLENDINDICES;
	float3 BiTangent : NORMAL2;
};

/************ ShadowCaster ************/
struct SHADOW_PS_INPUT
{
    float4 Pos : SV_POSITION;
};

SHADOW_PS_INPUT VSShadowCaster( VS_INPUT i)
{
	SHADOW_PS_INPUT output;
	float4 skinPos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos, 1.0));
	matrix MW = mul(World, transpose(Model));
	matrix MWV = mul(View, MW);
	matrix MWVP = mul(Projection, MWV);
	output.Pos = mul(MWVP, skinPos);
	return output;
}

float4 PSShadowCaster(SHADOW_PS_INPUT i) : SV_Target
{
	float z = i.Pos.z / i.Pos.w;
	return float4(0.467,0.533,0.6,1);
}

/************ ForwardBase ************/
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL0;//eye space
	float3 ToEye  : TEXCOORD1;//eye space
	float3 ToLight : TEXCOORD2;//eye space
	float4 PosInLight : TEXCOORD3;//light's ndc space
};

PS_INPUT VS(VS_INPUT i)
{
	PS_INPUT output = (PS_INPUT)0;
	matrix MW = mul(World, transpose(Model));
	
	//Normal
	float4 skinNormal = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Normal.xyz, 0.0));
	output.Normal = normalize(mul(mul(View, MW), skinNormal).xyz);
	
	//ToLight
	float4 skinPos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);
	output.ToLight = unity_LightPosition.xyz - output.Pos.xyz * unity_LightPosition.w;
	
	//PosInLight
	output.PosInLight = mul(MW, skinPos);
	output.PosInLight = mul(LightView, output.PosInLight);
	output.PosInLight = mul(LightProjection, output.PosInLight);
	
	//ToEye
	output.Pos = mul(View, output.Pos);
	output.ToEye = -output.Pos;
	
	//Pos
    output.Pos = mul(Projection, output.Pos);
	output.Tex = i.Tex;
    return output;
}

inline float3 GetAlbedo(float2 uv) {
	float3 albedo;
	if (hasAlbedo) albedo = GetTexture2D(txMain, samLinear, uv).rgb;
	else albedo = float3(1.0,1.0,1.0);	
	return albedo;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float4 finalColor;
	finalColor.rgb = MirBlinnPhongLight(input.ToLight, normalize(input.Normal), normalize(input.ToEye), GetAlbedo(input.Tex), LightType == 3);
	finalColor.a = 1.0;

	//finalColor.rgb *= CalLightStrengthWithShadow(input.PosInLight);
	finalColor.rgb *= CalcShadowFactor(samShadow, txDepthMap, input.PosInLight);
#if 0
	float4 shadowPosH = input.PosInLight;
	shadowPosH.xyz /= shadowPosH.w;
	shadowPosH.xy = shadowPosH.xy * 0.5 + 0.5;
	shadowPosH.y = 1.0 - shadowPosH.y;
	
	finalColor.rgb = txDepthMap.Sample(samLinear, shadowPosH.xy).r;
	//finalColor.rgb *= txDepthMap.SampleCmpLevelZero(samShadow, shadowPosH.xy, shadowPosH.z).r;
#endif
		
	return finalColor;
}

/************ ForwardAdd ************/
float4 PSAdd(PS_INPUT input) : SV_Target
{	
	float4 finalColor;
	finalColor.rgb = MirBlinnPhongLight(input.ToLight, normalize(input.Normal), normalize(input.ToEye), GetAlbedo(input.Tex), LightType == 3);
	finalColor.a = 1.0;
	
	//finalColor.rgb *= CalLightStrengthWithShadow(input.PosInLight);
	return finalColor;
}