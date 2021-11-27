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
	
	float3 worldpos = mul(MW, skinPos);
	output.ToLight = unity_LightPosition.xyz - worldpos.xyz * unity_LightPosition.w;
	
	matrix LightMWVP = mul(LightProjection,mul(LightView, MW));
	output.PosInLight = mul(LightMWVP, skinPos);
	
	output.Eye = -output.Pos;
    output.Pos = mul(Projection, output.Pos);
	output.Tex = i.Tex;
    return output;
}

float3 ShadeVertexLightsFull (float3 toLight, float3 viewN, bool spotLight)
{
    float3 lightColor = glstate_lightmodel_ambient.xyz;
    {
        float lengthSq = max(dot(toLight, toLight), 0.000001);
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

float4 PS(PS_INPUT input) : SV_Target
{	
	float4 finalColor;
	finalColor.xyz = ShadeVertexLightsFull(input.ToLight, normalize(input.Normal), LightType == 3);
	finalColor.w = 1.0;
	finalColor *= GetTexture2D(txMain, samLinear, input.Tex);

	finalColor.xyz *= CalLightStrengthWithShadow(input.PosInLight);
	return finalColor;
}

/************ ForwardAdd ************/
float4 PSAdd(PS_INPUT input) : SV_Target
{	
	float4 finalColor;
	finalColor.xyz = ShadeVertexLightsFull(input.ToLight, normalize(input.Normal), LightType == 3);
	finalColor.w = 1.0;
	finalColor *= GetTexture2D(txMain, samLinear, input.Tex);
	
	finalColor.xyz *= CalLightStrengthWithShadow(input.PosInLight);
	return finalColor;
}