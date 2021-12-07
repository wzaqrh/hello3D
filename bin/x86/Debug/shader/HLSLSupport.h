// Unity built-in shader source. Copyright (c) 2016 Unity Technologies. MIT license (see license.txt)

#ifndef HLSL_SUPPORT_INCLUDED
#define HLSL_SUPPORT_INCLUDED

#define SHADER_API_DESKTOP
#define SHADER_API_D3D11

#if defined(SHADER_API_D3D11)
    #define MIR_COMPILER_HLSL
#elif defined(SHADER_TARGET_GLSL)
    #define MIR_COMPILER_HLSL2GLSL
#else
    #define MIR_COMPILER_CG
#endif

#if defined(SHADER_API_GLCORE)
    #define MIR_STEREO_MULTIVIEW_ENABLED
#endif

#if defined(SHADER_API_D3D11) || defined(SHADER_API_GLCORE)
    #define MIR_STEREO_INSTANCING_ENABLED
#endif

#if defined(SHADER_API_D3D11) || defined(SHADER_API_GLCORE)
	#define MIR_SUPPORT_DEPTH_FETCH 1
#endif

// SV_Target[n] / SV_Depth defines, if not defined by compiler already
#if !defined(SHADER_API_D3D11)
#define SV_Target 	COLOR
#define SV_Target0 	COLOR0
#define SV_Target1 	COLOR1
#define SV_Target2 	COLOR2
#define SV_Target3 	COLOR3
#define SV_Target4 	S_TARGET_OUTPUT4
#define SV_Target5 	S_TARGET_OUTPUT5
#define SV_Target6 	S_TARGET_OUTPUT6
#define SV_Target7 	S_TARGET_OUTPUT7
#define SV_Depth 	DEPTH
#endif

#if defined(SHADER_API_GLES)
	#error
#else
    #define MIR_ALLOWED_MRT_COUNT 8
#endif

#if (SHADER_MODEL < 30)
    //no fast coherent dynamic branching on these hardware
#else
    #define MIR_FAST_COHERENT_DYNAMIC_BRANCHING 1
#endif

#if !defined(SHADER_TARGET_GLSL)
#define fixed half
#define fixed2 half2
#define fixed3 half3
#define fixed4 half4
#define fixed4x4 half4x4
#define fixed3x3 half3x3
#define fixed2x2 half2x2
#define sampler2D_half sampler2D
#define sampler2D_float sampler2D
#define samplerCUBE_half samplerCUBE
#define samplerCUBE_float samplerCUBE
#define sampler3D_float sampler3D
#define sampler3D_half sampler3D
#define Texture2D_half Texture2D
#define Texture2D_float Texture2D
#define Texture2DArray_half Texture2DArray
#define Texture2DArray_float Texture2DArray
#define Texture2DMS_half Texture2DMS
#define Texture2DMS_float Texture2DMS
#define TextureCube_half TextureCube
#define TextureCube_float TextureCube
#define TextureCubeArray_half TextureCubeArray
#define TextureCubeArray_float TextureCubeArray
#define Texture3D_float Texture3D
#define Texture3D_half Texture3D
#endif

#if !defined(SHADER_API_D3D11)
#define min16float half
#define min16float2 half2
#define min16float3 half3
#define min16float4 half4
#define min10float fixed
#define min10float2 fixed2
#define min10float3 fixed3
#define min10float4 fixed4
#endif

// specifically for samplers that are provided as arguments to entry functions
#if defined(SHADER_API_PSSL)
	#error
#else
	#define SAMPLER_UNIFORM
#endif

#if defined(SHADER_API_D3D11)
	#define CBUFFER_START(name) cbuffer name {
	#define CBUFFER_END };
#else
	// On specific platforms, like OpenGL, GLES3 and Metal, constant buffers may still be used for instancing
	#define CBUFFER_START(name)
	#define CBUFFER_END
#endif

#if defined(MIR_STEREO_MULTIVIEW_ENABLED) || ((defined(MIR_SINGLE_PASS_STEREO) || defined(MIR_STEREO_INSTANCING_ENABLED)) && defined(SHADER_API_GLCORE))
    #define GLOBAL_CBUFFER_START(name)    cbuffer name {
    #define GLOBAL_CBUFFER_END            }
#else
    #define GLOBAL_CBUFFER_START(name)    CBUFFER_START(name)
    #define GLOBAL_CBUFFER_END            CBUFFER_END
#endif

#define MIR_PROJ_COORD(a) a

// Depth texture sampling helpers.
//
// SAMPLE_DEPTH_TEXTURE(sampler,uv): returns scalar depth
// SAMPLE_DEPTH_TEXTURE_PROJ(sampler,uv): projected sample
// SAMPLE_DEPTH_TEXTURE_LOD(sampler,uv): sample with LOD level
#define SAMPLE_DEPTH_TEXTURE(sampler, uv) (tex2D(sampler, uv).r)
#define SAMPLE_DEPTH_TEXTURE_PROJ(sampler, uv) (tex2Dproj(sampler, uv).r)
#define SAMPLE_DEPTH_TEXTURE_LOD(sampler, uv) (tex2Dlod(sampler, uv).r)
 // Sample depth, all components.
#define SAMPLE_RAW_DEPTH_TEXTURE(sampler, uv) (tex2D(sampler, uv))
#define SAMPLE_RAW_DEPTH_TEXTURE_PROJ(sampler, uv) (tex2Dproj(sampler, uv))
#define SAMPLE_RAW_DEPTH_TEXTURE_LOD(sampler, uv) (tex2Dlod(sampler, uv))
#define SAMPLE_DEPTH_CUBE_TEXTURE(sampler, uv) (texCUBE(sampler, uv).r)

// Deprecated; use SAMPLE_DEPTH_TEXTURE & SAMPLE_DEPTH_TEXTURE_PROJ instead
#define MIR_SAMPLE_DEPTH(value) (value).r


// Macros to declare and sample shadow maps.
//
// MIR_DECLARE_SHADOWMAP declares a shadowmap.
// MIR_SAMPLE_SHADOW samples with a float3 coordinate (UV in xy, Z in z) and returns 0..1 scalar result.
// MIR_SAMPLE_SHADOW_PROJ samples with a projected coordinate (UV and Z divided by w).
#if defined(SHADER_API_D3D11)
    // DX11 & hlslcc platforms: built-in PCF
    #define MIR_DECLARE_SHADOWMAP(tex,slot) Texture2D tex :register(t##slot); SamplerComparisonState sampler##tex :register(s##slot)
    #define MIR_ARGS_SHADOWMAP(tex) Texture2D tex, SamplerComparisonState sampler##tex
	#define MIR_PASS_SHADOWMAP(tex) tex, sampler##tex
	#define MIR_DECLARE_TEXCUBE_SHADOWMAP(tex) TextureCube tex; SamplerComparisonState sampler##tex
    #define MIR_SAMPLE_SHADOW(tex,coord) tex.SampleCmpLevelZero (sampler##tex,(coord).xy,(coord).z)
	
    #define MIR_SAMPLE_SHADOW_PROJ(tex,coord) tex.SampleCmpLevelZero (sampler##tex,(coord).xy/(coord).w,(coord).z/(coord).w)
    #if defined(SHADER_API_GLCORE) || defined(SHADER_API_GLES3) || defined(SHADER_API_VULKAN) || defined(SHADER_API_SWITCH)
        // GLSL does not have textureLod(samplerCubeShadow, ...) support. GLES2 does not have core support for samplerCubeShadow, so we ignore it.
        #define MIR_SAMPLE_TEXCUBE_SHADOW(tex,coord) tex.SampleCmp (sampler##tex,(coord).xyz,(coord).w)
    #else
       #define MIR_SAMPLE_TEXCUBE_SHADOW(tex,coord) tex.SampleCmpLevelZero (sampler##tex,(coord).xyz,(coord).w)
    #endif
#else
    // Fallback / No built-in shadowmap comparison sampling: regular texture sample and do manual depth comparison
    #define MIR_DECLARE_SHADOWMAP(tex,slot) sampler2D_float tex
    #define MIR_ARGS_SHADOWMAP(tex) sampler2D_float tex
	#define MIR_PASS_SHADOWMAP(tex) tex
	#define MIR_DECLARE_TEXCUBE_SHADOWMAP(tex) samplerCUBE_float tex
    #define MIR_SAMPLE_SHADOW(tex,coord) ((SAMPLE_DEPTH_TEXTURE(tex,(coord).xy) < (coord).z) ? 0.0 : 1.0)
    #define MIR_SAMPLE_SHADOW_PROJ(tex,coord) ((SAMPLE_DEPTH_TEXTURE_PROJ(tex,MIR_PROJ_COORD(coord)) < ((coord).z/(coord).w)) ? 0.0 : 1.0)
    #define MIR_SAMPLE_TEXCUBE_SHADOW(tex,coord) ((SAMPLE_DEPTH_CUBE_TEXTURE(tex,(coord).xyz) < (coord).w) ? 0.0 : 1.0)
#endif


// Macros to declare textures and samplers, possibly separately. For platforms
// that have separate samplers & textures (like DX11), and we'd want to conserve
// the samplers.
//  - MIR_DECLARE_TEX*_NOSAMPLER declares a texture, without a sampler.
//  - MIR_SAMPLE_TEX*_SAMPLER samples a texture, using sampler from another texture.
//      That another texture must also be actually used in the current shader, otherwise
//      the correct sampler will not be set.
#if defined(SHADER_API_D3D11)
    // DX11 style HLSL syntax; separate textures and samplers
    //
    // Note: for HLSLcc we have special unity-specific syntax to pass sampler precision information.
    //
    // Note: for surface shader analysis, go into DX11 syntax path when non-mojoshader part of analysis is done,
    // this allows surface shaders to use _NOSAMPLER and similar macros, without using up a sampler register.
    // Don't do that for mojoshader part, as that one can only parse DX9 style HLSL.
    #define MIR_SEPARATE_TEXTURE_SAMPLER

    // 2D textures
	#define MIR_DECLARE_TEX2D(tex,slot) Texture2D tex :register(t##slot); SamplerState sampler##tex :register(s##slot)
    #define MIR_ARGS_TEX2D(tex) Texture2D tex, SamplerState sampler##tex
	#define MIR_PASS_TEX2D(tex) tex, sampler##tex
	#define MIR_DECLARE_TEX2D_NOSAMPLER(tex) Texture2D tex
	#define MIR_DECLARE_TEX2D_NOSAMPLER_INT(tex) Texture2D<int4> tex
    #define MIR_DECLARE_TEX2D_NOSAMPLER_UINT(tex) Texture2D<uint4> tex
    #define MIR_SAMPLE_TEX2D(tex,coord) tex.Sample (sampler##tex,coord)
    #define MIR_SAMPLE_TEX2D_SAMPLER(tex,samplertex,coord) tex.Sample (sampler##samplertex,coord)
#if defined(UNITY_COMPILER_HLSLCC) && !defined(SHADER_API_GLCORE) // GL Core doesn't have the _half mangling, the rest of them do.
	#error
#else
    #define MIR_DECLARE_TEX2D_HALF(tex) Texture2D tex; SamplerState sampler##tex
    #define MIR_DECLARE_TEX2D_FLOAT(tex) Texture2D tex; SamplerState sampler##tex
    #define MIR_DECLARE_TEX2D_NOSAMPLER_HALF(tex) Texture2D tex
    #define MIR_DECLARE_TEX2D_NOSAMPLER_FLOAT(tex) Texture2D tex
#endif

    // Cubemaps
    #define MIR_DECLARE_TEXCUBE(tex,slot) TextureCube tex :register(t##slot); SamplerState sampler##tex :register(s##slot)
    #define MIR_ARGS_TEXCUBE(tex) TextureCube tex, SamplerState sampler##tex
    #define MIR_PASS_TEXCUBE(tex) tex, sampler##tex
    #define MIR_PASS_TEXCUBE_SAMPLER(tex,samplertex) tex, sampler##samplertex
    #define MIR_PASS_TEXCUBE_SAMPLER_LOD(tex, samplertex, lod) tex, sampler##samplertex, lod
    #define MIR_DECLARE_TEXCUBE_NOSAMPLER(tex) TextureCube tex
    #define MIR_SAMPLE_TEXCUBE(tex,coord) tex.Sample (sampler##tex,coord)
    #define MIR_SAMPLE_TEXCUBE_LOD(tex,coord,lod) tex.SampleLevel (sampler##tex,coord, lod)
    #define MIR_SAMPLE_TEXCUBE_SAMPLER(tex,samplertex,coord) tex.Sample (sampler##samplertex,coord)
    #define MIR_SAMPLE_TEXCUBE_SAMPLER_LOD(tex, samplertex, coord, lod) tex.SampleLevel (sampler##samplertex, coord, lod)
    // 3D textures
    #define MIR_DECLARE_TEX3D(tex) Texture3D tex; SamplerState sampler##tex
    #define MIR_DECLARE_TEX3D_NOSAMPLER(tex) Texture3D tex
    #define MIR_SAMPLE_TEX3D(tex,coord) tex.Sample (sampler##tex,coord)
    #define MIR_SAMPLE_TEX3D_LOD(tex,coord,lod) tex.SampleLevel (sampler##tex,coord, lod)
    #define MIR_SAMPLE_TEX3D_SAMPLER(tex,samplertex,coord) tex.Sample (sampler##samplertex,coord)
    #define MIR_SAMPLE_TEX3D_SAMPLER_LOD(tex, samplertex, coord, lod) tex.SampleLevel(sampler##samplertex, coord, lod)

#if defined(MIR_COMPILER_HLSLCC) && !defined(SHADER_API_GLCORE) // GL Core doesn't have the _half mangling, the rest of them do.
    #define MIR_DECLARE_TEX3D_FLOAT(tex) Texture3D_float tex; SamplerState sampler##tex
    #define MIR_DECLARE_TEX3D_HALF(tex) Texture3D_half tex; SamplerState sampler##tex
#else
    #define MIR_DECLARE_TEX3D_FLOAT(tex) Texture3D tex; SamplerState sampler##tex
    #define MIR_DECLARE_TEX3D_HALF(tex) Texture3D tex; SamplerState sampler##tex
#endif

    // 2D arrays
    #define MIR_DECLARE_TEX2DARRAY_MS(tex) Texture2DMSArray<float> tex; SamplerState sampler##tex
    #define MIR_DECLARE_TEX2DARRAY_MS_NOSAMPLER(tex) Texture2DArray<float> tex
    #define MIR_DECLARE_TEX2DARRAY(tex) Texture2DArray tex; SamplerState sampler##tex
    #define MIR_DECLARE_TEX2DARRAY_NOSAMPLER(tex) Texture2DArray tex
    #define MIR_ARGS_TEX2DARRAY(tex) Texture2DArray tex, SamplerState sampler##tex
    #define MIR_PASS_TEX2DARRAY(tex) tex, sampler##tex
    #define MIR_SAMPLE_TEX2DARRAY(tex,coord) tex.Sample (sampler##tex,coord)
    #define MIR_SAMPLE_TEX2DARRAY_LOD(tex,coord,lod) tex.SampleLevel (sampler##tex,coord, lod)
    #define MIR_SAMPLE_TEX2DARRAY_SAMPLER(tex,samplertex,coord) tex.Sample (sampler##samplertex,coord)
    #define MIR_SAMPLE_TEX2DARRAY_SAMPLER_LOD(tex,samplertex,coord,lod) tex.SampleLevel (sampler##samplertex,coord,lod)

    // Cube arrays
    #define MIR_DECLARE_TEXCUBEARRAY(tex) TextureCubeArray tex; SamplerState sampler##tex
    #define MIR_DECLARE_TEXCUBEARRAY_NOSAMPLER(tex) TextureCubeArray tex
    #define MIR_ARGS_TEXCUBEARRAY(tex) TextureCubeArray tex, SamplerState sampler##tex
    #define MIR_PASS_TEXCUBEARRAY(tex) tex, sampler##tex
#if defined(SHADER_API_PSSL)

#else
    #define MIR_SAMPLE_TEXCUBEARRAY(tex,coord) tex.Sample (sampler##tex,coord)
    #define MIR_SAMPLE_TEXCUBEARRAY_LOD(tex,coord,lod) tex.SampleLevel (sampler##tex,coord, lod)
    #define MIR_SAMPLE_TEXCUBEARRAY_SAMPLER(tex,samplertex,coord) tex.Sample (sampler##samplertex,coord)
    #define MIR_SAMPLE_TEXCUBEARRAY_SAMPLER_LOD(tex,samplertex,coord,lod) tex.SampleLevel (sampler##samplertex,coord,lod)
#endif


#else
    // DX9 style HLSL syntax; same object for texture+sampler
    // 2D textures
    #define MIR_DECLARE_TEX2D(tex, slot) sampler2D tex
	#define MIR_ARGS_TEX2D(tex) sampler2D tex
	#define MIR_PASS_TEX2D(tex) tex
    #define MIR_DECLARE_TEX2D_HALF(tex) sampler2D_half tex
    #define MIR_DECLARE_TEX2D_FLOAT(tex) sampler2D_float tex

    #define MIR_DECLARE_TEX2D_NOSAMPLER(tex) sampler2D tex
    #define MIR_DECLARE_TEX2D_NOSAMPLER_HALF(tex) sampler2D_half tex
    #define MIR_DECLARE_TEX2D_NOSAMPLER_FLOAT(tex) sampler2D_float tex

    #define MIR_SAMPLE_TEX2D(tex,coord) tex2D (tex,coord)
    #define MIR_SAMPLE_TEX2D_SAMPLER(tex,samplertex,coord) tex2D (tex,coord)
    // Cubemaps
    #define MIR_DECLARE_TEXCUBE(tex,slot) samplerCUBE tex
    #define MIR_ARGS_TEXCUBE(tex) samplerCUBE tex
    #define MIR_PASS_TEXCUBE(tex) tex
    #define MIR_PASS_TEXCUBE_SAMPLER(tex,samplertex) tex
    #define MIR_DECLARE_TEXCUBE_NOSAMPLER(tex) samplerCUBE tex
    #define MIR_SAMPLE_TEXCUBE(tex,coord) texCUBE (tex,coord)

    #define MIR_SAMPLE_TEXCUBE_LOD(tex,coord,lod) texCUBElod (tex, half4(coord, lod))
    #define MIR_SAMPLE_TEXCUBE_SAMPLER_LOD(tex,samplertex,coord,lod) MIR_SAMPLE_TEXCUBE_LOD(tex,coord,lod)
    #define MIR_SAMPLE_TEXCUBE_SAMPLER(tex,samplertex,coord) texCUBE (tex,coord)

    // 3D textures
    #define MIR_DECLARE_TEX3D(tex) sampler3D tex
    #define MIR_DECLARE_TEX3D_NOSAMPLER(tex) sampler3D tex
    #define MIR_DECLARE_TEX3D_FLOAT(tex) sampler3D_float tex
    #define MIR_DECLARE_TEX3D_HALF(tex) sampler3D_float tex
    #define MIR_SAMPLE_TEX3D(tex,coord) tex3D (tex,coord)
    #define MIR_SAMPLE_TEX3D_LOD(tex,coord,lod) tex3D (tex,float4(coord,lod))
    #define MIR_SAMPLE_TEX3D_SAMPLER(tex,samplertex,coord) tex3D (tex,coord)
    #define MIR_SAMPLE_TEX3D_SAMPLER_LOD(tex,samplertex,coord,lod) tex3D (tex,float4(coord,lod))

    // 2D array syntax for hlsl2glsl and surface shader analysis
    #if defined(MIR_COMPILER_HLSL2GLSL) || defined(SHADER_TARGET_SURFACE_ANALYSIS)
        #define MIR_DECLARE_TEX2DARRAY(tex) sampler2DArray tex
        #define MIR_DECLARE_TEX2DARRAY_NOSAMPLER(tex) sampler2DArray tex
        #define MIR_ARGS_TEX2DARRAY(tex) sampler2DArray tex
        #define MIR_PASS_TEX2DARRAY(tex) tex
        #define MIR_SAMPLE_TEX2DARRAY(tex,coord) tex2DArray (tex,coord)
        #define MIR_SAMPLE_TEX2DARRAY_LOD(tex,coord,lod) tex2DArraylod (tex, float4(coord,lod))
        #define MIR_SAMPLE_TEX2DARRAY_SAMPLER(tex,samplertex,coord) tex2DArray (tex,coord)
        #define MIR_SAMPLE_TEX2DARRAY_SAMPLER_LOD(tex,samplertex,coord,lod) tex2DArraylod (tex, float4(coord,lod))
    #endif

    // surface shader analysis; just pretend that 2D arrays are cubemaps
    #if defined(SHADER_TARGET_SURFACE_ANALYSIS)
        #define sampler2DArray samplerCUBE
        #define tex2DArray texCUBE
        #define tex2DArraylod texCUBElod
    #endif

#endif

// For backwards compatibility, so we won't accidentally break shaders written by user
#define SampleCubeReflection(env, dir, lod) MIR_SAMPLE_TEXCUBE_LOD(env, dir, lod)

#define samplerRECT sampler2D
#define texRECT tex2D
#define texRECTlod tex2Dlod
#define texRECTbias tex2Dbias
#define texRECTproj tex2Dproj

// Data type to be used for "screen space position" pixel shader input semantic; just a float4 now (used to be float2 when on D3D9)
#define MIR_VPOS_TYPE float4

#if defined(MIR_COMPILER_HLSL) || defined (SHADER_TARGET_GLSL)
#define FOGC FOG
#endif

// Use VFACE pixel shader input semantic in your shaders to get front-facing scalar value.
// Requires shader model 3.0 or higher.
// Back when D3D9 existed MIR_VFACE_AFFECTED_BY_PROJECTION macro used to be defined there too.
#if defined(MIR_COMPILER_CG)
#define VFACE FACE
#endif
#if defined(MIR_COMPILER_HLSL2GLSL)
#define FACE VFACE
#endif
#if defined(SHADER_API_PSSL)
#define VFACE S_FRONT_FACE
#endif


#if !defined(SHADER_API_D3D11) && !defined(MIR_COMPILER_HLSLCC) && !defined(SHADER_API_PSSL)
#define SV_POSITION POSITION
#endif


// On D3D reading screen space coordinates from fragment shader requires SM3.0
#define MIR_POSITION(pos) float4 pos : SV_POSITION

// Kept for backwards-compatibility
#define MIR_ATTEN_CHANNEL r

#if defined(SHADER_API_D3D11) || defined(SHADER_API_PSSL) || defined(SHADER_API_METAL) || defined(SHADER_API_VULKAN) || defined(SHADER_API_SWITCH)
#define MIR_UV_STARTS_AT_TOP 1
#endif

#if defined(SHADER_API_D3D11) || defined(SHADER_API_PSSL) || defined(SHADER_API_XBOXONE) || defined(SHADER_API_METAL) || defined(SHADER_API_VULKAN) || defined(SHADER_API_SWITCH)
// D3D style platforms where clip space z is [0, 1].
#define MIR_REVERSED_Z 1
#endif

#if defined(MIR_REVERSED_Z)
#define MIR_NEAR_CLIP_VALUE (1.0)
#else
#define MIR_NEAR_CLIP_VALUE (-1.0)
#endif

// "platform caps" defines that were moved to editor, so they are set automatically when compiling shader
// MIR_NO_DXT5nm              - no DXT5NM support, so normal maps will encoded in rgb
// MIR_NO_RGBM                - no RGBM support, so doubleLDR
// MIR_NO_SCREENSPACE_SHADOWS - no screenspace cascaded shadowmaps
// MIR_FRAMEBUFFER_FETCH_AVAILABLE    - framebuffer fetch
// MIR_ENABLE_REFLECTION_BUFFERS - render reflection probes in deferred way, when using deferred shading


// On most platforms, use floating point render targets to store depth of point
// light shadowmaps. However, on some others they either have issues, or aren't widely
// supported; in which case fallback to encoding depth into RGBA channels.
// Make sure this define matches GraphicsCaps.useRGBAForPointShadows.
#if defined(SHADER_API_GLES) || defined(SHADER_API_GLES3)
#define MIR_USE_RGBA_FOR_POINT_SHADOWS
#endif


// Initialize arbitrary structure with zero values.
// Not supported on some backends (e.g. Cg-based particularly with nested structs).
// hlsl2glsl would almost support it, except with structs that have arrays -- so treat as not supported there either :(
#if defined(MIR_COMPILER_HLSL) || defined(SHADER_API_PSSL) || defined(MIR_COMPILER_HLSLCC)
#define MIR_INITIALIZE_OUTPUT(type,name) name = (type)0;
#else
#define MIR_INITIALIZE_OUTPUT(type,name)
#endif

#if defined(SHADER_API_D3D11) || defined(SHADER_API_GLES3) || defined(SHADER_API_GLCORE) || defined(SHADER_API_VULKAN) || defined(SHADER_API_METAL) || defined(SHADER_API_PSSL)
#define MIR_CAN_COMPILE_TESSELLATION 1
#   define MIR_domain                 domain
#   define MIR_partitioning           partitioning
#   define MIR_outputtopology         outputtopology
#   define MIR_patchconstantfunc      patchconstantfunc
#   define MIR_outputcontrolpoints    outputcontrolpoints
#endif

// Not really needed anymore, but did ship in Unity 4.0; with D3D11_9X remapping them to .r channel.
// Now that's not used.
#define MIR_SAMPLE_1CHANNEL(x,y) tex2D(x,y).a
#define MIR_ALPHA_CHANNEL a


// HLSL attributes
#if defined(MIR_COMPILER_HLSL)
    #define MIR_BRANCH    [branch]
    #define MIR_FLATTEN   [flatten]
    #define MIR_UNROLL    [unroll]
    #define MIR_LOOP      [loop]
    #define MIR_FASTOPT   [fastopt]
#else
    #define MIR_BRANCH
    #define MIR_FLATTEN
    #define MIR_UNROLL
    #define MIR_LOOP
    #define MIR_FASTOPT
#endif


// Unity 4.x shaders used to mostly work if someone used WPOS semantic,
// which was accepted by Cg. The correct semantic to use is "VPOS",
// so define that so that old shaders keep on working.
#if !defined(MIR_COMPILER_CG)
#define WPOS VPOS
#endif

// define use to identify platform with modern feature like texture 3D with filtering, texture array etc...
#define MIR_SM40_PLUS_PLATFORM (!((SHADER_TARGET < 30) || defined (SHADER_API_MOBILE) || defined(SHADER_API_GLES)))

// Ability to manually set descriptor set and binding numbers (Vulkan only)
#if defined(SHADER_API_VULKAN)
    #define CBUFFER_START_WITH_BINDING(Name, Set, Binding) CBUFFER_START(Name##Xhlslcc_set_##Set##_bind_##Binding##X)
    // Sampler / image declaration with set/binding decoration
    #define DECL_WITH_BINDING(Type, Name, Set, Binding) Type Name##hlslcc_set_##Set##_bind_##Binding
#else
    #define CBUFFER_START_WITH_BINDING(Name, Set, Binding) CBUFFER_START(Name)
    #define DECL_WITH_BINDING(Type, Name, Set, Binding) Type Name
#endif

// TODO: Really need a better define for iOS Metal than the framebuffer fetch one, that's also enabled on android and webgl (???)
#if defined(SHADER_API_VULKAN) || (defined(SHADER_API_METAL) && defined(MIR_FRAMEBUFFER_FETCH_AVAILABLE))
// Renderpass inputs: Vulkan/Metal subpass input
#define MIR_DECLARE_FRAMEBUFFER_INPUT_FLOAT(idx) cbuffer hlslcc_SubpassInput_f_##idx { float4 hlslcc_fbinput_##idx; }
#define MIR_DECLARE_FRAMEBUFFER_INPUT_FLOAT_MS(idx) cbuffer hlslcc_SubpassInput_F_##idx { float4 hlslcc_fbinput_##idx[8]; }
// For halfs
#define MIR_DECLARE_FRAMEBUFFER_INPUT_HALF(idx) cbuffer hlslcc_SubpassInput_h_##idx { half4 hlslcc_fbinput_##idx; }
#define MIR_DECLARE_FRAMEBUFFER_INPUT_HALF_MS(idx) cbuffer hlslcc_SubpassInput_H_##idx { half4 hlslcc_fbinput_##idx[8]; }
// For ints
#define MIR_DECLARE_FRAMEBUFFER_INPUT_INT(idx) cbuffer hlslcc_SubpassInput_i_##idx { int4 hlslcc_fbinput_##idx; }
#define MIR_DECLARE_FRAMEBUFFER_INPUT_INT_MS(idx) cbuffer hlslcc_SubpassInput_I_##idx { int4 hlslcc_fbinput_##idx[8]; }
// For uints
#define MIR_DECLARE_FRAMEBUFFER_INPUT_UINT(idx) cbuffer hlslcc_SubpassInput_u_##idx { uint4 hlslcc_fbinput_##idx; }
#define MIR_DECLARE_FRAMEBUFFER_INPUT_UINT_MS(idx) cbuffer hlslcc_SubpassInput_U_##idx { uint4 hlslcc_fbinput_##idx[8]; }

#define MIR_READ_FRAMEBUFFER_INPUT(idx, v2fname) hlslcc_fbinput_##idx
#define MIR_READ_FRAMEBUFFER_INPUT_MS(idx, sampleIdx, v2fname) hlslcc_fbinput_##idx[sampleIdx]


#else
// Renderpass inputs: General fallback path
#define MIR_DECLARE_FRAMEBUFFER_INPUT_FLOAT(idx) MIR_DECLARE_TEX2D_NOSAMPLER_FLOAT(_UnityFBInput##idx); float4 _UnityFBInput##idx##_TexelSize
#define MIR_DECLARE_FRAMEBUFFER_INPUT_HALF(idx) MIR_DECLARE_TEX2D_NOSAMPLER_HALF(_UnityFBInput##idx); float4 _UnityFBInput##idx##_TexelSize
#define MIR_DECLARE_FRAMEBUFFER_INPUT_INT(idx) MIR_DECLARE_TEX2D_NOSAMPLER_INT(_UnityFBInput##idx); float4 _UnityFBInput##idx##_TexelSize
#define MIR_DECLARE_FRAMEBUFFER_INPUT_UINT(idx) MIR_DECLARE_TEX2D_NOSAMPLER_UINT(_UnityFBInput##idx); float4 _UnityFBInput##idx##_TexelSize

#define MIR_READ_FRAMEBUFFER_INPUT(idx, v2fvertexname) _UnityFBInput##idx.Load(uint3(v2fvertexname.xy, 0))

// MSAA input framebuffers via tex2dms

#define MIR_DECLARE_FRAMEBUFFER_INPUT_FLOAT_MS(idx) Texture2DMS<float4> _UnityFBInput##idx; float4 _UnityFBInput##idx##_TexelSize
#define MIR_DECLARE_FRAMEBUFFER_INPUT_HALF_MS(idx) Texture2DMS<float4> _UnityFBInput##idx; float4 _UnityFBInput##idx##_TexelSize
#define MIR_DECLARE_FRAMEBUFFER_INPUT_INT_MS(idx) Texture2DMS<int4> _UnityFBInput##idx; float4 _UnityFBInput##idx##_TexelSize
#define MIR_DECLARE_FRAMEBUFFER_INPUT_UINT_MS(idx) Texture2DMS<uint4> _UnityFBInput##idx; float4 _UnityFBInput##idx##_TexelSize

#define MIR_READ_FRAMEBUFFER_INPUT_MS(idx, sampleIdx, v2fvertexname) _UnityFBInput##idx.Load(uint2(v2fvertexname.xy), sampleIdx)

#endif

// ---- Shader keyword backwards compatibility
// We used to have some built-in shader keywords, but they got removed at some point to save on shader keyword count.
// However some existing shader code might be checking for the old names, so define them as regular
// macros based on other criteria -- so that existing code keeps on working.

// Unity 5.0 renamed HDR_LIGHT_PREPASS_ON to MIR_HDR_ON
#if defined(MIR_HDR_ON)
#define HDR_LIGHT_PREPASS_ON 1
#endif

// MIR_NO_LINEAR_COLORSPACE was removed in 5.4 when MIR_COLORSPACE_GAMMA was introduced as a platform keyword and runtime gamma fallback removed.
#if !defined(MIR_NO_LINEAR_COLORSPACE) && defined(MIR_COLORSPACE_GAMMA)
#define MIR_NO_LINEAR_COLORSPACE 1
#endif

#if !defined(DIRLIGHTMAP_OFF) && !defined(DIRLIGHTMAP_COMBINED)
#define DIRLIGHTMAP_OFF 1
#endif

#if !defined(LIGHTMAP_OFF) && !defined(LIGHTMAP_ON)
#define LIGHTMAP_OFF 1
#endif

#if !defined(DYNAMICLIGHTMAP_OFF) && !defined(DYNAMICLIGHTMAP_ON)
#define DYNAMICLIGHTMAP_OFF 1
#endif


#if defined(MIR_STEREO_INSTANCING_ENABLED) || defined(MIR_STEREO_MULTIVIEW_ENABLED)

    #undef MIR_DECLARE_DEPTH_TEXTURE_MS
    #define MIR_DECLARE_DEPTH_TEXTURE_MS(tex)  MIR_DECLARE_TEX2DARRAY_MS (tex)

    #undef MIR_DECLARE_DEPTH_TEXTURE
    #define MIR_DECLARE_DEPTH_TEXTURE(tex) MIR_DECLARE_TEX2DARRAY (tex)

    #undef SAMPLE_DEPTH_TEXTURE
    #define SAMPLE_DEPTH_TEXTURE(sampler, uv) MIR_SAMPLE_TEX2DARRAY(sampler, float3((uv).x, (uv).y, (float)MIR_StereoEyeIndex)).r

    #undef SAMPLE_DEPTH_TEXTURE_PROJ
    #define SAMPLE_DEPTH_TEXTURE_PROJ(sampler, uv) MIR_SAMPLE_TEX2DARRAY(sampler, float3((uv).x/(uv).w, (uv).y/(uv).w, (float)MIR_StereoEyeIndex)).r

    #undef SAMPLE_DEPTH_TEXTURE_LOD
    #define SAMPLE_DEPTH_TEXTURE_LOD(sampler, uv) MIR_SAMPLE_TEX2DARRAY_LOD(sampler, float3((uv).xy, (float)MIR_StereoEyeIndex), (uv).w).r

    #undef SAMPLE_RAW_DEPTH_TEXTURE
    #define SAMPLE_RAW_DEPTH_TEXTURE(tex, uv) MIR_SAMPLE_TEX2DARRAY(tex, float3((uv).xy, (float)MIR_StereoEyeIndex))

    #undef SAMPLE_RAW_DEPTH_TEXTURE_PROJ
    #define SAMPLE_RAW_DEPTH_TEXTURE_PROJ(sampler, uv) MIR_SAMPLE_TEX2DARRAY(sampler, float3((uv).x/(uv).w, (uv).y/(uv).w, (float)MIR_StereoEyeIndex))

    #undef SAMPLE_RAW_DEPTH_TEXTURE_LOD
    #define SAMPLE_RAW_DEPTH_TEXTURE_LOD(sampler, uv) MIR_SAMPLE_TEX2DARRAY_LOD(sampler, float3((uv).xy, (float)MIR_StereoEyeIndex), (uv).w)

    #define MIR_DECLARE_SCREENSPACE_SHADOWMAP MIR_DECLARE_TEX2DARRAY
    #define MIR_SAMPLE_SCREEN_SHADOW(tex, uv) MIR_SAMPLE_TEX2DARRAY( tex, float3((uv).x/(uv).w, (uv).y/(uv).w, (float)MIR_StereoEyeIndex) ).r

    #define MIR_DECLARE_SCREENSPACE_TEXTURE MIR_DECLARE_TEX2DARRAY
    #define MIR_SAMPLE_SCREENSPACE_TEXTURE(tex, uv) MIR_SAMPLE_TEX2DARRAY(tex, float3((uv).xy, (float)MIR_StereoEyeIndex))
#else
    #define MIR_DECLARE_DEPTH_TEXTURE_MS(tex)  Texture2DMS<float> tex;
    #define MIR_DECLARE_DEPTH_TEXTURE(tex) sampler2D_float tex
    #define MIR_DECLARE_SCREENSPACE_SHADOWMAP(tex) sampler2D tex
    #define MIR_SAMPLE_SCREEN_SHADOW(tex, uv) tex2Dproj( tex, MIR_PROJ_COORD(uv) ).r
    #define MIR_DECLARE_SCREENSPACE_TEXTURE(tex) sampler2D tex;
    #define MIR_SAMPLE_SCREENSPACE_TEXTURE(tex, uv) tex2D(tex, uv)
#endif

#endif // HLSL_SUPPORT_INCLUDED
