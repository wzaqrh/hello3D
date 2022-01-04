#ifndef STANDARD_H
#define STANDARD_H
#include "HLSLSupport.cginc"

MIR_DECLARE_SHADOWMAP(_ShadowMapTexture, 8);
MIR_DECLARE_TEXCUBE(_DiffuseCube, 9);
MIR_DECLARE_TEXCUBE(_SpecCube, 10);
MIR_DECLARE_TEX2D(_LUT, 11);

MIR_DECLARE_TEX2D(_GBufferPos, 13);
MIR_DECLARE_TEX2D(_GBufferNormal, 14);
MIR_DECLARE_TEX2D(_GBufferAlbedo, 15);

struct vbSurface
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
	float2 Tex  : TEXCOORD0;
};

cbuffer cbPerLight : register(b1)
{
	float4 unity_LightPosition;	//world space
	float4 unity_LightColor;	//w(gross or luminance), 
	float4 unity_SpecColor;		//w(shiness)
	float4 unity_LightAtten;	//x(cutoff), y(1/(1-cutoff)), z(atten^2)
	float4 unity_SpotDirection;

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
	
	float4 glstate_lightmodel_ambient;
	float4 _ShadowMapTexture_TexelSize;
}
#endif