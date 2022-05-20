#ifndef LIGHTING_H
#define LIGHTING_H
#include "Standard.cginc"
#include "CommonFunction.cginc"
#include "UnityLighting.cginc"
#include "GltfLighting.cginc"

inline float CalcLightAtten(float lengthSq, float3 toLight, bool spotLight) {
	float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
	if (spotLight) {
		float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
		float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
		atten *= saturate(spotAtt);
	}	
	return atten;
}

inline float3 BlinnPhongLight(float3 l, float3 n, float3 v, float4 albedo, float4 specular)
{
	float lengthSq = max(dot(l, l), 0.000001);
	l *= rsqrt(lengthSq);
	float3 h = normalize(l + v);
	float nl = max(0, dot(n, l));
	float nh = max(0, dot(n, h));
	
	//ambient
	float3 color = EnvDiffuseColor.rgb;
	
	//diffuse
    color += LightColor.rgb * albedo.rgb * nl;

	//specular
	color += specular.rgb * albedo.w * pow(nh, specular.w * 128.0);

#if 0
    return CalcLightAtten(lengthSq, l, spotLight);
#else
	return color;
#endif
}



#endif