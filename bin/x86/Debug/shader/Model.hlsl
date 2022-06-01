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
MIR_DECLARE_TEX2D(txMetalness, 2);
MIR_DECLARE_TEX2D(txRoughness, 3);
MIR_DECLARE_TEX2D(txAmbientOcclusion, 4);
MIR_DECLARE_TEX2D(txEmissive, 5);

//#define HAS_ATTRIBUTE_NORMAL 1 
//#define HAS_ATTRIBUTE_TANGENT 1 
//#define ENABLE_PIXEL_BTN 1 
//#define ENABLE_NORMAL_MAP 1
//#define ENABLE_METALLIC_X_X_SMOOTHNESS_MAP 1
//#define ENABLE_METALLIC_MAP 1
//#define ENABLE_ROUGHNESS_MAP 1

cbuffer cbModel : register(b3)
{
	float4 AlbedoUV;
	float4 NormalUV; 
    float4 OcclusionUV;
    float4 RoughnessUV;
    float4 MetallicUV;
	float4 EmissiveUV;
	
	float4 AlbedoFactor;
	float NormalScale;
    float OcclusionStrength;
    float RoughnessFactor;
    float MetallicFactor;
	float4 EmissiveFactor;
}

inline float4 GetAlbedo(float2 uv) 
{
    float4 albedo = AlbedoFactor;
#if ENABLE_ALBEDO_MAP
	float4 color = MIR_SAMPLE_TEX2D(txAlbedo, GetUV(uv, AlbedoUV));
	#if ALBEDO_MAP_SRGB
		color = sRGBToLinear(color);
	#endif
	albedo *= color;
#endif
    return albedo;
}

inline float4 GetEmissive(float2 uv) 
{
    float4 emissive = EmissiveFactor;
#if ENABLE_EMISSIVE_MAP
	float3 color = MIR_SAMPLE_TEX2D(txEmissive, GetUV(uv, EmissiveUV)).rgb;
	#if EMISSIVE_MAP_SRGB
		color = sRGBToLinear(color);
	#endif
    emissive.rgb *= color;
#endif
    return emissive;
}

inline float4 GetAoRoughnessMetallic(float2 uv)
{
    float4 value = float4(1.0, RoughnessFactor, MetallicFactor, 1.0);
#if ENABLE_AO_ROUGHNESS_METALLIC_MAP
	float3 arm = MIR_SAMPLE_TEX2D(txAmbientOcclusion, GetUV(uv, OcclusionUV)).rgb;
	value.x = lerp(1.0, arm.x, OcclusionStrength);
	value.yz *= arm.yz;
#elif ENABLE_METALLIC_X_X_SMOOTHNESS_MAP
    #if ENABLE_AO_MAP
		float ao = MIR_SAMPLE_TEX2D(txAmbientOcclusion, GetUV(uv, OcclusionUV)).r;
		value.x = 1.0 + (ao - 1.0) * value.x;
	#endif
	float2 ms = MIR_SAMPLE_TEX2D(txMetalness, GetUV(uv, RoughnessUV)).ra;
	value.y *= 1.0 - ms.y;
	value.z *= ms.x; 
#elif ENABLE_SPECULAR_MAP
	value = MIR_SAMPLE_TEX2D(txMetalness, GetUV(uv, MetallicUV));
#else
    #if ENABLE_AO_MAP
		float ao = MIR_SAMPLE_TEX2D(txAmbientOcclusion, GetUV(uv, OcclusionUV)).r;
		value.x = 1.0 + (ao - 1.0) * value.x;
	#endif
    #if ENABLE_ROUGHNESS_MAP 
		value.y *= MIR_SAMPLE_TEX2D(txRoughness, GetUV(uv, RoughnessUV)).r;
	#endif
    #if ENABLE_METALLIC_MAP
		value.z *= MIR_SAMPLE_TEX2D(txMetalness, GetUV(uv, MetallicUV)).r;
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
		#define INIT_TANGENT_NORMAL(uv) float3 tangentNormal = UnpackScaleNormalRGorAG(MIR_SAMPLE_TEX2D(txNormal, GetUV(uv, NormalUV)), NormalScale)
	#else
		#define INIT_TANGENT_NORMAL(uv) float3 tangentNormal = MIR_SAMPLE_TEX2D(txNormal, GetUV(uv, NormalUV)).xyz * 2.0 - 1.0; tangentNormal = normalize(tangentNormal * float3(NormalScale, NormalScale, 1.0))
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

/************ ForwardBase ************/
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

float4 PS(PixelInput input) : SV_Target
{	
    SETUP_NORMAL(normal, input.Tex, input.WorldPos, normalize(input.Tangent.xyz), normalize(input.Bitangent.xyz), normalize(input.Normal.xyz));
	float3 toLight = normalize(input.ToLight);
	float3 toEye = normalize(input.ToEye);
	
	LightingInput li;
	li.albedo = GetAlbedo(input.Tex);
	li.ao_rough_metal = GetAoRoughnessMetallic(input.Tex);
	li.emissive = GetEmissive(input.Tex);
	li.uv = input.Tex;
#if DEBUG_CHANNEL   
	li.uv1 = MakeDummyColor(toEye).xy;
	#if ENABLE_NORMAL_MAP
		li.tangent_normal = MIR_SAMPLE_TEX2D(txNormal, GetUV(input.Tex, NormalUV)).xyz * 2.0 - 1.0;
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
	li.world_pos = input.WorldPos;
	li.window_pos = input.Pos.xyz;
#endif	
	return input.Color * Lighting(li, toLight, normal, toEye);
}

/************ ForwardAdd ************/
float4 PSAdd(PixelInput input) : SV_Target
{	
    SETUP_NORMAL(normal, input.Tex, input.WorldPos, normalize(input.Tangent.xyz), normalize(input.Bitangent.xyz), normalize(input.Normal.xyz));
	float3 toLight = normalize(input.ToLight);
	float3 toEye = normalize(input.ToEye);
	
	LightingInput li = (LightingInput)0;
	li.albedo = GetAlbedo(input.Tex);	
	li.ao_rough_metal = GetAoRoughnessMetallic(input.Tex);	
	li.emissive = GetEmissive(input.Tex);
	li.uv = input.Tex;
	//return input.Color * Lighting(li, toLight, normal, toEye);
	return float4(0.0,0.0,0.0,0.0);
}

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
		output += MIR_SAMPLE_TEX2D(_GBufferAlbedo, float2(fTexStart + fTexelOffset * i));
    
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

/************ PrepassBase ************/
struct PSPrepassBaseInput
{
	float4 SVPos : SV_POSITION;
    float4 Pos : POSITION0;//world space
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
	float3 WorldPos : POSITION1; //world space
};
PSPrepassBaseInput VSPrepassBase(vbSurface surf, vbWeightedSkin skin)
{
	PSPrepassBaseInput output;
	matrix MW = mul(World, transpose(Model));
	
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
	
	//Pos && WorldPos
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);
	output.WorldPos = output.Pos.xyz / output.Pos.w;
	
	output.Pos = mul(View, output.Pos);
    output.Pos = mul(Projection, output.Pos);
	output.SVPos = output.Pos;
	
	//Tex
	output.Tex = surf.Tex;
    return output;
}

struct PSPrepassBaseOutput
{
	float4 Pos : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Albedo : SV_Target2;
	float4 Emissive : SV_Target3;
};
PSPrepassBaseOutput PSPrepassBase(PSPrepassBaseInput input)
{
	PSPrepassBaseOutput output;
	output.Pos = float4(input.Pos.xyz / input.Pos.w * 0.5 + 0.5, 1.0);
	float4 aorm = GetAoRoughnessMetallic(input.Tex);
	SETUP_NORMAL(normal, input.Tex, input.WorldPos, normalize(input.Tangent.xyz), normalize(input.Bitangent.xyz), normalize(input.Normal.xyz));
	output.Normal = float4(normal * 0.5 + 0.5, aorm.x);
	output.Albedo = float4(GetAlbedo(input.Tex).xyz, aorm.y);
	output.Emissive = float4(GetEmissive(input.Tex).xyz, aorm.z);
	return output;
}

/************ PrepassFinal ************/
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

struct PSPrepassFinalOutput
{
	float4 Color : SV_Target0;
	///float Depth : SV_Depth;
};
PSPrepassFinalOutput PSPrepassFinal(PSPrepassFinalInput input)
{
	PSPrepassFinalOutput output;
	///output.Depth = MIR_SAMPLE_TEX2D_LEVEL(_GDepth, input.Tex, 0).r;
	
	float4 position = float4(MIR_SAMPLE_TEX2D(_GBufferPos, input.Tex).xyz * 2.0 - 1.0, 1.0);
	float4 worldPosition = mul(mul(ViewInv, ProjectionInv), position);
	worldPosition /= worldPosition.w;
	
	float4 normal = MIR_SAMPLE_TEX2D(_GBufferNormal, input.Tex);
	normal.xyz = normal.xyz * 2.0 - 1.0;
	float3 toLight = normalize(LightPosition.xyz - worldPosition.xyz * LightPosition.w);
	float3 toEye = normalize(CameraPositionExposure.xyz - worldPosition.xyz);
	
	LightingInput li = (LightingInput)0;
	li.albedo = MIR_SAMPLE_TEX2D(_GBufferAlbedo, input.Tex);
	li.emissive = MIR_SAMPLE_TEX2D(_GBufferEmissive, input.Tex);
	li.ao_rough_metal = float4(normal.w, li.albedo.w, li.emissive.w, 1.0);
	li.uv = input.Tex;
	output.Color = Lighting(li, toLight, normal.xyz, toEye);
	
	return output;
}