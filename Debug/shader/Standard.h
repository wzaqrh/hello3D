struct LIGHT_DIRECT
{
	float4 LightPos;//world space
	float4 DiffuseColor;
	float4 SpecularColorPower;
};

struct LIGHT_POINT
{
	LIGHT_DIRECT L;
	float4 Attenuation;
};

struct LIGHT_SPOT
{
	LIGHT_POINT Base;
	float3 Direction;
    float Cutoff;
};

static const int MAX_LIGHTS = 4;
cbuffer cbGlobalParam : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	
	matrix WorldInv;
	matrix ViewInv;
	matrix ProjectionInv;
	
	int4 LightNum;
	LIGHT_DIRECT DirectLights[MAX_LIGHTS];
	LIGHT_POINT PointLights[MAX_LIGHTS];
	LIGHT_SPOT SpotLights[MAX_LIGHTS];
	
	matrix LightView;
	matrix LightProjection;
	int HasDepthMap;
}