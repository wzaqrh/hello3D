#ifndef BRDF_COMMON_FUNCTION_H
#define BRDF_COMMON_FUNCTION_H

#define MIR_EPS 1e-7f
#define MIR_PI  3.141592f
#define MIR_INV_PI 0.31830988618f

inline float3 SafeNormalize(float3 inVec)
{
	float dp3 = max(0.001f, dot(inVec, inVec));
	return inVec * rsqrt(dp3);
}
inline half Pow5(half x)
{
	return x * x * x * x * x;
}

inline float3 MakeDummyColor(float3 toEye)
{
	float3 fcolor = float3(1, 0, 1);
    fcolor.r = fcolor.b = float(max(2.0 * sin(0.1 * (toEye.x + toEye.y) * 1024), 0.0) + 0.3); 
	return fcolor;
}
inline float3 DisneyDiffuse(float nv, float nl, float lh, float perceptualRoughness, float3 baseColor)
{
    float fd90 = 0.5 + 2 * lh * lh * perceptualRoughness;
    float lightScatter = (1 + (fd90 - 1) * Pow5(1 - nl));
    float viewScatter  = (1 + (fd90 - 1) * Pow5(1 - nv));
    return baseColor / MIR_PI * lightScatter * viewScatter;
}
inline float3 LambertDiffuse(float3 baseColor)
{
    return baseColor / MIR_PI;
}

inline float GGXTRDistribution(float NdotH, float roughness) 
{
	float a2 = roughness * roughness;
    float denom = NdotH * NdotH * (a2 - 1) + 1.0f;
    return a2 * MIR_INV_PI / (denom * denom + 1e-7);
}

inline float SmithJointGGXVisibility(float nl, float nv, float roughness)
{
    float lambdaV = nl * (nv * (1 - roughness) + roughness);
    float lambdaL = nv * (nl * (1 - roughness) + roughness);
    return 0.5 / (lambdaV + lambdaL + 1e-5);
}
inline float SmithJointGGXFilamentVisibility(float nl, float nv, float roughness)
{
    float a2 = roughness * roughness;
    float lambdaV = nl * sqrt((1 - a2) * nv * nv + a2);
    float lambdaL = nv * sqrt((1 - a2) * nl * nl + a2);
	if (lambdaV + lambdaL <= 0.0) return 0.0;
    else return 0.5 / (lambdaV + lambdaL);
}

//·µ»Ølerp(f0, f90, (1-vh)^5)
inline float3 SchlickFresnel(float3 F0, float3 F90, float VdotH)
{
    return F0 + (F90 - F0) * pow(1 - VdotH, 5);
}

#define VectorIdentity float3(1,1,1)
#define DielectricSpec float4(0.04, 0.04, 0.04, 1.0 - 0.04)

#endif