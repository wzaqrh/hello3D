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

SamplerState samLinear : register(s0);
SamplerState samAnsp   : register(s1);
SamplerState samPoint  : register(s2);
SamplerState samShadow : register(s3) {
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;
    AddressU = Clamp;
    AddressV = Clamp;	
};

Texture2D txDepthMap : register(t8);
TextureCube txSkybox : register(t9);

//implements
#define SHADOW_EPSILON 0.00005f
#define SMAP_SIZE 512
float CalLightStrengthWithShadow(float4 posInLight)
{
    float2 projPosInLight = 0.5 * posInLight.xy / posInLight.w + float2(0.5, 0.5);
    projPosInLight.y = 1.0f - projPosInLight.y;
	
	float LightAmount;
#if 0
	float depthInLight = posInLight.z / posInLight.w - SHADOW_EPSILON;
	float depthMap = txDepthMap.Sample(samShadow, projPosInLight).r;
	if (depthInLight < depthMap) {
		LightAmount = 1.0;
	}
	else {
		LightAmount = 0.0;
	}
#else
	if (HasDepthMap > 0) {
		float depthInLight = posInLight.z / posInLight.w - SHADOW_EPSILON;
		
		float2 texelpos = SMAP_SIZE * projPosInLight;
		
		// Determine the lerp amounts           
		float2 lerps = frac(texelpos);

		//read in bilerp stamp, doing the shadow checks
		float sourcevals[4];
		sourcevals[0] = (txDepthMap.Sample(samShadow, projPosInLight) < depthInLight) ? 0.0f : 1.0f;  
		sourcevals[1] = (txDepthMap.Sample(samShadow, projPosInLight + float2(1.0/SMAP_SIZE, 0) ) < depthInLight) ? 0.0f : 1.0f;  
		sourcevals[2] = (txDepthMap.Sample(samShadow, projPosInLight + float2(0, 1.0/SMAP_SIZE) ) < depthInLight) ? 0.0f : 1.0f;  
		sourcevals[3] = (txDepthMap.Sample(samShadow, projPosInLight + float2(1.0/SMAP_SIZE, 1.0/SMAP_SIZE) ) < depthInLight) ? 0.0f : 1.0f;  
		
		// lerp between the shadow values to calculate our light amount
		LightAmount = lerp(lerp(sourcevals[0], sourcevals[1], lerps.x),
						   lerp(sourcevals[2], sourcevals[3], lerps.x),
						   lerps.y);
	}
	else {
		LightAmount = 1.0;
	}
#endif
	return LightAmount;	
}