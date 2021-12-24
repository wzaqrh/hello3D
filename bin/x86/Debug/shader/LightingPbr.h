#define MIR_EPS 1e-7f
#define MIR_PI  3.141592f

inline float3 DisneyDiffuse(float NdotV, float NdotL, float LdotH, float perceptualRoughness, float3 baseColor)
{
    float fd90 = 0.5 + 2 * LdotH * LdotH * perceptualRoughness;
    float lightScatter = (1 + (fd90 - 1) * pow(1 - NdotL,5));
    float viewScatter  = (1 + (fd90 - 1) * pow(1 - NdotV,5));
    return baseColor / MIR_PI * lightScatter * viewScatter;
}

inline float3 GGXTRDistribution(float NdotH, float alpha) 
{
	float a2 = alpha * alpha;
    float denom = NdotH * NdotH * (a2 - 1) + 1.0f;
    return a2 / (MIR_PI * denom * denom + MIR_EPS);
}

float SmithJointGGXVisibility(float NdotL, float NdotV, float alpha)
{
    float lambdaV = NdotL * (NdotV * (1 - alpha) + alpha);
    float lambdaL = NdotV * (NdotL * (1 - alpha) + alpha);
    return 0.5f / (lambdaV + lambdaL + 1e-4f);
}

inline float3 SchlickFresnel(float3 F0, float LdotH)
{
    return F0 + (1.0f - F0) * pow(1.0f - LdotH, 5) ;
}

inline float3 CookTorranceSpec(float NdotL, float LdotH, float NdotH, float NdotV, float perceptualRoughness, float3 F0)
{
    float alpha = max(perceptualRoughness * perceptualRoughness, 0.002);
    float D = GGXTRDistribution(NdotH, alpha);
    float F = SchlickFresnel(F0, LdotH);
	float V = SmithJointGGXVisibility(NdotL, NdotV, alpha);
    return NdotL * D * F * V;
}

#define DielectricSpec float4(0.04, 0.04, 0.04, 1.0 - 0.04)
inline float3 MirPbrLight(float3 toLight_, float3 normal, float3 toEye, float3 albedo, float metalness, float smoothness)
{
	float lengthSq = max(dot(toLight_, toLight_), 0.000001);
	toLight_ *= rsqrt(lengthSq);
	
	float3 halfView = normalize(toLight_ + toEye);
	float NdotL = dot(normal, toLight_);
	float LdotH = dot(toLight_, halfView);
	float NdotH = dot(normal, halfView);
	float NdotV = dot(normal, toEye);
	float perceptualRoughness = 1.0 - smoothness;
	float3 diffuse = DisneyDiffuse(NdotV, NdotL, LdotH, perceptualRoughness, albedo);
	
	float reflectivity = lerp(DielectricSpec.x, 1, metalness);
	float kd = 1.0f - reflectivity;
	
	float3 F0 = lerp(DielectricSpec.rgb, albedo, metalness);
	float3 specular = CookTorranceSpec(NdotL, LdotH, NdotH, NdotV, perceptualRoughness, F0);

	float ks = 1.0f;
	return MIR_PI * (kd * diffuse + ks * specular) * unity_LightColor.rgb * NdotL;
}