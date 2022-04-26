#ifndef SHADOW_H
#define SHADOW_H
#include "Standard.cginc"
#include "PercentageCloserSoftShadow.cginc"

#define SHADOW_RAW 1 
#define SHADOW_PCF_FAST 2
#define SHADOW_PCF 3
#define SHADOW_PCSS 4
#define SHADOW_MODE 4/*SHADOW_PCSS*/

float CalcShadowFactor(float3 posLight, float viewZLight)
{
#if DEBUG_SHADOW_MAP
	return posLight.y;
#endif

#if SHADOW_MODE == SHADOW_RAW
	return MIR_SAMPLE_SHADOW(_ShadowMapTexture, posLight).r;
#elif SHADOW_MODE == SHADOW_PCF_FAST
	float3 dz_duv = float3(DepthGradient(posLight.xy, posLight.z), 0.0);
	return FastPCFShadow(posLight, dz_duv, ShadowMapSize.zw, MIR_PASS_SHADOWMAP(_ShadowMapTexture));
#elif SHADOW_MODE == SHADOW_PCF
	PCFShadowInput pcfIn;
	pcfIn.lightRadiusUVNearFar = LightRadiusUVNearFar;
	pcfIn.lightDepthParam = LightDepthParam;
	float2 dz_duv = DepthGradient(posLight.xy, posLight.z);
	//float2 dz_duv = float2(0.0, 0.0);
	return PCFShadow(posLight.xy, posLight.z, dz_duv, viewZLight, pcfIn, MIR_PASS_SHADOWMAP(_ShadowMapTexture), sampler_GDepth);
#elif SHADOW_MODE == SHADOW_PCSS
	PCFShadowInput pcfIn;
	pcfIn.lightRadiusUVNearFar = LightRadiusUVNearFar;
	pcfIn.lightDepthParam = LightDepthParam;
	float2 dz_duv = DepthGradient(posLight.xy, posLight.z);
	//float2 dz_duv = float2(0.0, 0.0);
	return PCSSShadow(posLight.xy, posLight.z, dz_duv, viewZLight, pcfIn, MIR_PASS_SHADOWMAP(_ShadowMapTexture), sampler_GDepth);
#else
	return 1.0;
#endif
}
#endif