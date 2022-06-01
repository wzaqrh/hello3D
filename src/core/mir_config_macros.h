#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_CHANNEL_UV_0 1
#define DEBUG_CHANNEL_UV_1 2

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
#define DEBUG_CHANNEL_IBL_SPECULAR_LUT 40

#define DEBUG_CHANNEL_METTALIC_ROUGHNESS 50
#define DEBUG_CHANNEL_BASECOLOR 51
#define DEBUG_CHANNEL_METTALIC 52
#define DEBUG_CHANNEL_PERCEPTUAL_ROUGHNESS 53
#if !defined DEBUG_CHANNEL
#define DEBUG_CHANNEL DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV
#endif

#if !defined USE_IBL
#define USE_IBL 1
#endif

#if !defined USE_PUNCTUAL
#define USE_PUNCTUAL 1
#endif

#if !defined COLORSPACE_GAMMA
#define COLORSPACE_GAMMA 1
#endif

#if !defined REVERSE_Z
#define REVERSE_Z 1
#endif

#define TONEMAP_LINEAR 0
#define TONEMAP_TO_SRGB 1
#define TONEMAP_ACES_NARKOWICZ 2
#define TONEMAP_ACES_HILL 3
#define TONEMAP_ACES_HILL_EXPOSURE_BOOST 4
#if !defined TONEMAP_MODE
	#if COLORSPACE_GAMMA
		#define TONEMAP_MODE 0/*TONEMAP_LINEAR*/
	#else
		#define TONEMAP_MODE TONEMAP_ACES_NARKOWICZ
	#endif
#endif

#define LIGHTING_BLINN_PHONG 0
#define LIGHTING_UNITY 1
#define LIGHTING_GLTF 2
#if !defined LIGHTING_MODE
#define LIGHTING_MODE 1/*LIGHTING_UNITY*/
#endif

#if LIGHTING_MODE == LIGHTING_GLTF
#define DEBUG_RIGHT_HANDEDNESS 1
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

#if !defined RIGHT_HANDNESS_RESOURCE
#define RIGHT_HANDNESS_RESOURCE 1
#endif

#define LIGHTMODE_SHADOW_CASTER 0
#define LIGHTMODE_FORWARD_BASE 1
#define LIGHTMODE_FORWARD_ADD 2
#define LIGHTMODE_PREPASS_BASE 3
#define LIGHTMODE_PREPASS_FINAL 4
#define LIGHTMODE_TRANSPARENT 5
#define LIGHTMODE_SKYBOX 6
#define LIGHTMODE_OVERLAY 7
#define LIGHTMODE_POSTPROCESS 8
#if !defined LIGHTMODE
#define LIGHTMODE -1
#endif

#define PCSS_QUALITY_LOW 1
#define PCSS_QUALITY_MEDIUM 2 
#define PCSS_QUALITY_HIGH 3
#if !defined PCSS_QUALITY
#define PCSS_QUALITY 3/*PCSS_QUALITY_MEDIUM*/
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