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

MIR_DECLARE_SHADOWMAP(txDepthMap, 8);
MIR_DECLARE_TEX2D(txSkybox, 9);