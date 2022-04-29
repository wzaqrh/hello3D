#ifndef SHADOW_H
#define SHADOW_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "PercentageCloserSoftShadow.cginc"
#include "VarianceShadow.cginc"

float CalcShadowFactor(float3 posLight, float3 viewPosLight)
{
#if DEBUG_SHADOW_MAP
	return posLight.y;
#endif

	posLight.xy = posLight.xy * float2(0.5, -0.5) + 0.5;
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
	return PCFShadow(posLight.xy, posLight.z, dz_duv, viewPosLight.z, pcfIn, MIR_PASS_SHADOWMAP(_ShadowMapTexture), sampler_GDepth);
#elif SHADOW_MODE == SHADOW_PCSS
	PCFShadowInput pcfIn;
	pcfIn.lightRadiusUVNearFar = LightRadiusUVNearFar;
	pcfIn.lightDepthParam = LightDepthParam;
	float2 dz_duv = DepthGradient(posLight.xy, posLight.z);
	//float2 dz_duv = float2(0.0, 0.0);
	return PCSSShadow(posLight.xy, posLight.z, dz_duv, viewPosLight.z, pcfIn, MIR_PASS_SHADOWMAP(_ShadowMapTexture), sampler_GDepth);
#elif SHADOW_MODE == SHADOW_VSM
	return VSMShadow(posLight.xy, length(viewPosLight), MIR_PASS_TEX2D(_ShadowMapTexture));
#else
	return 1.0;
#endif
}
#endif