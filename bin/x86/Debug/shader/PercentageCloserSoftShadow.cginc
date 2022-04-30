#ifndef PERCENTAGE_CLOSER_SOFT_SHADOW_H
#define PERCENTAGE_CLOSER_SOFT_SHADOW_H
#include "Macros.cginc"
#include "CommonFunction.cginc"

#define BLOCKER_SAMPLE_ALL 0
#define BLOCKER_SAMPLE_POISSON25 1
#define BLOCKER_SAMPLE_POISSON32 2
#define BLOCKER_SAMPLE_POISSON64 3

#define PCF_SAMPLE_ALL 0
#define PCF_SAMPLE_POISSON25 1
#define PCF_SAMPLE_POISSON32 2
#define PCF_SAMPLE_POISSON64 3

#if PCSS_QUALITY == PCSS_QUALITY_LOW
#include "Poisson.cginc"
#define BLOCKER_SAMPLE_MODE BLOCKER_SAMPLE_POISSON25
#define PCF_SAMPLE_MODE BLOCKER_SAMPLE_POISSON25
#elif PCSS_QUALITY == PCSS_QUALITY_MEDIUM
#include "Poisson.cginc"
#define BLOCKER_SAMPLE_MODE BLOCKER_SAMPLE_POISSON32
#define PCF_SAMPLE_MODE BLOCKER_SAMPLE_POISSON64
#elif PCSS_QUALITY == PCSS_QUALITY_HIGH
#define BLOCKER_SAMPLE_MODE 0
#define PCF_SAMPLE_MODE 0
#endif

#if BLOCKER_SAMPLE_MODE == BLOCKER_SAMPLE_POISSON25
#define BLOCKER_POISSON_COUNT 25
#define BLOCKER_POISSON_KERNEL PoissonKernel25
#elif BLOCKER_SAMPLE_MODE == BLOCKER_SAMPLE_POISSON32
#define BLOCKER_POISSON_COUNT 32
#define BLOCKER_POISSON_KERNEL PoissonKernel32
#elif BLOCKER_SAMPLE_MODE == BLOCKER_SAMPLE_POISSON64
#define BLOCKER_POISSON_COUNT 64
#define BLOCKER_POISSON_KERNEL PoissonKernel64
#else
#define BLOCKER_SEARCH_STEP_COUNT 3
#endif

#if PCF_SAMPLE_MODE == PCF_SAMPLE_POISSON25
#define PCF_POISSON_COUNT 25
#define PCF_POISSON_KERNEL PoissonKernel25
#elif PCF_SAMPLE_MODE == PCF_SAMPLE_POISSON32
#define PCF_POISSON_COUNT 32
#define PCF_POISSON_KERNEL PoissonKernel32
#elif PCF_SAMPLE_MODE == PCF_SAMPLE_POISSON64
#define PCF_POISSON_COUNT 64
#define PCF_POISSON_KERNEL PoissonKernel64
#else
#define PCF_FILTER_STEP_COUNT 7
#endif

struct PCFShadowInput
{
	float4 lightRadiusUVNearFar; // g_LightRadiusUV, g_LightZNear, g_LightZFar
	float4 lightDepthParam; // f/(f*n), (f-n)/(f*n), minPCFRadius
};

inline float2 SearchRegionRadiusUV(float zWorld, float4 lightParam)
{
	return lightParam.xy * (zWorld - lightParam.z) / zWorld;
}

void FindBlocker(out float avgBlockerDepth, out float numBlockers,
                float2 uv, float z0, float2 dz_duv, float2 searchRegionRadiusUV,
				MIR_ARGS_TEX2D(tDepth))
{
	float blockerSum = 0;
	numBlockers = 0;

#if BLOCKER_SAMPLE_MODE != 0
	for (int i = 0; i < BLOCKER_POISSON_COUNT; ++i)
	{
		float2 offset = BLOCKER_POISSON_KERNEL[i] * searchRegionRadiusUV;
		float shadowMapDepth = MIR_SAMPLE_TEX2D_LEVEL(tDepth, uv + offset, 0);
		float z = BiasedZ(z0, dz_duv, offset);
		if (shadowMapDepth < z)
		{
			blockerSum += shadowMapDepth;
			numBlockers++;
		}
	}
#else
	float2 stepUV = searchRegionRadiusUV / BLOCKER_SEARCH_STEP_COUNT;
	for (float x = -BLOCKER_SEARCH_STEP_COUNT; x <= BLOCKER_SEARCH_STEP_COUNT; ++x)
		for (float y = -BLOCKER_SEARCH_STEP_COUNT; y <= BLOCKER_SEARCH_STEP_COUNT; ++y)
		{
			float2 offset = float2(x, y) * stepUV;
			float shadowMapDepth = MIR_SAMPLE_TEX2D_LEVEL(tDepth, uv + offset, 0);
			float z = BiasedZ(z0, dz_duv, offset);
			if (shadowMapDepth < z)
			{
				blockerSum += shadowMapDepth;
				numBlockers++;
			}
		}
#endif
	avgBlockerDepth = blockerSum / numBlockers;
}

float PCF_Filter(float2 uv, float z0, float2 dz_duv, float2 filterRadiusUV, 
				 MIR_ARGS_SHADOWMAP(tDepthMap))
{
	float sum = 0;
    
#if PCF_SAMPLE_MODE != 0
    for ( int i = 0; i < PCF_POISSON_COUNT; ++i )
    {
		float2 offset = PCF_POISSON_KERNEL[i] * filterRadiusUV;
        float z = BiasedZ(z0, dz_duv, offset);
		sum += MIR_SAMPLE_SHADOW(tDepthMap, float3(uv + offset, z));
	}
    return sum / PCF_POISSON_COUNT;
#else
	float2 stepUV = filterRadiusUV / PCF_FILTER_STEP_COUNT;
	for (float x = -PCF_FILTER_STEP_COUNT; x <= PCF_FILTER_STEP_COUNT; ++x)
		for (float y = -PCF_FILTER_STEP_COUNT; y <= PCF_FILTER_STEP_COUNT; ++y)
		{
			float2 offset = float2(x, y) * stepUV;
			float z = BiasedZ(z0, dz_duv, offset);
			sum += MIR_SAMPLE_SHADOW(tDepthMap, float3(uv + offset, z));
		}            
	float numSamples = (PCF_FILTER_STEP_COUNT * 2 + 1);
	return sum / (numSamples * numSamples);
#endif
}

float PCFShadow(float2 uv, float z, float2 dz_duv, float zEye, 
				PCFShadowInput input, MIR_ARGS_SHADOWMAP(tDepthMap), SamplerState samplertDepth)
{
#if 1
    // Do a blocker search to enable early out
	float avgBlockerDepth = 0;
	float numBlockers = 0;
	float2 searchRegionRadiusUV = SearchRegionRadiusUV(zEye, input.lightRadiusUVNearFar);
	FindBlocker(avgBlockerDepth, numBlockers, uv, z, dz_duv, searchRegionRadiusUV, tDepthMap, samplertDepth);
	if (numBlockers == 0) return 1.0;
#endif
	
	float2 filterRadiusUV = 0.1 * input.lightRadiusUVNearFar.xy;
	return PCF_Filter(uv, z, dz_duv, filterRadiusUV, MIR_PASS_SHADOWMAP(tDepthMap));
}

inline float2 PenumbraRadiusUV(float zReceiver, float zBlocker, float4 lightParam)
{
	return lightParam.xy * (zReceiver - zBlocker) / zBlocker;
}
inline float2 ProjectToLightUV(float2 sizeUV, float zWorld, float4 lightParam)
{
	return sizeUV * lightParam.z / zWorld;
}
float PCSSShadow(float2 uv, float z, float2 dz_duv, float zEye, 
				 PCFShadowInput input, MIR_ARGS_SHADOWMAP(tDepthMap), SamplerState samplertDepth)
{
    // ------------------------
    // STEP 1: blocker search
    // ------------------------
	float avgBlockerDepth = 0;
	float numBlockers = 0;
	float2 searchRegionRadiusUV = SearchRegionRadiusUV(zEye, input.lightRadiusUVNearFar);
	FindBlocker(avgBlockerDepth, numBlockers, uv, z, dz_duv, searchRegionRadiusUV, tDepthMap, samplertDepth);

    // Early out if no blocker found
	if (numBlockers == 0) return 1.0;

    // ------------------------
    // STEP 2: penumbra size
    // ------------------------
	float avgBlockerDepthWorld = ZClipToZEye(avgBlockerDepth, input.lightDepthParam.xy);
	float2 penumbraRadiusUV = PenumbraRadiusUV(zEye, avgBlockerDepthWorld, input.lightRadiusUVNearFar);
	float2 filterRadiusUV = ProjectToLightUV(penumbraRadiusUV, zEye, input.lightRadiusUVNearFar);
	filterRadiusUV = max(filterRadiusUV, float2(input.lightDepthParam.z/*minPCFRadius*/, input.lightDepthParam.z));
    // ------------------------
    // STEP 3: filtering
    // ------------------------
	return PCF_Filter(uv, z, dz_duv, filterRadiusUV, MIR_PASS_SHADOWMAP(tDepthMap));
}

inline float3 CombineShadowcoordComponents(float2 baseUV, float2 deltaUV, float depth, float3 receiverPlaneDepthBias)
{
	float3 uv = float3(baseUV + deltaUV, depth + receiverPlaneDepthBias.z);
	uv.z += dot(deltaUV, receiverPlaneDepthBias.xy);
	return uv;
}
inline float FastPCFShadow(float3 coord, float3 receiverPlaneDepthBias, float2 ts, MIR_ARGS_SHADOWMAP(tDepthMap))
{
	float2 base_uv = coord.xy;
	float shadow = 0;
	shadow += MIR_SAMPLE_SHADOW(tDepthMap, CombineShadowcoordComponents(base_uv, float2(-ts.x, -ts.y), coord.z, receiverPlaneDepthBias));
	shadow += MIR_SAMPLE_SHADOW(tDepthMap, CombineShadowcoordComponents(base_uv, float2(0, -ts.y), coord.z, receiverPlaneDepthBias));
	shadow += MIR_SAMPLE_SHADOW(tDepthMap, CombineShadowcoordComponents(base_uv, float2(ts.x, -ts.y), coord.z, receiverPlaneDepthBias));
	shadow += MIR_SAMPLE_SHADOW(tDepthMap, CombineShadowcoordComponents(base_uv, float2(-ts.x, 0), coord.z, receiverPlaneDepthBias));
	shadow += MIR_SAMPLE_SHADOW(tDepthMap, CombineShadowcoordComponents(base_uv, float2(0, 0), coord.z, receiverPlaneDepthBias));
	shadow += MIR_SAMPLE_SHADOW(tDepthMap, CombineShadowcoordComponents(base_uv, float2(ts.x, 0), coord.z, receiverPlaneDepthBias));
	shadow += MIR_SAMPLE_SHADOW(tDepthMap, CombineShadowcoordComponents(base_uv, float2(-ts.x, ts.y), coord.z, receiverPlaneDepthBias));
	shadow += MIR_SAMPLE_SHADOW(tDepthMap, CombineShadowcoordComponents(base_uv, float2(0, ts.y), coord.z, receiverPlaneDepthBias));
	shadow += MIR_SAMPLE_SHADOW(tDepthMap, CombineShadowcoordComponents(base_uv, float2(ts.x, ts.y), coord.z, receiverPlaneDepthBias));
	shadow /= 9.0;
	return shadow;
}

#endif