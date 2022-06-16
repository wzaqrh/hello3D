#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_CHANNEL_NONE 0
#define DEBUG_CHANNEL_SHADOW 1
#define DEBUG_CHANNEL_UV_0 2
#define DEBUG_CHANNEL_UV_1 3

#define DEBUG_CHANNEL_NORMAL_TEXTURE 5
#define DEBUG_CHANNEL_GEOMETRY_NORMAL 6
#define DEBUG_CHANNEL_GEOMETRY_TANGENT 7
#define DEBUG_CHANNEL_GEOMETRY_BITANGENT 8
#define DEBUG_CHANNEL_SHADING_NORMAL 9

#define DEBUG_CHANNEL_ALPHA 10
#define DEBUG_CHANNEL_OCCLUSION 11
#define DEBUG_CHANNEL_EMISSIVE 12
#define DEBUG_WINDOW_POS 13
#define DEBUG_CAMERA_POS 14
#define DEBUG_SURFACE_POS 15
#define DEBUG_CHANNEL_VECTOR_L 16
#define DEBUG_CHANNEL_VECTOR_V 17
#define DEBUG_CHANNEL_INTENSITY_NDOTL 18
#define DEBUG_CHANNEL_MIP_LEVEL 19
#define DEBUG_CHANNEL_VECTOR_R 20

#define DEBUG_CHANNEL_BRDF_DIFFUSE	30
#define DEBUG_CHANNEL_BRDF_SPECULAR 31
#define DEBUG_CHANNEL_BRDF_SPECULAR_D 32
#define DEBUG_CHANNEL_BRDF_SPECULAR_V 33
#define DEBUG_CHANNEL_BRDF_SPECULAR_F 34
#define DEBUG_CHANNEL_IBL_DIFFUSE 35
#define DEBUG_CHANNEL_IBL_SPECULAR 36
#define DEBUG_CHANNEL_IBL_DIFFUSE_PREFILTER_ENV 37
#define DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV_UV 38
#define DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV 39
#define DEBUG_CHANNEL_LUT 40

#define DEBUG_CHANNEL_METTALIC_ROUGHNESS 50
#define DEBUG_CHANNEL_BASECOLOR 51
#define DEBUG_CHANNEL_METTALIC 52
#define DEBUG_CHANNEL_PERCEPTUAL_ROUGHNESS 53

#define DEBUG_CHANNEL_TRANSMISSION_VOLUME 54
#define DEBUG_CHANNEL_TRANSMISSION_INTENSITY 55
#define DEBUG_CHANNEL_TRANSMISSION_IBL 56
#define DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_REFRACTION_COORDS 57
#define DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_ATTEN_COLOR 58
#define DEBUG_CHANNEL_TRANSMISSION_LIGHT 59
#define DEBUG_CHANNEL_TRANSMISSION_FACTOR 60
#define DEBUG_CHANNEL_ATTENUATION_COLOR 61
#define DEBUG_CHANNEL_ATTENUATION_DISTANCE_SEPC_WEIGHT 62

#define DEBUG_CHANNEL_SHEEN 70
#define DEBUG_CHANNEL_SHEEN_COLOR 71
#define DEBUG_CHANNEL_SHEEN_ROUGHNESS_SCALING 72
#define DEBUG_CHANNEL_SHEEN_IBL 73
#define DEBUG_CHANNEL_SHEEN_IBL_LUT 74
#define DEBUG_CHANNEL_SHEEN_IBL_LUT_UV 75
#define DEBUG_CHANNEL_SHEEN_IBL_LOD 76
#define DEBUG_CHANNEL_SHEEN_IBL_IRRADIANCE 77

#define DEBUG_CHANNEL_CLEARCOAT 80
#define DEBUG_CHANNEL_CLEARCOAT_F0 81
#define DEBUG_CHANNEL_CLEARCOAT_F90 82
#define DEBUG_CHANNEL_CLEARCOAT_FACTOR 83
#define DEBUG_CHANNEL_CLEARCOAT_ROUGHNESS 84
#define DEBUG_CHANNEL_CLEARCOAT_FRESNEL 85

#define DEBUG_CHANNEL_GBUFFER_POS 90
#define DEBUG_CHANNEL_GBUFFER_NORMAL 91
#define DEBUG_CHANNEL_GBUFFER_ALBEDO 92

#if !defined DEBUG_CHANNEL
	#define DEBUG_CHANNEL 0
#endif

#define LIGHTING_BLINN_PHONG 0
#define LIGHTING_UNITY 1
#define LIGHTING_GLTF 2
#if !defined LIGHTING_MODE
	#define LIGHTING_MODE 2/*LIGHTING_GLTF*/
#endif

#define COLORSPACE_GAMMA 0
#define COLORSPACE_LINEAR 1
#if !defined COLORSPACE
	#if LIGHTING_MODE == LIGHTING_GLTF
		#define COLORSPACE COLORSPACE_LINEAR
	#else
		#define COLORSPACE COLORSPACE_GAMMA
	#endif
#endif

#define TONEMAP_NONE 0
#define TONEMAP_TO_SRGB 1
#define TONEMAP_ACES_NARKOWICZ 2
#define TONEMAP_ACES_HILL 3
#define TONEMAP_ACES_HILL_EXPOSURE_BOOST 4
#if !defined TONEMAP_MODE
	#if COLORSPACE == COLORSPACE_GAMMA
		#define TONEMAP_MODE 0//TONEMAP_NONE
	#else
		#define TONEMAP_MODE 1//TONEMAP_TO_SRGB
	#endif
#endif

#if !defined DEBUG_RIGHT_HANDEDNESS
	#if LIGHTING_MODE == LIGHTING_GLTF
		#define DEBUG_RIGHT_HANDEDNESS 1
	#else
		#define DEBUG_RIGHT_HANDEDNESS 0
	#endif
#endif

#if !defined RIGHT_HANDNESS_RESOURCE
	#if LIGHTING_MODE == LIGHTING_GLTF
		#define RIGHT_HANDNESS_RESOURCE 1
	#else
		#define RIGHT_HANDNESS_RESOURCE 0
	#endif
#endif

#if !defined USE_IBL
	#define USE_IBL 1
#endif

#if !defined USE_PUNCTUAL
	#define USE_PUNCTUAL 1
#endif

#if !defined ENABLE_TRANSMISSION
	#define ENABLE_TRANSMISSION 0
#endif

#if !defined ENABLE_SHEEN
	#if ENABLE_SHEEN_MAP
		#define ENABLE_SHEEN 1
	#else
		#define ENABLE_SHEEN 0
	#endif
#endif

#if !defined ENABLE_CLEARCOAT
	#if ENABLE_CLEARCOAT_MAP
		#define ENABLE_CLEARCOAT 1
	#else
		#define ENABLE_CLEARCOAT 0
	#endif
#endif

#if !defined REVERSE_Z
	#define REVERSE_Z 1
#endif

#if !defined ENABLE_CORRCET_TANGENT_BASIS
	#define ENABLE_CORRCET_TANGENT_BASIS 1
#endif

#if !defined HAS_ATTRIBUTE_NORMAL
	#define HAS_ATTRIBUTE_NORMAL 1
#endif

#if !defined HAS_ATTRIBUTE_TANGENT
	#define HAS_ATTRIBUTE_TANGENT 1
#endif

#define LIGHTMODE_UNKOWN 0
#define LIGHTMODE_SHADOW_CASTER 1
#define LIGHTMODE_SHADOW_CASTER_POSTPROCESS 2
#define LIGHTMODE_FORWARD_BASE 3
#define LIGHTMODE_FORWARD_ADD 4
#define LIGHTMODE_PREPASS_BASE 5
#define LIGHTMODE_PREPASS_FINAL 6
#define LIGHTMODE_PREPASS_FINAL_ADD 7
#define LIGHTMODE_POSTPROCESS 8
#define LIGHTMODE_OVERLAY 9
#if !defined LIGHTMODE
	#define LIGHTMODE 0//LIGHTMODE_UNKOWN
#endif
#define IS_GLOBAL_LIGHTMODE(LM) (LM == LIGHTMODE_SHADOW_CASTER_POSTPROCESS || LM == LIGHTMODE_PREPASS_FINAL || LM == LIGHTMODE_PREPASS_FINAL_ADD || LM == LIGHTMODE_POSTPROCESS) 

#define RENDER_QUEUE_BACKGROUND 0
#define RENDER_QUEUE_GEOMETRY 1000
#define RENDER_QUEUE_TRANSPARENT 2000
#define RENDER_QUEUE_OVERLAY 3000

#define RENDER_TYPE_UNKOWN 0
#define RENDER_TYPE_BACKGROUND 1
#define RENDER_TYPE_GEOMETRY 2
#define RENDER_TYPE_TRANSPARENT 3
#define RENDER_TYPE_OVERLAY 4
#define RENDER_TYPE_UI 5
#define RENDER_TYPE_MAX 6

#define PCSS_QUALITY_LOW 1
#define PCSS_QUALITY_MEDIUM 2 
#define PCSS_QUALITY_HIGH 3
#if !defined PCSS_QUALITY
	#define PCSS_QUALITY 3/*PCSS_QUALITY_HIGH*/
#endif

#define SHADOW_RAW 1 
#define SHADOW_PCF_FAST 2
#define SHADOW_PCF 3
#define SHADOW_PCSS 4
#define SHADOW_VSM 5
#if !defined SHADOW_MODE
	#define SHADOW_MODE 5/*SHADOW_VSM*/
#endif

#if !defined ENABLE_SHADOW_MAP
	#define ENABLE_SHADOW_MAP 1
#endif

#endif