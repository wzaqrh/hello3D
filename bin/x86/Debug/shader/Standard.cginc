#ifndef STANDARD_H
#define STANDARD_H
#include "HLSLSupport.cginc"
#include "Macros.cginc"

#if LIGHTMODE == LIGHTMODE_POSTPROCESS
	MIR_DECLARE_TEX2D(_SceneImage, 8);
#else
	MIR_DECLARE_TEX2D(_LightMap, 8);
#endif

#if SHADOW_MODE == SHADOW_VSM
	MIR_DECLARE_TEX2D(_ShadowMap, 12);
#else
	MIR_DECLARE_SHADOWMAP(_ShadowMap, 12);
#endif

MIR_DECLARE_TEXCUBE(_DiffuseCube, 13);
MIR_DECLARE_TEXCUBE(_SpecCube, 14);
MIR_DECLARE_TEX2D(_LUT, 15);

#if LIGHTMODE == LIGHTMODE_PREPASS_FINAL || LIGHTMODE == LIGHTMODE_PREPASS_FINAL_ADD
	MIR_DECLARE_TEX2D(_GDepth, 0);
	MIR_DECLARE_TEX2D(_GBufferPos, 1);
	MIR_DECLARE_TEX2D(_GBufferNormal, 2);
	MIR_DECLARE_TEX2D(_GBufferAlbedo, 3);
	MIR_DECLARE_TEX2D(_GBufferEmissive, 4);
#endif

struct vbSurface
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
	float2 Tex  : TEXCOORD0;
};

cbuffer cbPerLight : register(b1)
{
	float4 LightPosition;	//world space
	float4 LightColor;	//w(gross or luminance), 
	float4 unity_LightAtten;	//x(cutoff), y(1/(1-cutoff)), z(atten^2)
	float4 unity_SpotDirection;
	float4 LightRadiusUVNearFar;
	float4 LightDepthParam;
	bool IsSpotLight;
}

cbuffer cbPerFrame : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	matrix ViewInv;
	matrix ProjectionInv;

	matrix LightView;
	matrix LightProjection;
	
	float4 CameraPositionExposure;
	
	matrix SHC0C1;
	matrix SHC2;
	float4 SHC2_2;
	float4 EnvDiffuseColor;
	float4 EnvSpecColorMip;
	float4 LightMapUV;
	float4 LightMapSizeMip;
	
	float4 FrameBufferSize;
	float4 ShadowMapSize;
}
#endif