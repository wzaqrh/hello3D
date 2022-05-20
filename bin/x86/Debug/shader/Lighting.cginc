#ifndef LIGHTING_H
#define LIGHTING_H
#include "Standard.cginc"
#include "CommonFunction.cginc"
#include "UnityLighting.cginc"
#include "GltfLighting.cginc"
#include "LightingInput.cginc"

inline float CalcLightAtten(float lengthSq, float3 toLight, bool spotLight) {
	float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
	if (spotLight) {
		float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
		float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
		atten *= saturate(spotAtt);
	}	
	return atten;
}

inline float3 BlinnPhongLight(LightingInput i, float3 l, float3 n, float3 v)
{
	float3 h = normalize(l + v);
	float nl = max(0, dot(n, l));
	float nh = max(0, dot(n, h));
	//ambient
	float3 color = EnvDiffuseColor.rgb;
	//diffuse
    color += LightColor.rgb * i.albedo.rgb * nl;
	//specular
	color += i.ao_rough_metal.rgb * i.albedo.w * pow(nh, i.ao_rough_metal.w * 128.0);
	return color;
}

inline float4 Lighting(LightingInput i, float3 l, float3 n, float3 v) 
{
	float3 fcolor;
#if LIGHTING_MODE == LIGHTING_BLINN_PHONG
	fcolor.rgb = BlinnPhongLight(i, l, n, v);
#elif LIGHTING_MODE == LIGHTING_UNITY
	fcolor.rgb = UnityPbrLight(i, l, n, v);
#elif LIGHTING_MODE == LIGHTING_GLTF
	fcolor.rgb = GltfPbrLight(i, l, n, v);
#endif

#if 0
#if ENABLE_SHADOW_MAP
	//float depth = length(LightPosition.xyz - input.WorldPos.xyz * LightPosition.w);
	//finalColor.rgb *= CalcShadowFactor(input.PosLight.xyz / input.PosLight.w, input.ViewPosLight.xyz);
#endif

#if ENABLE_SHADOW_MAP
	float4 viewPosLight = mul(LightView, worldPosition);
	float4 posLight = mul(LightProjection, viewPosLight);
#if ENABLE_SHADOW_MAP_BIAS
	float bias = max(0.001 * (1.0 - dot(normal.xyz, toLight)), 1e-5);
	posLight.z -= bias * posLight.w;
#endif
	output.Color.rgb *= CalcShadowFactor(posLight.xyz / posLight.w, viewPosLight.xyz);
#endif
#endif
	
#if DEBUG_CHANNEL 	
	float ao = i.ao_rough_metal.x;
    float perceptualRoughness = i.ao_rough_metal.y;
	float metallic = i.ao_rough_metal.z;
#endif
	
#if DEBUG_CHANNEL == DEBUG_CHANNEL_OCCLUSION
	fcolor = linearTosRGB(float3(ao, ao, ao));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_EMISSIVE
	fcolor = linearTosRGB(i.emissive.rgb);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_VECTOR_L
	fcolor = l;
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.z = -fcolor.z;
	#endif
	fcolor = fcolor * 0.5 + 0.5;	
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_VECTOR_V
	fcolor = v;
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.z = -fcolor.z;
	#endif
	fcolor = fcolor * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_INTENSITY_NDOTL
	fcolor = float3(nl, nl, nl);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_VECTOR_R
	fcolor = normalize(reflect(-v, n));
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.z = -fcolor.z;
	#endif
	fcolor = fcolor * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BASECOLOR
    fcolor = linearTosRGB(i.albedo.rgb);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHADING_NORMAL
	fcolor = n.xyz;	
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.z = -fcolor.z;
	#endif
	fcolor = fcolor * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_METTALIC
    fcolor = linearTosRGB(float3(metallic, metallic, metallic));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_PERCEPTUAL_ROUGHNESS
    fcolor = (float3(perceptualRoughness, perceptualRoughness, perceptualRoughness));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_UV_0
    fcolor = float3(i.uv, 0);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_UV_1
    fcolor = float3(i.uv1, 0);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_NORMAL_TEXTURE
	fcolor = i.tangent_normal.xyz;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_NORMAL
	fcolor = i.normal_basis.xyz;
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.z = 1.0 - fcolor.z;
	#endif
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_TANGENT
	fcolor = i.tangent_basis.xyz;
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.z = 1.0 - fcolor.z;
	#endif
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_BITANGENT
	fcolor = MakeDummyColor(v);
#elif DEBUG_CHANNEL == DEBUG_WINDOW_POS
	fcolor = float3(input.Pos.xy * FrameBufferSize.zw, 0);
	fcolor.y = 1.0 - fcolor.y;
#elif DEBUG_CHANNEL == DEBUG_CAMERA_POS
	fcolor.xyz = CameraPositionExposure.xyz;
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.z = -fcolor.z;
	#endif
	fcolor.xyz = fcolor.xyz * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_SURFACE_POS
	fcolor.xyz = i.world_pos * 32 * 0.5 + 0.5;
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.z = 1.0 - fcolor.z;
	#endif
#endif
	return float4(fcolor, 1.0);
}

#endif