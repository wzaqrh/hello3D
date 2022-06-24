#include "Standard.cginc"
#include "Skeleton.cginc"
#include "CommonFunction.cginc"
#include "Shadow.cginc"
#include "Lighting.cginc"
#include "Lighting.cginc"
#include "ToneMapping.cginc"
#include "Macros.cginc"

MIR_DECLARE_TEX2D(txAlbedo, 0);
MIR_DECLARE_TEX2D(txNormal, 1);
MIR_DECLARE_TEX2D(txMetallic, 2);
MIR_DECLARE_TEX2D(txRoughness, 3);
MIR_DECLARE_TEX2D(txAmbientOcclusion, 4);
MIR_DECLARE_TEX2D(txEmissive, 5);
MIR_DECLARE_TEX2D(txSheen, 6);
MIR_DECLARE_TEX2D(txClearCoat, 7);
#if LIGHTMODE == LIGHTMODE_SHADOW_CASTER_POSTPROCESS
	MIR_DECLARE_TEX2D(PrePassOutput, 9);
#endif

//#define HAS_ATTRIBUTE_NORMAL 1 
//#define HAS_ATTRIBUTE_TANGENT 1 
//#define ENABLE_PIXEL_BTN 1 
//#define ENABLE_NORMAL_MAP 1
//#define ENABLE_METALLIC_X_X_SMOOTHNESS_MAP 1
//#define ENABLE_METALLIC_MAP 1
//#define ENABLE_ROUGHNESS_MAP 1

cbuffer cbModel : register(b3)
{
	float4 AlbedoTransUV;
	float4 NormalTransUV; 
    float4 AoTransUV;
    float4 RoughnessTransUV;
    float4 MetallicTransUV;
	float4 EmissiveTransUV;
	float4 SheenTransUV;
	float4 ClearCoatTransUV;
	
	float4 AlbedoFactor;
	float4 EmissiveFactor;
	float4 SheenColorRoughness;
	float4 ClearCoatRoughness;
	float NormalScale;
    float AoStrength;
    float RoughnessFactor;
    float MetallicFactor;
	float TransmissionFactor;
}

inline float3 GetAlbedo(float2 uv) 
{
    float3 albedo = AlbedoFactor.rgb;
#if ENABLE_ALBEDO_MAP
	float3 color = MIR_SAMPLE_TEX2D(txAlbedo, GetUV(uv, AlbedoTransUV)).rgb;
	#if ALBEDO_MAP_SRGB && (COLORSPACE == COLORSPACE_LINEAR)
		color = sRGBToLinear(color);
	#endif
	albedo *= color;
#endif
    return albedo;
}

inline float3 GetEmissive(float2 uv) 
{
    float3 emissive = EmissiveFactor.rgb;
#if ENABLE_EMISSIVE_MAP
	float3 color = MIR_SAMPLE_TEX2D(txEmissive, GetUV(uv, EmissiveTransUV)).rgb;
	#if EMISSIVE_MAP_SRGB && (COLORSPACE == COLORSPACE_LINEAR)
		color = sRGBToLinear(color);
	#endif
    emissive *= color;
#endif
    return emissive;
}

inline float4 GetSheenColorRoughness(float2 uv)
{
    float4 sheen = SheenColorRoughness;
#if ENABLE_SHEEN_MAP
	float4 color = MIR_SAMPLE_TEX2D(txSheen, GetUV(uv, SheenTransUV));
	#if SHEEN_MAP_SRGB && (COLORSPACE == COLORSPACE_LINEAR)
		color.rgb = sRGBToLinear(color.rgb);
	#endif
    sheen *= color;
#endif
    return sheen;	
}

inline float4 GetClearCoatRoughness(float2 uv)
{
    float4 clearcoat = ClearCoatRoughness;
#if ENABLE_CLEARCOAT_MAP
	float4 color = MIR_SAMPLE_TEX2D(txClearCoat, GetUV(uv, ClearCoatTransUV)).rrrg;
	#if CLEARCOAT_MAP_SRGB && (COLORSPACE == COLORSPACE_LINEAR)
		color.rgb = sRGBToLinear(color.rgb);
	#endif
    clearcoat *= color;
#endif
    return clearcoat;
}

inline float4 GetAoRoughnessMetallicTransmission(float2 uv)
{
    float4 value = float4(1.0, RoughnessFactor, MetallicFactor, TransmissionFactor);
#if ENABLE_AO_ROUGHNESS_METALLIC_MAP
	float4 armt = MIR_SAMPLE_TEX2D(txAmbientOcclusion, GetUV(uv, AoTransUV)).rgba;
	value.x = lerp(1.0, armt.x, AoStrength);
	value.yzw *= armt.yzw;
#elif ENABLE_METALLIC_X_X_SMOOTHNESS_MAP
    #if ENABLE_AO_MAP
		float ao = MIR_SAMPLE_TEX2D(txAmbientOcclusion, GetUV(uv, AoTransUV)).r;
		value.x = 1.0 + (ao - 1.0) * value.x;
	#endif
	float2 ms = MIR_SAMPLE_TEX2D(txMetallic, GetUV(uv, RoughnessTransUV)).ra;
	value.y *= 1.0 - ms.y;
	value.z *= ms.x; 
#elif ENABLE_SPECULAR_MAP
	value = MIR_SAMPLE_TEX2D(txMetallic, GetUV(uv, MetallicTransUV));
#else
    #if ENABLE_AO_MAP
		float ao = MIR_SAMPLE_TEX2D(txAmbientOcclusion, GetUV(uv, AoTransUV)).r;
		value.x = 1.0 + (ao - 1.0) * value.x;
	#endif
    #if ENABLE_ROUGHNESS_MAP 
		value.y *= MIR_SAMPLE_TEX2D(txRoughness, GetUV(uv, RoughnessTransUV)).r;
	#endif
    #if ENABLE_METALLIC_MAP
		value.z *= MIR_SAMPLE_TEX2D(txMetallic, GetUV(uv, MetallicTransUV)).r;
	#endif
#endif
	return value;
}

#if HAS_ATTRIBUTE_NORMAL
	#define SET_WORLD_NORMAL(normal, worldNormal, worldPos) normal = worldNormal;
#else
	#define SET_WORLD_NORMAL(normal, worldNormal, worldPos) float3 dpx = ddx(worldPos); float3 dpy = ddy(worldPos); normal = normalize(cross(dpy, dpx))
#endif
#if ENABLE_NORMAL_MAP
	#if NORMAL_TEXTURE_PACKED
		#define INIT_TANGENT_NORMAL(uv) float3 tangentNormal = UnpackScaleNormalRGorAG(MIR_SAMPLE_TEX2D(txNormal, GetUV(uv, NormalTransUV)), NormalScale)
	#else
		#define INIT_TANGENT_NORMAL(uv) float3 tangentNormal = MIR_SAMPLE_TEX2D(txNormal, GetUV(uv, NormalTransUV)).xyz * 2.0 - 1.0; tangentNormal = normalize(tangentNormal * float3(NormalScale, NormalScale, 1.0))
	#endif
	
	#if !ENABLE_PIXEL_BTN	
		#define MUL_TANGENT_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis) normal = normalize(tangentNormal.x * tangentBasis + tangentNormal.y * bitangentBasis + tangentNormal.z * normal);
    #else
		#if HAS_ATTRIBUTE_TANGENT
			#define MUL_TANGENT_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis) float3x3 tbn = GetTangentToWorldTBN(tangentBasis, normal, GetDpDuv(worldPos, uv)); normal = normalize(mul(tangentNormal, tbn));
		#else
			#define MUL_TANGENT_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis) float3x3 tbn = GetTangentToWorldTBN(normal, GetDpDuv(worldPos, uv)); normal = normalize(mul(tangentNormal, tbn));
		#endif
	#endif
	
	#define APPLY_NORMALMAP(normal, uv, worldPos, tangentBasis, bitangentBasis) INIT_TANGENT_NORMAL(uv); MUL_TANGENT_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis)
#else
	#define APPLY_NORMALMAP(normal, uv, worldPos, tangentBasis, bitangentBasis)
#endif
#define SETUP_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis, worldNormal) float3 normal; SET_WORLD_NORMAL(normal, worldNormal, worldPos); APPLY_NORMALMAP(normal, uv, worldPos, tangentBasis, bitangentBasis);

/************ ForwardBase && ForwardAdd ************/
struct PixelInput
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float4 Color : COLOR;
	float3 ToEye   : TEXCOORD1;//world space
	float3 ToLight : TEXCOORD2;//world space
#if HAS_ATTRIBUTE_NORMAL
	float3 Normal  : TEXCOORD3;//world space
#endif
#if HAS_ATTRIBUTE_TANGENT
    float3 Tangent : TEXCOORD4;//world space
#endif
#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT && !ENABLE_PIXEL_BTN
	float3 Bitangent : TEXCOORD5;//world space
#endif
	float3 WorldPos : POSITION0;//world space
#if ENABLE_SHADOW_MAP
	float4 PosLight : POSITION1; //ndc space
	float4 ViewPosLight : POSITION2;
#endif
};
PixelInput VS(vbSurface surf, vbWeightedSkin skin)
{
	PixelInput output;
	matrix MW = mul(World, transpose(Model));
		
	//WorldPos
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);
	
	//normal && tangent && bitangent
#if HAS_ATTRIBUTE_NORMAL
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal = normalize(mul(MW, skinNormal).xyz);
#endif

#if HAS_ATTRIBUTE_TANGENT
    float4 skinTangent = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Tangent.xyz, 0.0));
	output.Tangent = normalize(mul(MW, skinTangent).xyz);
#endif

#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT && !ENABLE_PIXEL_BTN
	output.Bitangent = cross(output.Normal.xyz, output.Tangent.xyz) * skin.Tangent.w;
#endif
	
	//ToEye
	output.ToEye = CameraPositionExposure.xyz - output.Pos.xyz;

	//PosLight
#if ENABLE_SHADOW_MAP
	output.ViewPosLight = mul(LightView, output.Pos);
	output.PosLight = mul(LightProjection, output.ViewPosLight);
#endif
	
    //ToLight
    output.ToLight = LightPosition.xyz - output.Pos.xyz * LightPosition.w;
    
	//WorldPos	
	output.WorldPos = output.Pos.xyz / output.Pos.w;
    
#if ENABLE_SHADOW_MAP_BIAS && HAS_ATTRIBUTE_NORMAL
	float bias = max(0.05 * (1.0 - dot(output.Normal, output.ToLight)), 0.005);
	output.PosLight.z -= bias * output.PosLight.w;	
#endif
	
	//Pos
    output.Pos = mul(View, output.Pos);
    output.Pos = mul(Projection, output.Pos);
	output.Tex = surf.Tex;
	output.Color = surf.Color;
    return output;
}

#if LIGHTMODE == LIGHTMODE_FORWARD_BASE || LIGHTMODE == LIGHTMODE_FORWARD_ADD
float4 PS_(PixelInput input, bool additive)
{
    SETUP_NORMAL(normal, input.Tex, input.WorldPos, normalize(input.Tangent.xyz), normalize(input.Bitangent.xyz), normalize(input.Normal.xyz));
	float3 toLight = normalize(input.ToLight);
	float3 toEye = normalize(input.ToEye);
	
	LightingInput li;
	li.light_color = LightColor;
	li.albedo = input.Color.rgb * GetAlbedo(input.Tex);
	float4 armt = GetAoRoughnessMetallicTransmission(input.Tex);
	li.ao = armt.x;
	li.percertual_roughness = armt.y;
	li.metallic = armt.z;
	li.transmission_factor = armt.w;
	li.emissive = GetEmissive(input.Tex);
	li.sheen_color_roughness = GetSheenColorRoughness(input.Tex);
	li.clearcoat_color_roughness = GetClearCoatRoughness(input.Tex);
	li.uv = input.Tex;
	li.world_pos = input.WorldPos;
#if DEBUG_CHANNEL 
	li.uv1 = MakeDummyColor(toEye).xy;
	#if ENABLE_NORMAL_MAP
		li.tangent_normal = MIR_SAMPLE_TEX2D(txNormal, GetUV(input.Tex, NormalTransUV)).xyz * 2.0 - 1.0;
		li.tangent_normal = normalize(li.tangent_normal * float3(NormalScale, NormalScale, 1.0));
		li.tangent_normal = li.tangent_normal * 0.5 + 0.5;
	#else
		li.tangent_normal = MakeDummyColor(toEye);
	#endif
	#if HAS_ATTRIBUTE_NORMAL
		li.normal_basis = normalize(input.Normal.xyz) * 0.5 + 0.5;
	#else
		li.normal_basis = MakeDummyColor(toEye);
	#endif
	#if HAS_ATTRIBUTE_TANGENT
		li.tangent_basis = normalize(input.Tangent.xyz) * 0.5 + 0.5;
	#else
		li.tangent_basis = MakeDummyColor(toEye);
	#endif
	#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT
		#if !ENABLE_PIXEL_BTN
			li.bitangent_basis = normalize(input.Bitangent.xyz) * 0.5 + 0.5;
		#elif ENABLE_NORMAL_MAP
			li.bitangent_basis = tbn[1] * 0.5 + 0.5;
		#else
			float3x3 tbn = GetTangentToWorldTBN(normalize(input.Tangent.xyz), normalize(input.Normal.xyz), GetDpDuv(input.WorldPos, input.Tex));
			li.bitangent_basis = tbn[1] * 0.5 + 0.5;
		#endif
	#else
		li.bitangent_basis = MakeDummyColor(toEye);
	#endif
	li.window_pos = input.Pos.xyz;
#endif
	return Lighting(li, toLight, normal, toEye, additive);	
	//return float4(GetAlbedo(input.Tex), 1.0);
	//return float4(input.Tex, 0.0, 1.0);
}
float4 PS(PixelInput input) : SV_Target
{	
	return PS_(input, false);
}
float4 PSAdd(PixelInput input) : SV_Target
{	
	return PS_(input, true);
}
#endif

/************ ShadowCaster ************/
struct PSShadowCasterInput 
{
	float4 Pos  : SV_POSITION;
	///float2 Tex : TEXCOORD0;
};
PSShadowCasterInput VSShadowCaster(vbSurface surf, vbWeightedSkin skin)
{
	PSShadowCasterInput output;
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos, 1.0));
	output.Pos = mul(mul(LightProjection, mul(LightView, mul(World, transpose(Model)))), skinPos);
	///output.Tex = surf.Tex;
	return output;
}
float4 PSShadowCasterDebug(PSShadowCasterInput input) : SV_Target
{
	float4 finalColor = float4(1.0,0,0,1);
	///finalColor = GetAlbedo(input.Tex);
	return finalColor;
}

struct PSGenerateVSMInput
{
	float4 Pos : SV_POSITION;
	float4 ViewPos : POSITION0;
	///float4 WorldPos : POSITION1;
};
PSGenerateVSMInput VSGenerateVSM(vbSurface surf, vbWeightedSkin skin)
{
	PSGenerateVSMInput output;
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos, 1.0));
	float4 worldPos = mul(mul(World, transpose(Model)), skinPos);
	///output.WorldPos = worldPos;
	output.ViewPos = mul(LightView, worldPos);
	output.Pos = mul(LightProjection, output.ViewPos);
	return output;
}
float4 PSGenerateVSM(PSGenerateVSMInput input) : SV_Target
{
	float depth = length(input.ViewPos);
	///float worldPos = input.WorldPos.xyz / input.WorldPos.w;
	///float depth = length(LightPosition.xyz - worldPos * LightPosition.w);
	return float4(depth, depth * depth, 0.0f, 1.0f);
}

#if LIGHTMODE == LIGHTMODE_SHADOW_CASTER_POSTPROCESS
struct VSMBlurInput
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 texUV : TEXCOORD0;
};
VSMBlurInput VSBlurVSM(vbSurface input)
{
	VSMBlurInput output;
	output.Pos = float4(input.Pos, 1.0);
	output.Color = input.Color;
	output.texUV = input.Tex;
	return output;
}
inline float BoxFilterStart(float fWidth)  //Assumes filter is odd
{
	return ((fWidth - 1.0f) / 2.0f);
}
inline float4 BlurVSMFunction(VSMBlurInput input, bool bVertical)
{
	const float fFilterWidth = 9.0f;
	const float fStepSize = 1.0f;
	
	float fStartOffset = BoxFilterStart(fFilterWidth);
	float2 fTexelOffset = float2(bVertical * (fStepSize / ShadowMapSize.x), !bVertical * (fStepSize / ShadowMapSize.y));
    
	float2 fTexStart = input.texUV - (fStartOffset * fTexelOffset);
	float4 output = (float4) 0.0f;
    
	for (int i = 0; i < fFilterWidth; ++i)
		output += MIR_SAMPLE_TEX2D(PrePassOutput, float2(fTexStart + fTexelOffset * i));
    
	return output / fFilterWidth;
}
float4 PSBlurVSMX(VSMBlurInput input) : SV_Target
{
	return BlurVSMFunction(input, false);
}
float4 PSBlurVSMY(VSMBlurInput input) : SV_Target
{
	return BlurVSMFunction(input, true);
}
#endif

/************ PrepassBase ************/
struct PSPrepassBaseInput
{
	float4 SVPos : SV_POSITION;
	float2 Tex : TEXCOORD0;
#if HAS_ATTRIBUTE_NORMAL
	float3 Normal : TEXCOORD1;
#endif
#if HAS_ATTRIBUTE_TANGENT
	float3 Tangent : TEXCOORD2;
#endif
#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT && !ENABLE_PIXEL_BTN
	float3 Bitangent : TEXCOORD3;
#endif
	float3 WorldPos : POSITION0; //world space
	float4 Color : COLOR;
};
PSPrepassBaseInput VSPrepassBase(vbSurface surf, vbWeightedSkin skin)
{
	PSPrepassBaseInput output;
	matrix MW = mul(World, transpose(Model));
	
	//Pos && WorldPos
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	float4 worldPos = mul(MW, skinPos);
	output.WorldPos = worldPos.xyz / worldPos.w;
	
	//normal && tangent && bitangent
#if HAS_ATTRIBUTE_NORMAL
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal = normalize(mul(MW, skinNormal).xyz);
#endif

#if HAS_ATTRIBUTE_TANGENT
	float4 skinTangent = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Tangent.xyz, 0.0));
	output.Tangent = normalize(mul(MW, skinTangent).xyz);
#endif

#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT && !ENABLE_PIXEL_BTN
	output.Bitangent = cross(output.Normal.xyz, output.Tangent.xyz) * skin.Tangent.w;
#endif
	
	//Pos
	output.SVPos = mul(View, worldPos);
    output.SVPos = mul(Projection, output.SVPos);
	
	//Tex, Color
	output.Tex = surf.Tex;
	output.Color = surf.Color;
    return output;
}

struct PSPrepassBaseOutput
{
	float4 Pos : SV_Target0;//worldPos(RGB), roughness(A)
    float4 Normal : SV_Target1;//worldNormal(RGB), metallic(A)
    float4 Albedo : SV_Target2;//albedo(RGB), ao(A)
	float4 Emissive : SV_Target3;//emissive(RGB), transmissionFactor(A)
	float4 Sheen : SV_Target4;//sheenColor(RGB), sheenRoughness(A)
	float4 ClearCoat : SV_Target5;//clearcoatColor(RGB), clearcoatRoughness(A)
};
PSPrepassBaseOutput PSPrepassBase(PSPrepassBaseInput input)
{
	PSPrepassBaseOutput output;
	float4 armt = GetAoRoughnessMetallicTransmission(input.Tex);
	output.Pos = float4(input.WorldPos * 0.5 + 0.5, armt.y);
	SETUP_NORMAL(normal, input.Tex, input.WorldPos, normalize(input.Tangent.xyz), normalize(input.Bitangent.xyz), normalize(input.Normal.xyz));
	output.Normal = float4(normal * 0.5 + 0.5, armt.z);
	output.Albedo = float4(input.Color.rgb * GetAlbedo(input.Tex), armt.x);
	output.Emissive = float4(GetEmissive(input.Tex), armt.w);
	output.Sheen = GetSheenColorRoughness(input.Tex);
	output.ClearCoat = GetClearCoatRoughness(input.Tex);
	return output;
}

/************ PrepassFinal && PrepassFinalAdd ************/
#if LIGHTMODE == LIGHTMODE_PREPASS_FINAL || LIGHTMODE == LIGHTMODE_PREPASS_FINAL_ADD
struct PSPrepassFinalInput
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};
PSPrepassFinalInput VSPrepassFinal(vbSurface input)
{
	PSPrepassFinalInput output;
    output.Pos = float4(input.Pos, 1.0);
	output.Tex = input.Tex;
    return output;
}

float4 PSPrepassFinal_(PSPrepassFinalInput input, bool additive)
{
	LightingInput li = (LightingInput)0;
	li.light_color = LightColor;
	
	float4 worldPosition = MIR_SAMPLE_TEX2D(_GBufferPos, input.Tex);//worldPos(RGB), roughness(A)
	li.world_pos = worldPosition.xyz * 2.0 - 1.0;
	li.percertual_roughness = worldPosition.w;
	
	float4 worldNormal = MIR_SAMPLE_TEX2D(_GBufferNormal, input.Tex);//worldNormal(RGB), metallic(A)
	worldNormal.xyz = worldNormal.xyz * 2.0 - 1.0;
	li.metallic = worldNormal.w;
	float3 toLight = normalize(LightPosition.xyz - li.world_pos * LightPosition.w);
	float3 toEye = normalize(CameraPositionExposure.xyz - li.world_pos);
	
	float4 albedo = MIR_SAMPLE_TEX2D(_GBufferAlbedo, input.Tex);//albedo(RGB), ao(A)
	li.albedo = albedo.rgb;
	li.ao = albedo.w;
	
	float4 emissive = MIR_SAMPLE_TEX2D(_GBufferEmissive, input.Tex);//emissive(RGB), transmissionFactor(A)
	li.emissive = emissive.rgb;
	li.transmission_factor = emissive.w;
	
	li.sheen_color_roughness = MIR_SAMPLE_TEX2D(_GBufferSheen, input.Tex);//sheenColor(RGB), sheenRoughness(A)
	li.clearcoat_color_roughness = MIR_SAMPLE_TEX2D(_GBufferClearCoat, input.Tex);
	
#if DEBUG_CHANNEL == DEBUG_CHANNEL_GBUFFER_POS
	return worldPosition;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GBUFFER_NORMAL
	return float4(worldNormal.xyz * 0.5 + 0.5, worldNormal.w);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GBUFFER_ALBEDO
	return albedo;
#else
	return Lighting(li, toLight, worldNormal.xyz, toEye, additive);
#endif
}
float4 PSPrepassFinal(PSPrepassFinalInput input) : SV_Target0
{
	return PSPrepassFinal_(input, false);
}
float4 PSPrepassFinalAdd(PSPrepassFinalInput input) : SV_Target0
{
	return PSPrepassFinal_(input, true);	
}
#endif