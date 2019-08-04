/********** Diffuse Light **********/
#include "Standard.h"
#include "Skeleton.h"

Texture2D txDiffuse : register( t0 );

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
	float3 Tangent : NORMAL1;
	float2 Tex  : TEXCOORD0;
    float4 BlendWeights : BLENDWEIGHT;
    uint4  BlendIndices : BLENDINDICES;
	float3 BiTangent : NORMAL2;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;
	float3 Light : POSITION0;
	float3 Eye : POSITION1;
};

PS_INPUT VS(VS_INPUT i)
{
	PS_INPUT output = (PS_INPUT)0;

	matrix MW = mul(World, transpose(Model));
	matrix MWV = mul(View, MW);

	output.Normal = normalize(mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Normal.xyz, 1.0)))).xyz;
	output.Pos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0));
	output.Pos = mul(MW, output.Pos);
	output.Light = normalize(DirectLights[0].LightPos - output.Pos).xyz;
	
    output.Pos = mul(View, output.Pos);
    output.Pos = mul(Projection, output.Pos);
    
	output.Tex = i.Tex;
    return output;
}

float3 GetDiffuse(float3 normal, float3 light, float2 texcoord) {
	float diffuseFactor = saturate(dot(normal, light));
	float3 diffuseMat = txDiffuse.Sample(samLinear, texcoord).xyz;
	return diffuseMat * diffuseFactor * DirectLights[0].DiffuseColor;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float3 normal = normalize(input.Normal);
	float3 light = normalize(input.Light);
	float3 color = GetDiffuse(normal, light, input.Tex);
	return float4(color, 1.0);
}