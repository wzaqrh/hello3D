/********** PBR **********/
#include "Standard.h"
#include "Skeleton.h"
#include "Lighting.h"

#if SHADER_MODEL > 30000
//Texture2D txAlbedo : register(t0);//rgb
#define txAlbedo txMain
Texture2D txNormal : register(t1);//rgb
Texture2D txMetalness : register(t2);//r
Texture2D txSmoothness : register(t3);//r
Texture2D txAmbientOcclusion : register(t4);//r
#else
texture  textureAlbedo : register(t0);
sampler2D txAlbedo : register(s0) = sampler_state { 
	Texture = <textureAlbedo>; 
};

texture  textureNormal : register(t1);
sampler2D txNormal : register(s1) = sampler_state { 
	Texture = <textureNormal>; 
};

texture  textureMetalness : register(t2);
sampler2D txMetalness : register(s2) = sampler_state { 
	Texture = <textureMetalness>; 
};

texture  textureSmoothness : register(t3);
sampler2D txSmoothness : register(s3) = sampler_state { 
	Texture = <textureSmoothness>;
};

texture  textureAmbientOcclusion : register(t4);
sampler2D txAmbientOcclusion : register(s4) = sampler_state { 
	Texture = <textureAmbientOcclusion>; 	
};
#endif


cbuffer cbUnityMaterial : register(b3)
{
	float4 _SpecColor;
	float4 _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	int _SpecLightOff;
}

cbuffer cbUnityGlobal : register(b4)
{
	float4 _Unity_IndirectSpecColor;
	float4 _AmbientOrLightmapUV;
	float4 _Unity_SpecCube0_HDR;
};

/************ ShadowCaster ************/
struct ShadowCasterPixelInput
{
    float4 Pos : SV_POSITION;
};

ShadowCasterPixelInput VSShadowCaster(vbSurface surf, vbWeightedSkin skin)
{
	ShadowCasterPixelInput output;
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos, 1.0));
	matrix MW = mul(World, transpose(Model));
	matrix MWVP = mul(Projection, mul(View, MW));
	output.Pos = mul(MWVP, skinPos);	
	return output;
}

float4 PSShadowCaster(ShadowCasterPixelInput i) : SV_Target
{
	return float4(0.467,0.533,0.6,1);
}

/************ ForwardBase ************/
struct PixelInput
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;//world space
	float3 Tangent : NORMAL1;//world space
	float3 BiTangent : NORMAL2;//world space
	float3 Eye : TEXCOORD1;//world space
	float3 SurfacePosition : TEXCOORD2;//world space
	float3x3 TangentBasis : TEXCOORD4;
	float4 PosInLight : TEXCOORD3;//world space
};

PixelInput VS(vbSurface surf, vbWeightedSkin skin)
{
	PixelInput output = (PixelInput)0;

	matrix MW = mul(World, transpose(Model));
	//matrix MWV = mul(View, MW);
	
	output.Tangent = normalize(mul(MW, Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Tangent.xyz, 0.0))).xyz);
	output.BiTangent = normalize(mul(MW, Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.BiTangent.xyz, 0.0))).xyz);
	output.Normal = normalize(mul(MW, Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0))).xyz);
	
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos, 1.0));
	output.Pos = mul(MW, skinPos);
	output.SurfacePosition = output.Pos.xyz;
	
	matrix LightMWVP = mul(LightProjection,mul(LightView, MW));
	output.PosInLight = mul(LightMWVP, skinPos);
	
	output.Pos = mul(View, output.Pos);	
	output.Eye = mul(ViewInv, float4(0.0,0.0,0.0,1.0)).xyz;
    
	output.Pos = mul(Projection, output.Pos);
    
	float3x3 TBN = float3x3(skin.Tangent, skin.BiTangent, skin.Normal);
	output.TangentBasis = mul((float3x3)MW, transpose(TBN));
	
	output.Tex = surf.Tex;
    return output;
}

#define SHADER_TARGET 30
//#define _ALPHAPREMULTIPLY_ON
#define UNITY_SETUP_BRDF_INPUT MetallicSetup
//#define UNITY_SETUP_BRDF_INPUT SpecularSetup
#define UNITY_COLORSPACE_GAMMA
#define _SPECGLOSSMAP

/*FragmentSetup*/
struct FragmentCommonData
{
    float3 diffColor, specColor;
    float oneMinusReflectivity, smoothness;
    float alpha;
};
float4 SpecularGloss(float2 uv)//float4(_SpecColor.rgb, tex2D(_MainTex, uv).a * _GlossMapScale)
{
    float4 sg;
#ifdef _SPECGLOSSMAP//Shader='Standard(Specular setup)' -> 'Specular'=$texture
    #if defined(_SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A)
    #else
		sg.rgb = GetTexture2D(txMetalness, samLinear, uv).rgb;
		sg.a = GetTexture2D(txSmoothness, samLinear, uv).r;
    #endif
    sg.a *= _GlossMapScale;//Shader='Standard' -> 'Smoothness Scale'=[0,1]
#else
#endif
    return sg;
}
float3 Albedo(float2 i_tex)//_Color.rgb * tex2D(_MainTex, texcoords.xy).rgb
{
	return _Color.rgb * GetTexture2D(txAlbedo, samLinear, i_tex).rgb;
}
float SpecularStrength(float3 specular)//max(specular.rgb)
{
#if (SHADER_TARGET < 30)
    return specular.r;
#else
    return max (max (specular.r, specular.g), specular.b);
#endif
}
float3 EnergyConservationBetweenDiffuseAndSpecular(float3 albedo, float3 specColor, out float oneMinusReflectivity)//albedo*(1-max(specColor.rgb)),1-max(specColor.rgb)
{
    oneMinusReflectivity = 1 - SpecularStrength(specColor);
    return albedo * oneMinusReflectivity;
}
FragmentCommonData SpecularSetup(float2 i_tex)
{
	//SpecularGloss
	float4 specGloss = SpecularGloss(i_tex);
    float3 specColor = specGloss.rgb;
    float smoothness = specGloss.a;

	//EnergyConservationBetweenDiffuseAndSpecular
    float oneMinusReflectivity;
    float3 diffColor = EnergyConservationBetweenDiffuseAndSpecular(Albedo(i_tex), specColor, /*out*/ oneMinusReflectivity);//diffuseBRDF

    FragmentCommonData o = (FragmentCommonData)0;
    o.diffColor = diffColor;
    o.specColor = specColor;
    o.oneMinusReflectivity = oneMinusReflectivity;
    o.smoothness = smoothness;
    return o;
}

float2 MetallicGloss(float2 uv)
{
    float2 mg;
	//_METALLICGLOSSMAP
    #ifdef _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
    #else
        mg.x = GetTexture2D(txMetalness, samLinear, uv).r;//_MetallicGlossMap("Metallic", 2D) = "white" {}
		mg.y = GetTexture2D(txSmoothness, samLinear, uv).r;
    #endif
	mg.y *= _GlossMapScale;
	//_METALLICGLOSSMAP
    return mg;
}
#ifdef UNITY_COLORSPACE_GAMMA
#define unity_ColorSpaceDielectricSpec float4(0.220916301, 0.220916301, 0.220916301, 1.0 - 0.220916301)
#else // Linear values
#define unity_ColorSpaceDielectricSpec float4(0.04, 0.04, 0.04, 1.0 - 0.04) // standard dielectric reflectivity coef at incident angle (= 4%)
#endif
float OneMinusReflectivityFromMetallic(float metallic)
{
    // We'll need oneMinusReflectivity, so
    //   1-reflectivity = 1-lerp(dielectricSpec, 1, metallic) = lerp(1-dielectricSpec, 0, metallic)
    // store (1-dielectricSpec) in unity_ColorSpaceDielectricSpec.a, then
    //   1-reflectivity = lerp(alpha, 0, metallic) = alpha + metallic*(0 - alpha) =
    //                  = alpha - metallic * alpha
    float oneMinusDielectricSpec = unity_ColorSpaceDielectricSpec.a;
    return oneMinusDielectricSpec - metallic * oneMinusDielectricSpec;
}
float3 DiffuseAndSpecularFromMetallic(float3 albedo, float metallic, out float3 specColor, out float oneMinusReflectivity)
{
    specColor = lerp(unity_ColorSpaceDielectricSpec.rgb, albedo, metallic);
    oneMinusReflectivity = OneMinusReflectivityFromMetallic(metallic);
    return albedo * oneMinusReflectivity;
}
FragmentCommonData MetallicSetup(float2 i_tex)
{
    float2 metallicGloss = MetallicGloss(i_tex.xy);
    float metallic = metallicGloss.x;
    float smoothness = metallicGloss.y; // this is 1 minus the square root of real roughness m.

    float oneMinusReflectivity;
    float3 specColor;
    float3 diffColor = DiffuseAndSpecularFromMetallic(Albedo(i_tex), metallic, /*out*/ specColor, /*out*/ oneMinusReflectivity);

    FragmentCommonData o = (FragmentCommonData)0;
    o.diffColor = diffColor;
    o.specColor = specColor;
    o.oneMinusReflectivity = oneMinusReflectivity;
    o.smoothness = smoothness;
    return o;
}
float Alpha(float2 uv)//_Color.a
{
#if defined(_SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A)
    return _Color.a;
#else
    return GetTexture2D(txAlbedo, samLinear, uv).a * _Color.a;
#endif
}
float3 PreMultiplyAlpha(float3 diffColor, float alpha, float oneMinusReflectivity, out float outModifiedAlpha)//diffColor*alpha,1-oneMinusReflectivity + alpha*oneMinusReflectivity
{
    #if defined(_ALPHAPREMULTIPLY_ON)
        diffColor *= alpha;
        #if (SHADER_TARGET < 30)
            outModifiedAlpha = alpha;
        #else
            outModifiedAlpha = 1-oneMinusReflectivity + alpha*oneMinusReflectivity;
        #endif
    #else
        outModifiedAlpha = alpha;
    #endif
    return diffColor;
}
FragmentCommonData FragmentSetup(float2 i_tex)
{
    float alpha = Alpha(i_tex.xy);//_Color.a
    FragmentCommonData o = UNITY_SETUP_BRDF_INPUT(i_tex);
    o.diffColor = PreMultiplyAlpha(o.diffColor, alpha, o.oneMinusReflectivity, /*out*/o.alpha);
    return o;
}

/*MainLight*/
struct UnityLight
{
    float3 color;
};
UnityLight MainLight()//{color=_LightColor0.rgb,dir=_WorldSpaceLightPos0.xyz}
{
    UnityLight l;
    l.color = unity_LightColor.rgb;
    return l;
}

/*Occlusion*/
float LerpOneTo(float b, float t)
{
    float oneMinusT = 1 - t;
    return oneMinusT + b * t;
}
float Occlusion(float2 uv)//1-_OcclusionStrength + tex2D(_OcclusionMap, uv).g*_OcclusionStrength
{
#if (SHADER_TARGET < 30)
    return GetTexture2D(txAmbientOcclusion, samLinear, uv).g;
#else
    float occ = GetTexture2D(txAmbientOcclusion, samLinear, uv).g;
    return LerpOneTo(occ, _OcclusionStrength);
#endif
}

/*FragmentGI*/
struct UnityIndirect
{
    float3 diffuse;
    float3 specular;
};
struct UnityGI
{
    UnityLight light;
    UnityIndirect indirect;
};
struct UnityGIInput
{
    UnityLight light; // pixel light, sent from the engine
    float3 ambient;
};
struct Unity_GlossyEnvironmentData
{
    float roughness;
	float3 reflUVW;
};
float SmoothnessToPerceptualRoughness(float smoothness)//1-smoothness
{
    return (1 - smoothness);
}
Unity_GlossyEnvironmentData UnityGlossyEnvironmentSetup(float Smoothness)//{roughness=1.0-Smoothness}
{
    Unity_GlossyEnvironmentData g;
    g.roughness = SmoothnessToPerceptualRoughness(Smoothness);
    return g;
}
/*{light={color=data.light.color*data.atten}}*/
UnityGI UnityGI_Base(UnityGIInput data, float occlusion)
{
    UnityGI o_gi;
    o_gi.light = data.light;
	
    o_gi.indirect.diffuse = 0.0;
    o_gi.indirect.specular = 0.0;
	
    o_gi.indirect.diffuse *= occlusion;
    return o_gi;
}
#define UNITY_ARGS_TEXCUBE(tex) TextureCube tex, SamplerState sampler##tex
#define UNITY_PASS_TEXCUBE(tex) tex, sampler##tex
#define UNITY_PASS_TEXCUBE_SAMPLER(tex,samplertex) tex, sampler##samplertex
#define UNITY_SAMPLE_TEXCUBE_LOD(tex,coord,lod) tex.SampleLevel (sampler##tex,coord, lod)

#ifndef UNITY_SPECCUBE_LOD_STEPS
#define UNITY_SPECCUBE_LOD_STEPS (6)
#endif
float perceptualRoughnessToMipmapLevel(float perceptualRoughness)
{
    return perceptualRoughness * UNITY_SPECCUBE_LOD_STEPS;
}
float3 DecodeHDR (float4 data, float4 decodeInstructions)
{
    float alpha = decodeInstructions.w * (data.a - 1.0) + 1.0;
    #if defined(UNITY_COLORSPACE_GAMMA)
        return (decodeInstructions.x * alpha) * data.rgb;
    #else
    #endif
}
float3 Unity_GlossyEnvironment(Unity_GlossyEnvironmentData glossIn)
{
    float perceptualRoughness = glossIn.roughness /* perceptualRoughness */ ;

// TODO: CAUTION: remap from Morten may work only with offline convolution, see impact with runtime convolution!
// For now disabled
#if 0
    float m = PerceptualRoughnessToRoughness(perceptualRoughness); // m is the real roughness parameter
    const float fEps = 1.192092896e-07F;        // smallest such that 1.0+FLT_EPSILON != 1.0  (+1e-4h is NOT good here. is visibly very wrong)
    float n =  (2.0/max(fEps, m*m))-2.0;        // remap to spec power. See eq. 21 in --> https://dl.dropboxusercontent.com/u/55891920/papers/mm_brdf.pdf

    n /= 4;                                     // remap from n_dot_h formulatino to n_dot_r. See section "Pre-convolved Cube Maps vs Path Tracers" --> https://s3.amazonaws.com/docs.knaldtech.com/knald/1.0.0/lys_power_drops.html

    perceptualRoughness = pow( 2/(n+2), 0.25);      // remap back to square root of real roughness (0.25 include both the sqrt root of the conversion and sqrt for going from roughness to perceptualRoughness)
#else
    // MM: came up with a surprisingly close approximation to what the #if 0'ed out code above does.
    perceptualRoughness = perceptualRoughness*(1.7 - 0.7*perceptualRoughness);
#endif

    float mip = perceptualRoughnessToMipmapLevel(perceptualRoughness);
    float3 R = glossIn.reflUVW;
	
    float4 rgbm = GetTextureCubeLevel(txSkybox, samLinear, R, mip);
    //skybox tone mapping
	rgbm.rgb *= (1.0f + rgbm.rgb/1.5f);
	rgbm.rgb /= (1.0f + rgbm.rgb);

	float4 hdr = _Unity_SpecCube0_HDR;
    return DecodeHDR(rgbm, hdr);
}
float3 Unity_GlossyEnvironment(float perceptualRoughness)
{
    Unity_GlossyEnvironmentData g;
    g.roughness /* perceptualRoughness */ = perceptualRoughness;
    return Unity_GlossyEnvironment(g);
}
float3 UnityGI_IndirectSpecular(UnityGIInput data, float occlusion, Unity_GlossyEnvironmentData glossIn)//unity_IndirectSpecColor.rgb*occlusion
{
    float3 specular;

    #ifdef UNITY_SPECCUBE_BOX_PROJECTION
    #endif

    #ifdef _GLOSSYREFLECTIONS_OFF
    #else
        float3 env0 = Unity_GlossyEnvironment(glossIn);
        #ifdef UNITY_SPECCUBE_BLENDING
        #else
            specular = env0;
        #endif
    #endif

    return specular * occlusion;
}
UnityGI UnityGlobalIllumination(UnityGIInput data, float occlusion, Unity_GlossyEnvironmentData glossIn)//std_branch_this
{
    UnityGI o_gi = UnityGI_Base(data, occlusion);
    o_gi.indirect.specular = UnityGI_IndirectSpecular(data, occlusion, glossIn);
    return o_gi;
}
UnityGI FragmentGI(FragmentCommonData s, float occlusion, UnityLight light, float3 normal, float3 eye)
{
    UnityGIInput d;
    d.light = light;
    d.ambient = _AmbientOrLightmapUV.rgb;
    Unity_GlossyEnvironmentData g = UnityGlossyEnvironmentSetup(s.smoothness);
	g.reflUVW = reflect(-eye, normal);
    return UnityGlobalIllumination(d, occlusion, g);
}

/*UNITY_BRDF_PBS*/
float3 Unity_SafeNormalize(float3 inVec)//normalize(inVec)
{
    float dp3 = max(0.001f, dot(inVec, inVec));
    return inVec * rsqrt(dp3);
}
// Note: Disney diffuse must be multiply by diffuseAlbedo / PI. This is done outside of this function.
float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float perceptualRoughness)
{
    float fd90 = 0.5 + 2 * LdotH * LdotH * perceptualRoughness;
    float lightScatter = (1 + (fd90 - 1) * pow(1 - NdotL,5));
    float viewScatter  = (1 + (fd90 - 1) * pow(1 - NdotV,5));
    return lightScatter * viewScatter;
}
float PerceptualRoughnessToRoughness(float perceptualRoughness)//perceptualRoughness^2
{
    return perceptualRoughness * perceptualRoughness;
}
// Ref: http://jcgt.org/published/0003/02/03/paper.pdf
float SmithJointGGXVisibilityTerm(float NdotL, float NdotV, float roughness)//function G
{
    // Approximation of the above formulation (simplify the sqrt, not mathematically correct but close enough)
    float a = roughness;
    float lambdaV = NdotL * (NdotV * (1 - a) + a);
    float lambdaL = NdotV * (NdotL * (1 - a) + a);
    return 0.5f / (lambdaV + lambdaL + 1e-4f); // work-around against hlslcc rounding error
}
#define UNITY_INV_PI 0.31830988618f
#define UNITY_PI 3.141592f
float GGXTerm(float NdotH, float roughness)//function D
{
    float a2 = roughness * roughness;
    float d = (NdotH * a2 - NdotH) * NdotH + 1.0f; // 2 mad
    return UNITY_INV_PI * a2 / (d * d + 1e-7f); // This function is not intended to be running on Mobile,
}
float3 FresnelTerm(float3 F0, float cosA)
{
    float t = pow(1 - cosA, 5);   // ala Schlick interpoliation
    return F0 + (1-F0) * t;
}
float3 FresnelLerp(float3 F0, float3 F90, float cosA)
{
    float t = pow(1 - cosA, 5);   // ala Schlick interpoliation
    return lerp(F0, F90, t);
}
float3 UNITY_BRDF_PBS(FragmentCommonData s, UnityGI gi, float3 normal, float3 toLight, float3 toEye, float2 texcoor)
{
    float perceptualRoughness = SmoothnessToPerceptualRoughness(s.smoothness);//1-smoothness
    float3 halfDir = Unity_SafeNormalize(toLight + toEye);//H

    float nv = abs(dot(normal, toEye));    // This abs allow to limit artifact

    float nl = saturate(dot(normal, toLight));
    float nh = saturate(dot(normal, halfDir));

    float lv = saturate(dot(toLight, toEye));
    float lh = saturate(dot(toLight, halfDir));

    // Diffuse term
    float diffuseTerm = DisneyDiffuse(nv, nl, lh, perceptualRoughness) * nl;

    // Specular term
    float roughness = PerceptualRoughnessToRoughness(perceptualRoughness);
#define UNITY_BRDF_GGX 1   
#if UNITY_BRDF_GGX
	roughness = max(roughness, 0.002);
    float V = SmithJointGGXVisibilityTerm(nl, nv, roughness);
    float D = GGXTerm (nh, roughness);
#endif
    float specularTerm = V * D * UNITY_PI; // Torrance-Sparrow model, Fresnel is applied later

#ifdef UNITY_COLORSPACE_GAMMA//Edit -> Project Settings -> Player -> Other Settings //std_enabled
    specularTerm = sqrt(max(1e-4h, specularTerm));
#endif

    specularTerm = max(0, specularTerm * nl);
#if defined(_SPECULARHIGHLIGHTS_OFF)
	specularTerm = 0.0;
#else
	if (_SpecLightOff > 0)
		specularTerm = 0.0;
#endif

    float surfaceReduction;
#ifdef UNITY_COLORSPACE_GAMMA
    surfaceReduction = 1.0-0.28*roughness*perceptualRoughness;      // 1-0.28*x^3 as approximation for (1/(x^4+1))^(1/2.2) on the domain [0;1]
#else
    surfaceReduction = 1.0 / (roughness*roughness + 1.0);           // fade \in [0.5;1]
#endif

    specularTerm *= any(s.specColor) ? 1.0 : 0.0;

    float grazingTerm = saturate(s.smoothness + (1-s.oneMinusReflectivity));
    float3 color = s.diffColor * (gi.indirect.diffuse + gi.light.color * diffuseTerm)
                 + specularTerm * gi.light.color * FresnelTerm(s.specColor, lh)
                 + surfaceReduction * gi.indirect.specular * FresnelLerp(s.specColor, grazingTerm, nv);
	/*
    half grazingTerm = saturate(smoothness + (1-oneMinusReflectivity));
    half3 color =   diffColor * (gi.diffuse + light.color * diffuseTerm)
                    + specularTerm * light.color * FresnelTerm (specColor, lh)
                    + surfaceReduction * gi.specular * FresnelLerp (specColor, grazingTerm, nv);
	*/


	//color = s.diffColor;
	//color = s.diffColor * (gi.indirect.diffuse + gi.light.color * diffuseTerm);
	//color = specularTerm * gi.light.color * FresnelTerm(s.specColor, lh) + surfaceReduction * gi.indirect.specular * FresnelLerp(s.specColor, grazingTerm, nv);
    return color;
}

float3 CalLight(float3 toLight, float3 normal, float3 toEye, float2 texcoord, bool spotLight, bool forwardAdd) 
{
	float lengthSq = max(dot(toLight, toLight), 0.000001);
	toLight *= rsqrt(lengthSq);
	
	FragmentCommonData s = FragmentSetup(texcoord);
	UnityLight mainLight = MainLight();
	float occlusion = Occlusion(texcoord);
	UnityGI gi = FragmentGI(s, occlusion, mainLight, normal, toEye);
	if (forwardAdd) {
		gi.indirect.diffuse = 0.0;
		gi.indirect.specular = 0.0;
	}
	float3 c = UNITY_BRDF_PBS(s, gi, normal, toLight, toEye, texcoord);
	//OutputForward
	

	float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
    if (spotLight) {
        float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
        float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
        atten *= saturate(spotAtt);
    }
	
	return c * atten;
}

float3x3 CalTBN(float3 normal, float3 tangent, float3 bitangent) {
	//float3 n = normalize(normal);
	//float3 t = normalize(tangent - normal * dot(normal,tangent));
	//float3 b = normalize(cross(n,t));
	float3 n = normalize(normal);
	float3 t = normalize(tangent);
	float3 b = normalize(bitangent);
	return float3x3(t,b,n);
}
float3 GetBumpBySampler(float3x3 tbn, float2 texcoord) {
	float3 bump = GetTexture2D(txNormal, samLinear, texcoord).xyz * 2.0 - 1.0;
	bump = mul(bump, tbn);
	return bump;
}

float4 PS(PixelInput input) : SV_Target
{	
	float3 toEye = normalize(input.Eye - input.SurfacePosition);
	
	float3 normal;
	if (hasNormal > 0) {
		//float3x3 tbn = CalTBN(input.Normal, input.Tangent, input.BiTangent);
		//normal = GetBumpBySampler(tbn, input.Tex);
		float3 rawNormal = GetTexture2D(txNormal, samLinear, input.Tex).xyz;
		normal = normalize(2.0 * rawNormal - 1.0);
		normal = normalize(mul(input.TangentBasis, normal));
	}
	else {
		normal = normalize(input.Normal);
	}
	
	float4 finalColor;
	float3 toLight = unity_LightPosition.xyz - input.SurfacePosition * unity_LightPosition.w;
	finalColor.xyz = CalLight(toLight, normalize(input.Normal), toEye, input.Tex, LightType == 3, false);
	finalColor.w = 1.0;
	
	//finalColor.rgb *= CalLightStrengthWithShadow(input.PosInLight);
	finalColor.rgb *= CalcShadowFactor(samShadow, txDepthMap, input.PosInLight);
#if 0
	{
		float3 ao;
		if (hasAO)
			ao = GetTexture2D(txAmbientOcclusion, samLinear, input.Tex).xyz;
		else
			ao = 0.0;
		
		float3 albedo = GetTexture2D(txAlbedo, samLinear, input.Tex).rgb; 
		albedo = pow(albedo, 2.2);
		
		float3 ambient = albedo * ao * 0.03;
		finalColor.xyz += ambient;
	}
#endif
	//float4 c = GetTexture2D(txAlbedo, samLinear, input.Tex).rgba;
	//finalColor.rgb = float4(c.rgb * c.a,c.a);
	
    //finalColor.xyz = finalColor.xyz / (finalColor.xyz + 1.0); // HDR tonemapping
    //finalColor.xyz = pow(finalColor.xyz, 1.0/2.2); // gamma correct
	//finalColor = float4(1.0, 0.0, 0.0, 1.0);
	return finalColor;
}

/************ ForwardAdd ************/
float4 PSAdd(PixelInput input) : SV_Target
{
	float3 toEye = normalize(input.Eye - input.SurfacePosition);
	//float3 toEye = normalize(float3(0.0,0.0,-150.0) - input.SurfacePosition);
	
	float3 normal;
	if (hasNormal > 0) {
		//float3x3 tbn = CalTBN(input.Normal, input.Tangent, input.BiTangent);
		//normal = GetBumpBySampler(tbn, input.Tex);
		float3 rawNormal = GetTexture2D(txNormal, samLinear, input.Tex).xyz;
		normal = normalize(2.0 * rawNormal - 1.0);
		normal = normalize(mul(input.TangentBasis, normal));
	}
	else {
		normal = normalize(input.Normal);
	}
	
	float4 finalColor;
	float3 toLight = unity_LightPosition.xyz - input.SurfacePosition * unity_LightPosition.w;
	finalColor.xyz = CalLight(toLight, normalize(input.Normal), toEye, input.Tex, LightType == 3, true);
	finalColor.w = 1.0;
		
	return finalColor;
}