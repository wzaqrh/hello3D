#ifndef LIGHTING_PBR_H
#define LIGHTING_PBR_H

#define MIR_EPS 1e-7f
#define MIR_PI  3.141592f
#define MIR_INV_PI 0.31830988618f

inline float3 DisneyDiffuse(float NdotV, float NdotL, float LdotH, float perceptualRoughness, float3 baseColor)
{
    float fd90 = 0.5 + 2 * LdotH * LdotH * perceptualRoughness;
    float lightScatter = (1 + (fd90 - 1) * pow(1 - NdotL,5));
    float viewScatter  = (1 + (fd90 - 1) * pow(1 - NdotV,5));
    return baseColor / MIR_PI * lightScatter * viewScatter;
}
inline float3 LambertDiffuse(float3 baseColor)
{
    return baseColor / MIR_PI;
}

#if 0
inline float GGXTRDistribution(float NdotH, float alpha) 
{
	float a2 = alpha * alpha;
    float denom = NdotH * (a2 * NdotH - NdotH) + 1.0f;
    return (a2 * MIR_INV_PI) / (denom * denom + MIR_EPS);
}
#else
inline float GGXTRDistribution(float NdotH, float roughness)
{
    float a2 = roughness * roughness;
    float d = (NdotH * a2 - NdotH) * NdotH + 1.0f; // 2 mad
    return MIR_INV_PI * a2 / (d * d + 1e-7f); // This function is not intended to be running on Mobile,
                                            // therefore epsilon is smaller than what can be represented by half
}
#endif

#if 0
inline float SmithJointGGXVisibility(float NdotL, float NdotV, float alpha)
{
    float lambdaV = NdotL * (NdotV * (1 - alpha) + alpha);
    float lambdaL = NdotV * (NdotL * (1 - alpha) + alpha);
    return 0.5f / (lambdaV + lambdaL + 1e-5f);
}
#else
inline float SmithJointGGXVisibility(float NdotL, float NdotV, float roughness)
{
#if 0

#else
    // Approximation of the above formulation (simplify the sqrt, not mathematically correct but close enough)
    float a = roughness;
    float lambdaV = NdotL * (NdotV * (1 - a) + a);
    float lambdaL = NdotV * (NdotL * (1 - a) + a);

#if defined(SHADER_API_SWITCH)
    return 0.5f / (lambdaV + lambdaL + 1e-4f); // work-around against hlslcc rounding error
#else
    return 0.5f / (lambdaV + lambdaL + 1e-5f);
#endif

#endif
}
#endif

inline float SmithJointGGXFilamentVisibility(float NdotL, float NdotV, float alpha)
{
    float a2 = alpha * alpha;
    float lambdaV = NdotL * sqrt(NdotV * NdotV * (1 - a2) + a2);
    float lambdaL = NdotV * sqrt(NdotL * NdotL * (1 - a2) + a2);
    return 0.5f / (lambdaV + lambdaL + 1e-5f);
}

#if 0
inline float3 SchlickFresnel(float3 F0, float LdotH)
{
    return F0 + (1.0f - F0) * pow(1.0f - LdotH, 5);
}
#else
inline float Pow5(float x)
{
    return x * x * x * x * x;
}
inline float3 SchlickFresnel(float3 F0, float cosA)
{
    half t = Pow5(1 - cosA); // ala Schlick interpoliation
    return F0 + (1 - F0) * t;
}
#endif

#define VectorIdentity float3(1,1,1)
#define DielectricSpec float4(0.04, 0.04, 0.04, 1.0 - 0.04)
float3 UnityPbrLight(float3 toLight_, float3 normal, float3 toEye, float3 albedo, float metalness, float smoothness)
{
	float lengthSq = max(dot(toLight_, toLight_), 0.000001);
	toLight_ *= rsqrt(lengthSq);
	
    float3 halfView = normalize(toLight_ + toEye);
    float NdotL = saturate(dot(normal, toLight_));
    float LdotH = saturate(dot(toLight_, halfView));
    float NdotH = saturate(dot(normal, halfView));
    float NdotV = saturate(dot(normal, toEye));
	float perceptualRoughness = 1.0 - smoothness;
    float alpha = max(perceptualRoughness * perceptualRoughness, 0.002);
    float3 F0 = lerp(DielectricSpec.rgb, albedo, metalness);
    
	float3 diffuse = DisneyDiffuse(NdotV, NdotL, LdotH, perceptualRoughness, albedo);
    
    float D = GGXTRDistribution(NdotH, alpha);
    float3 F = SchlickFresnel(F0, LdotH);
    float V = SmithJointGGXVisibility(NdotL, NdotV, alpha);
    float3 specular = D * F * V;
    
    float reflectivity = lerp(DielectricSpec.x, 1, metalness);
    float kd = 1.0f - reflectivity;
    float ks = 1.0f;
    //kd = 0.0f;
    //ks = 1.0;
    float3 finalColor = (kd * diffuse + ks * specular) * unity_LightColor.rgb * NdotL;
    //finalColor = diffuse;
    //finalColor = VectorIdentity * -normal;
    //finalColor = VectorIdentity * -toEye;
    //finalColor = VectorIdentity * -toLight_;
    //finalColor = VectorIdentity * -halfView;
    //finalColor = VectorIdentity * NdotL;
    //finalColor = VectorIdentity * LdotH;
    //finalColor = VectorIdentity * NdotH;
    //finalColor = VectorIdentity * NdotV;
    return finalColor;
}

float3 GltfPbrLight(float3 toLight_, float3 normal, float3 toEye, float3 albedo, float metalness, float smoothness)
{
    float lengthSq = max(dot(toLight_, toLight_), 0.000001);
    toLight_ *= rsqrt(lengthSq);
	
    float3 halfView = normalize(toLight_ + toEye);
    float NdotL = saturate(dot(normal, toLight_));
    float LdotH = saturate(dot(toLight_, halfView));
    float NdotH = saturate(dot(normal, halfView));
    float NdotV = saturate(dot(normal, toEye));
    float perceptualRoughness = 1.0 - smoothness;
    float alpha = max(perceptualRoughness * perceptualRoughness, 0.002);
    float3 F0 = lerp(DielectricSpec.rgb, albedo, metalness);
    
    float3 diffuse = LambertDiffuse(albedo);
    
    float D = GGXTRDistribution(NdotH, alpha);
    float3 F = SchlickFresnel(F0, LdotH);
    float V = SmithJointGGXFilamentVisibility(NdotL, NdotV, alpha);
    float3 specular = D * F * V;
    
    float3 kd = 1.0 - F;
    float ks = 1.0;
    return (kd * diffuse + ks * specular) * unity_LightColor.rgb * NdotL;
}

#endif