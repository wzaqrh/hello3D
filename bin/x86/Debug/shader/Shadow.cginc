#ifndef SHADOW_H
#define SHADOW_H
#include "Standard.cginc"
#include "PercentageCloserSoftShadow.cginc"

#define SHADOW_RAW 1 
#define SHADOW_PCF_FAST 2
#define SHADOW_PCF 3
#define SHADOW_PCSS 4
#define SHADOW_MODE 4/*SHADOW_PCSS*/

inline float3 UnityCombineShadowcoordComponents(float2 baseUV, float2 deltaUV, float depth, float3 receiverPlaneDepthBias)
{
	float3 uv = float3(baseUV + deltaUV, depth + receiverPlaneDepthBias.z);
	uv.z += dot(deltaUV, receiverPlaneDepthBias.xy);
	return uv;
}
inline float UnitySampleShadowmap_PCF3x3NoHardwareSupport(float3 coord, float3 receiverPlaneDepthBias)
{
    float2 base_uv = coord.xy;
    float2 ts = ShadowMapSize.zw;
	float shadow = 0;
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(-ts.x, -ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(0, -ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(ts.x, -ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(-ts.x, 0), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(0, 0), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(ts.x, 0), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(-ts.x, ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(0, ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(ts.x, ts.y), coord.z, receiverPlaneDepthBias));
    shadow /= 9.0;
	return shadow;
}

float CalcShadowFactor(float3 posLight, float viewZLight)
{
	//if (ShadowMapSize.z == 0) return 1.0;
#if DEBUG_SHADOW_MAP
	return posLight.y;
#endif

#if SHADOW_MODE == SHADOW_RAW
	return MIR_SAMPLE_SHADOW(_ShadowMapTexture, posLight).r;
#elif SHADOW_MODE == SHADOW_PCF_FAST
	float3 dz_duv = float3(DepthGradient(posLight.xy, posLight.z), 0.0);
	return UnitySampleShadowmap_PCF3x3NoHardwareSupport(posLight, dz_duv);
#elif SHADOW_MODE == SHADOW_PCF
	PCFShadowInput pcfIn;
	pcfIn.lightRadiusUVNearFar = LightRadiusUVNearFar;
	pcfIn.lightDepthParam = LightDepthParam;
	//float2 dz_duv = DepthGradient(posLight.xy, posLight.z);
	float2 dz_duv = float2(0.0, 0.0);
	return PCFShadow(posLight.xy, posLight.z, dz_duv, viewZLight, pcfIn, MIR_PASS_SHADOWMAP(_ShadowMapTexture), sampler_GDepth);
#elif SHADOW_MODE == SHADOW_PCSS
	PCFShadowInput pcfIn;
	pcfIn.lightRadiusUVNearFar = LightRadiusUVNearFar;
	pcfIn.lightDepthParam = LightDepthParam;
	//float2 dz_duv = DepthGradient(posLight.xy, posLight.z);
	float2 dz_duv = float2(0.0, 0.0);
	return PCSSShadow(posLight.xy, posLight.z, dz_duv, viewZLight, pcfIn, MIR_PASS_SHADOWMAP(_ShadowMapTexture), sampler_GDepth);
#else
	return 1.0;
#endif
}
#endif