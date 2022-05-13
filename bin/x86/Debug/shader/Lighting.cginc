#ifndef LIGHTING_H
#define LIGHTING_H
#include "Standard.cginc"
#include "CommonFunction.cginc"
#include "UnityLighting.cginc"
#include "GltfLighting.cginc"

inline float3 MirLambertLight(float3 toLight, float3 normal, float3 albedo, bool spotLight)
{
    float3 lightColor = glstate_lightmodel_ambient.xyz;
    {
        float lengthSq = max(dot(toLight, toLight), 0.000001);
        toLight *= rsqrt(lengthSq);

        float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
        if (spotLight)
        {
            float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
            float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
            atten *= saturate(spotAtt);
        }

        float diff = max (0, dot (normal, toLight));
        lightColor += unity_LightColor.rgb * (diff * atten);
    }
    return lightColor * albedo;
}

inline float CalcLightAtten(float lengthSq, float3 toLight, bool spotLight) {
	float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
	if (spotLight) {
		float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
		float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
		atten *= saturate(spotAtt);
	}	
	return atten;
}

inline float3 GetSpecColor(float3 normal, float3 toEye) 
{
#if ENABLE_ENVIRONMENT_MAP
    float3 reflUVW = reflect(-toEye, normal);
	return MIR_SAMPLE_TEXCUBE(_SpecCube, reflUVW).rgb;
#else
	return unity_SpecColor.rgb;
#endif
}

inline float3 BlinnPhongLight(float3 toLight_, float3 normal, float3 toEye, float3 albedo, bool spotLight)
{
	//ambient
	float3 luminance = glstate_lightmodel_ambient.xyz;
	
	//lambert
	float lengthSq = max(dot(toLight_, toLight_), 0.000001);
	toLight_ *= rsqrt(lengthSq);
	
	float ndotl = max(0, dot(normal, toLight_));
    luminance += albedo;// * ndotl * unity_LightColor.rgb;

	//blinn-phong
#if ENABLE_ENVIRONMENT_MAP
    float3 reflUVW = reflect(-toEye, normal);
	luminance += MIR_SAMPLE_TEXCUBE(_SpecCube, reflUVW).rgb;
#else
	float3 h = normalize(toLight_ + toEye);
	float ndoth = max(0, dot(normal, h));
	float spec = pow(ndoth, unity_SpecColor.w*128.0) * unity_LightColor.w;
	luminance += spec * unity_SpecColor.rgb;
#endif

	//point lighting, spot lighting
    return luminance * CalcLightAtten(lengthSq, toLight_, spotLight);
}



#endif