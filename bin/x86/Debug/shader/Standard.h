#include "HLSLSupport.h"

struct vbSurface
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
	float2 Tex  : TEXCOORD0;
};

cbuffer cbPerLight : register(b1)
{
	matrix LightView;
	matrix LightProjection;	

	float4 unity_LightPosition;	//world space
	float4 unity_LightColor;	//w(gross or luminance), 
	float4 unity_SpecColor;		//w(shiness)
	float4 unity_LightAtten;	//x(cutoff), y(1/(1-cutoff)), z(atten^2)
	float4 unity_SpotDirection;

	bool IsSpotLight;
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
	bool HasDepthMap;
}

#if SHADER_MODEL > 30000
SamplerState samLinear : register(s0);
SamplerState samAnsp   : register(s1);
SamplerState samPoint  : register(s2);
SamplerComparisonState samShadow : register(s8);

Texture2D txMain : register(t0);
Texture2D txDepthMap : register(t8);
TextureCube txSkybox : register(t9);

#define GetTexture2D(TEX, SAMPLER, COORD) TEX.Sample(SAMPLER, COORD)
#define GetTextureCube(TEX, SAMPLER, COORD) TEX.Sample(SAMPLER, COORD)
#define GetTextureCubeLevel(TEX, SAMPLER, COORD, LOD) TEX.SampleLevel(SAMPLER, COORD, LOD)
#else
texture  textureMain : register(t0);
sampler2D txMain : register(s0);

texture  textureDepthMap : register(t8);
sampler2D txDepthMap : register(s8);

texture  textureSkybox : register(t9);
samplerCUBE txSkybox : register(s9);

#define GetTexture2D(TEX, SAMPLER, COORD) tex2D(TEX, COORD)
#define GetTextureCube(TEX, SAMPLER, COORD) texCUBE(TEX, COORD)
#define GetTextureCubeLevel(TEX, SAMPLER, COORD, LOD) texCUBElod(TEX, float4(COORD,LOD)) 
#endif
float4 GetTextureMain(float2 inputTex) {
	return GetTexture2D(txMain, samLinear, inputTex);
}