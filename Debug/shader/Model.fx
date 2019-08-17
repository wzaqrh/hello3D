/********** Multi Light(Direct Point) (eye space) (SpecularMap) **********/
#include "Standard.h"
#include "Skeleton.h"

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
	int4  BlendIndices : BLENDINDICES;
	float3 BiTangent : NORMAL2;
};

/************ ShadowCaster ************/
struct SHADOW_PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 Depth : TEXCOORD0;
};

SHADOW_PS_INPUT VSShadowCaster( VS_INPUT i)
{
	SHADOW_PS_INPUT output;
	float4 skinPos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0));
	matrix MW = mul(World, transpose(Model));
	matrix MWVP = mul(Projection, mul(View, MW));
	output.Pos = mul(MWVP, skinPos);
	output.Depth = output.Pos;	
	return output;
}

float4 PSShadowCaster(SHADOW_PS_INPUT i) : SV_Target
{
	float depthValue = i.Depth.z / i.Depth.w;
	float4 finalColor = depthValue;
	return finalColor;
}

/************ ForwardBase ************/
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	
	float3 Normal : NORMAL0;//eye space
	float3 Eye : TEXCOORD1;//eye space
	float3 ToLight : TEXCOORD2;//eye space
	float4 PosInLight : TEXCOORD3;//world space
};

PS_INPUT VS(VS_INPUT i)
{
	PS_INPUT output = (PS_INPUT)0;

	matrix MW = mul(World, transpose(Model));
	matrix MWV = mul(View, MW);
	
	float4 skinNormal = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Normal.xyz, 0.0));
	output.Normal = normalize(mul(MWV, skinNormal).xyz);
	
	float4 skinPos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0));
	output.Pos = mul(MWV, skinPos);
	
	if (LightType == 1) {
		output.ToLight = normalize(mul(View, float4(-Light.Base.Base.LightPos.xyz,0.0)));	
	}
	else if (LightType == 2 || LightType == 3) {
		output.ToLight = mul(View,float4(Light.Base.Base.LightPos.xyz,1.0)).xyz - output.Pos.xyz;
	}
	
	matrix LightMWVP = mul(LightProjection,mul(LightView, MW));
	output.PosInLight = mul(LightMWVP, skinPos);
	
	output.Eye = -output.Pos;
    output.Pos = mul(Projection, output.Pos);
	output.Tex = i.Tex;
    return output;
}

float3 GetDiffuseBySampler(float3 normal, float3 light, float3 lightDiffuseColor, float2 texcoord) {
	float diffuseFactor = saturate(dot(normal, light));
	float3 diffuseMat = GetTexture2D(txMain, samLinear, texcoord).xyz;
	return diffuseMat * diffuseFactor * lightDiffuseColor;
}
float3 GetSpecularByDef(float3 normal, float3 light, float3 eye, float4 SpecColorPower) {
	float3 reflection = reflect(-light, normal);
	float specularFactor = saturate(dot(reflection, eye));
	specularFactor = pow(specularFactor, SpecColorPower.w);
	return specularFactor * SpecColorPower.xyz;
}
float3 GetSpecularBySampler(float3 normal, float3 light, float3 eye, float4 SpecColorPower, float2 texcoord) {
	float3 specularMat = GetTexture2D(txSpecular, samLinear, texcoord).xyz;
	return GetSpecularByDef(normal, light, eye, SpecColorPower) * specularMat;
}

float3 CalDirectLight(LIGHT_DIRECT directLight, float3 normal, float3 light, float3 eye, float2 texcoord) {
	float3 color = 0.0;
	if (dot(normal, light) > 0.0) 
	{
		float3 diffuse = GetDiffuseBySampler(normal, light, directLight.DiffuseColor.xyz, texcoord);
		float3 specular = GetSpecularBySampler(normal, light, eye, directLight.SpecularColorPower, texcoord);
		color = diffuse + specular;	
	}
	return color;
}
float3 CalPointLight(LIGHT_POINT pointLight, float3 normal, float3 light, float3 eye, float2 texcoord, float Distance) {
	float3 color = CalDirectLight(pointLight.Base, normal, light, eye, texcoord);
	float Attenuation = pointLight.Attenuation.x 
	+ pointLight.Attenuation.y * Distance 
	+ pointLight.Attenuation.z * Distance * Distance;
	return color / Attenuation;
}
float3 CalSpotLight(LIGHT_SPOT spotLight, float3 normal, float3 light, float3 eye, float2 texcoord, float Distance, float3 spotDirection) {
	float3 color = 0.0;
	float spotFactor = dot(light, spotDirection);
	if (spotFactor > spotLight.DirectionCutOff.w) {
		color = CalPointLight(spotLight.Base, normal, light, eye, texcoord, Distance);
        color = color * ((spotFactor - spotLight.DirectionCutOff.w) / (1.0 - spotLight.DirectionCutOff.w));
	}
	return color;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float3 normal = normalize(input.Normal);
	float3 eye = normalize(input.Eye);
	
	float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
	
	if (LightType == 1) {
		float3 light = normalize(input.ToLight);
		finalColor.xyz += CalDirectLight(Light.Base.Base, normal, light, eye, input.Tex);
	}
	else if (LightType == 2) {
		float Distance = length(input.ToLight);
		float3 light = normalize(input.ToLight);
		finalColor.xyz += CalPointLight(Light.Base, normal, light, eye, input.Tex, Distance);
	}
	else if (LightType == 3) {
		float Distance = length(input.ToLight);
		float3 light = normalize(input.ToLight);
		float3 spotDirection = normalize(-Light.DirectionCutOff.xyz.xyz);
		finalColor.xyz += CalSpotLight(Light, normal, light, eye, input.Tex, Distance, spotDirection);
	}
	
	finalColor.rgb = finalColor.rgb * CalLightStrengthWithShadow(input.PosInLight);
	
	return finalColor;
}

/************ ForwardAdd ************/
float4 PSAdd(PS_INPUT input) : SV_Target
{	
	float3 normal = normalize(input.Normal);
	float3 eye = normalize(input.Eye);
	
	float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
	
	if (LightType == 1) {
		float3 light = normalize(input.ToLight);
		finalColor.xyz += CalDirectLight(Light.Base.Base, normal, light, eye, input.Tex);
	}
	else if (LightType == 2) {
		float Distance = length(input.ToLight);
		float3 light = normalize(input.ToLight);
		finalColor.xyz += CalPointLight(Light.Base, normal, light, eye, input.Tex, Distance);
	}
	else if (LightType == 3) {
		float Distance = length(input.ToLight);
		float3 light = normalize(input.ToLight);
		float3 spotDirection = normalize(-Light.DirectionCutOff.xyz.xyz);
		finalColor.xyz += CalSpotLight(Light, normal, light, eye, input.Tex, Distance, spotDirection);
	}
	
	finalColor.rgb = finalColor.rgb * CalLightStrengthWithShadow(input.PosInLight);
	
	return finalColor;
}