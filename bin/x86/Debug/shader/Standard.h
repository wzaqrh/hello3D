cbuffer cbPerLight : register(b1)
{
	matrix LightView;
	matrix LightProjection;	

	float4 unity_LightPosition;//world space
	float4 unity_LightColor;
	float4 SpecularColorPower;
	float4 unity_LightAtten;
	float4 unity_SpotDirection;

	int LightType;
	int HasDepthMap;
}

cbuffer cbGlobalParam : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	
	matrix WorldInv;
	matrix ViewInv;
	matrix ProjectionInv;

	float4 glstate_lightmodel_ambient;
}

#if SHADER_MODEL > 30000
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

Texture2D txMain : register(t0);
Texture2D txDepthMap : register(t8);
TextureCube txSkybox : register(t9);

#define GetTexture2D(TEX, SAMPLER, COORD) TEX.Sample(SAMPLER, COORD)
#define GetTextureCube(TEX, SAMPLER, COORD) TEX.Sample(SAMPLER, COORD)
#define GetTextureCubeLevel(TEX, SAMPLER, COORD, LOD) TEX.SampleLevel(SAMPLER, COORD, LOD)
#else
texture  textureMain : register(t0);
sampler2D txMain : register(s0) =
sampler_state
{
    Texture = <textureMain>;
};

texture  textureDepthMap : register(t8);
sampler2D txDepthMap : register(s8) =
sampler_state
{
    Texture = <textureDepthMap>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;
	AddressU = Clamp;
	AddressV = Clamp;
};

texture  textureSkybox : register(t9);
samplerCUBE txSkybox : register(s9) =
sampler_state
{
    Texture = <textureSkybox>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

#define GetTexture2D(TEX, SAMPLER, COORD) tex2D(TEX, COORD)
#define GetTextureCube(TEX, SAMPLER, COORD) texCUBE(TEX, COORD)
#define GetTextureCubeLevel(TEX, SAMPLER, COORD, LOD) texCUBElod(TEX, float4(COORD,LOD)) 
#endif
float4 GetTextureMain(float2 inputTex) {
	return GetTexture2D(txMain, samLinear, inputTex);
}
float GetTextureDepthMap(float2 inputTex) {
	return GetTexture2D(txDepthMap, samShadow, inputTex);
}

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
		sourcevals[0] = (GetTextureDepthMap(projPosInLight) < depthInLight) ? 0.0f : 1.0f;  
		sourcevals[1] = (GetTextureDepthMap(projPosInLight + float2(1.0/SMAP_SIZE, 0)) < depthInLight) ? 0.0f : 1.0f;  
		sourcevals[2] = (GetTextureDepthMap(projPosInLight + float2(0, 1.0/SMAP_SIZE)) < depthInLight) ? 0.0f : 1.0f;  
		sourcevals[3] = (GetTextureDepthMap(projPosInLight + float2(1.0/SMAP_SIZE, 1.0/SMAP_SIZE)) < depthInLight) ? 0.0f : 1.0f;  
		
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