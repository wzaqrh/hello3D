/********** Multi Light(Direct Point) (world space) (SpecularMap) **********/
#include "Standard.h"
#include "Skeleton.h"

Texture2D txDiffuse : register(t0);
Texture2D txSpecular : register(t1);
Texture2D txNormal : register(t2);

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
	float3 Normal : NORMAL;//world space
	float3 Eye : POSITION0;//world space
	float3 PointLights[MAX_LIGHTS] : POSITION1;//world space
	float3 DirectLights[MAX_LIGHTS] : POSITION5;//world space
};

PS_INPUT VS(VS_INPUT i)
{
	PS_INPUT output = (PS_INPUT)0;

	matrix MW = mul(World, transpose(Model));
	matrix MWV = mul(View, MW);
	
	output.Normal = normalize(mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Normal.xyz, 0.0))).xyz);
	output.Pos = mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0)));	
	for (int j = 0; j < LightNum.x; ++j) {
		output.DirectLights[j] = normalize(-DirectLights[j].LightPos.xyz);	
	}
	for (int j = 0; j < LightNum.y; ++j) {
		output.PointLights[j] = normalize(PointLights[j].LightPos.xyz - output.Pos.xyz);
	}
	
	output.Pos = mul(View, output.Pos);
	output.Eye = mul(ViewInv, float4(0.0,0.0,0.0,1.0)) - output.Pos;
	
    output.Pos = mul(Projection, output.Pos);
    
	output.Tex = i.Tex;
    return output;
}

float3 GetDiffuseBySampler(float3 normal, float3 light, float3 lightDiffuseColor, float2 texcoord) {
	float diffuseFactor = saturate(dot(normal, light));
	float3 diffuseMat = txDiffuse.Sample(samLinear, texcoord).xyz;
	return diffuseMat * diffuseFactor * lightDiffuseColor;
}

float3 GetSpecularByDef(float3 normal, float3 light, float3 eye, float4 SpecColorPower) {
	float3 reflection = reflect(-light, normal);
	float specularFactor = saturate(dot(reflection, eye));
	specularFactor = pow(specularFactor, SpecColorPower.w);
	return specularFactor * SpecColorPower.xyz;
}
float3 GetSpecularBySampler(float3 normal, float3 light, float3 eye, float4 SpecColorPower, float2 texcoord) {
	float3 specularMat = txSpecular.Sample(samLinear, texcoord).xyz;
	return GetSpecularByDef(normal, light, eye, SpecColorPower) * specularMat;
}

float3 CalPointLight(float3 normal, float3 light, float3 eye, float2 texcoord, float4 DiffuseColor, float4 SpecularColorPower) {
	float3 color = 0.0;
	if (dot(normal, light) > 0.0) 
	{
		float3 diffuse = GetDiffuseBySampler(normal, light, DiffuseColor.xyz, texcoord);
		float3 specular = GetSpecularBySampler(normal, light, eye, SpecularColorPower, texcoord);
		color = diffuse + specular;	
	}
	return color;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float3 normal = normalize(input.Normal);
	float3 eye = normalize(input.Eye);
	
	float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
	for (int i = 0; i < LightNum.x; ++i) {
		float3 light = normalize(input.DirectLights[i]);
		finalColor.xyz += CalPointLight(normal, light, eye, input.Tex, DirectLights[i].DiffuseColor, DirectLights[i].SpecularColorPower);
	}
	for (int i = 0; i < LightNum.y; ++i) {
		float3 light = normalize(input.PointLights[i]);
		finalColor.xyz += CalPointLight(normal, light, eye, input.Tex, PointLights[i].DiffuseColor, PointLights[i].SpecularColorPower);
	}
	
	//float dir = -eye.z;
	//float dir = -normal.z;
	//float dir = -input.PointLights[0].z;
	//finalColor = float4(dir, dir, dir, 1.0);
	
	//finalColor = float4(reflect(-input.PointLights[0], normal), 1.0);
	//float a = dot(input.PointLights[0], normal);
	//finalColor = float4(a, a, a, 1.0);
	
	//finalColor = float4(normal.x, normal.y, -normal.z, 1.0);
	//finalColor = float4(input.PointLights[0].x, input.PointLights[0].y, -input.PointLights[0].z, 1.0);
	
	return finalColor;
}



