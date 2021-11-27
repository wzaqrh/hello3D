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
		output.ToLight = normalize(mul(View, float4(-LightPos.xyz,0.0)));
	}
	else if (LightType == 2 || LightType == 3) {
		output.ToLight = mul(View,float4(LightPos.xyz,1.0)).xyz - output.Pos.xyz;
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

float3 CalDirectLight(float3 normal, float3 light, float3 eye, float2 texcoord) {
	float3 color = 0.0;
	if (dot(normal, light) > 0.0) 
	{
		float3 diffuse = GetDiffuseBySampler(normal, light, DiffuseColor.xyz, texcoord);
		float3 specular = GetSpecularBySampler(normal, light, eye, SpecularColorPower, texcoord);
		color = diffuse + specular;	
	}
	return color;
}
float3 CalPointLight(float3 normal, float3 light, float3 eye, float2 texcoord, float Distance) {
	float3 color = CalDirectLight(normal, light, eye, texcoord);
	float attenuation = Attenuation.x
	+ Attenuation.y * Distance
	+ Attenuation.z * Distance * Distance;
	return color / attenuation;
}
float3 CalSpotLight(float3 normal, float3 light, float3 eye, float2 texcoord, float Distance, float3 spotDirection) {
	float3 color = 0.0;
	float spotFactor = dot(light, spotDirection);
	if (spotFactor > DirectionCutOff.w) {
		color = CalPointLight(normal, light, eye, texcoord, Distance);
        color = color * ((spotFactor - DirectionCutOff.w) / (1.0 - DirectionCutOff.w));
	}
	return color;
}

#ifdef CPPPP
float3 ShadeVertexLightsFull (float4 vertex, float3 normal, bool spotLight)
{
    float3 viewpos = vertex.xyz;
    float3 viewN = normal;

    float3 lightColor = glstate_lightmodel_ambient.xyz;
    {
        float3 toLight = unity_LightPosition.xyz - viewpos.xyz * unity_LightPosition.w;
        float lengthSq = dot(toLight, toLight);

        // don't produce NaNs if some vertex position overlaps with the light
        lengthSq = max(lengthSq, 0.000001);

        toLight *= rsqrt(lengthSq);

        float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
        if (spotLight)
        {
            float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
            float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
            atten *= saturate(spotAtt);
        }

        float diff = max (0, dot (viewN, toLight));
        lightColor += unity_LightColor.rgb * (diff * atten);
    }
    return lightColor;
}
#endif

float4 PS(PS_INPUT input) : SV_Target
{	
	float3 normal = normalize(input.Normal);
	float3 eye = normalize(input.Eye);
	
	float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
	
	if (LightType == 1) {
		float3 light = normalize(input.ToLight);
		finalColor.xyz += CalDirectLight(normal, light, eye, input.Tex);
	}
	else if (LightType == 2) {
		float Distance = length(input.ToLight);
		float3 light = normalize(input.ToLight);
		finalColor.xyz += CalPointLight(normal, light, eye, input.Tex, Distance);
	}
	else if (LightType == 3) {
		float Distance = length(input.ToLight);
		float3 light = normalize(input.ToLight);
		float3 spotDirection = normalize(-DirectionCutOff.xyz);
		finalColor.xyz += CalSpotLight(normal, light, eye, input.Tex, Distance, spotDirection);
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
		finalColor.xyz += CalDirectLight(normal, light, eye, input.Tex);
	}
	else if (LightType == 2) {
		float Distance = length(input.ToLight);
		float3 light = normalize(input.ToLight);
		finalColor.xyz += CalPointLight(normal, light, eye, input.Tex, Distance);
	}
	else if (LightType == 3) {
		float Distance = length(input.ToLight);
		float3 light = normalize(input.ToLight);
		float3 spotDirection = normalize(-DirectionCutOff.xyz.xyz);
		finalColor.xyz += CalSpotLight(normal, light, eye, input.Tex, Distance, spotDirection);
	}
	
	finalColor.rgb = finalColor.rgb * CalLightStrengthWithShadow(input.PosInLight);
	return finalColor;
}