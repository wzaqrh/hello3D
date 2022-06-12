#ifndef LIGHTING_H
#define LIGHTING_H
#include "Standard.cginc"
#include "CommonFunction.cginc"
#include "UnityLighting.cginc"
#include "GltfLighting.cginc"
#include "LightingInput.cginc"
#include "Shadow.cginc"

inline float CalcLightAtten(float lengthSq, float3 toLight, bool spotLight) {
	float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
	if (spotLight) {
		float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
		float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
		atten *= saturate(spotAtt);
	}	
	return atten;
}

inline float3 BlinnPhongLightBase()
{
	//ambient
	return EnvDiffuseColor.rgb;
}
inline float3 BlinnPhongLightAdditive(LightingInput i, float3 l, float3 n, float3 v)
{
	float3 h = normalize(l + v);
	float nl = max(0, dot(n, l));
	float nh = max(0, dot(n, h));
	//diffuse
    float3 color = i.light_color.rgb * i.albedo.rgb * (1.0 - i.metallic) * nl;
	//specular
	color += i.light_color.rgb * i.metallic * pow(nh, (1.0 - i.percertual_roughness) * 128.0);
	return color;
}

inline float4 Lighting(LightingInput i, float3 l, float3 n, float3 v, bool isAdditive) 
{
	float3 fcolor = 0.0;
	
#if ENABLE_SHADOW_MAP
	if (!isAdditive && any(i.light_color)) {
		float4 lightViewPos = mul(LightView, float4(i.world_pos, 1.0));
		float4 lightNdc = mul(LightProjection, lightViewPos);
		#if ENABLE_SHADOW_MAP_BIAS
			float bias = max(0.001 * (1.0 - dot(normal.xyz, toLight)), 1e-5);
			posLight.z -= bias * posLight.w;
		#endif
		i.light_color.rgb *= CalcShadowFactor(lightNdc.xyz / lightNdc.w, lightViewPos.xyz);
	}
#endif
#if DEBUG_CHANNEL == DEBUG_CHANNEL_SHADOW
	return i.light_color;
#endif
	
#if LIGHTING_MODE == LIGHTING_BLINN_PHONG
	if (!isAdditive) fcolor += BlinnPhongLightBase();
	fcolor += BlinnPhongLightAdditive(i, l, n, v);
#elif LIGHTING_MODE == LIGHTING_UNITY
	if (!isAdditive) fcolor += UnityLightBase(i, l, n, v);
	fcolor += UnityLightAdditive(i, l, n, v);
#elif LIGHTING_MODE == LIGHTING_GLTF
	GltfLightInput gli = GetGlftInput(i, l, n, v);
	if (!isAdditive) fcolor += GltfLightBase(gli, i, l, n, v);
	fcolor = GltfLightAdditive(gli, i, fcolor, l, n, v);
#endif

#if TONEMAP_MODE && !DEBUG_CHANNEL
	fcolor = toneMap(fcolor, CameraPositionExposure.w);
#endif
	
#if DEBUG_CHANNEL == DEBUG_CHANNEL_OCCLUSION
	fcolor = i.ao;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_EMISSIVE
	fcolor = i.emissive;
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
	fcolor = saturate(dot(n, l));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_VECTOR_R
	fcolor = normalize(reflect(-v, n));
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.z = -fcolor.z;
	#endif
	fcolor = fcolor * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHADING_NORMAL
	fcolor = n.xyz;	
	#if DEBUG_RIGHT_HANDEDNESS
		#if ENABLE_NORMAL_MAP
			float3 _t = i.tangent_basis.xyz * 2.0 - 1.0;
			float3 _b = i.bitangent_basis.xyz * 2.0 - 1.0;
			float3 _n = i.normal_basis.xyz * 2.0 - 1.0;
			float3 tn = i.tangent_normal.xyz * 2.0 - 1.0;
			fcolor.x += -2.0 * _b.x * tn.y;					//dot(tn.xyz, float3(_t.x, -_b.x, _n.x));
			fcolor.y += -2.0 * _b.y * tn.y;					//dot(tn.xyz, float3(_t.y, -_b.y, _n.y));
			fcolor.z += -2.0 * (_t.z * tn.x + _n.z * tn.z); //dot(tn.xyz, float3(-_t.z, _b.z, -_n.z));
			fcolor = normalize(fcolor);
		#else
			fcolor.z = -fcolor.z;
		#endif
	#endif
	fcolor = fcolor * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_METTALIC
    fcolor = i.metallic;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_PERCEPTUAL_ROUGHNESS
    fcolor = i.percertual_roughness;
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
	fcolor = i.bitangent_basis.xyz;
	#if DEBUG_RIGHT_HANDEDNESS
		fcolor.xy = 1.0 - fcolor.xy;
	#endif
#elif DEBUG_CHANNEL == DEBUG_WINDOW_POS
	fcolor = float3(i.window_pos.xy * FrameBufferSize.zw, 0);
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

#if DEBUG_CHANNEL	
	if (isAdditive) fcolor = 0.0;
#endif
	return float4(fcolor, 1.0);
}

#endif