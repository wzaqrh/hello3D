#include "Standard.glinc"
#include "Skeleton.glinc"
#include "CommonFunction.glinc"
#include "Shadow.glinc"
#include "Lighting.glinc"
#include "Lighting.glinc"
#include "ToneMapping.glinc"
#include "Macros.glinc"

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

layout (binding = 3, std140) uniform cbModel
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
};

#if SHADER_STAGE == SHADER_STAGE_PIXEL
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
		#define INIT_TANGENT_NORMAL(uv) float3 tangentNormal = MIR_SAMPLE_TEX2D(txNormal, GetUV(uv, NormalTransUV)).xyz * 2.0 - float3(1.0); tangentNormal = normalize(tangentNormal * float3(NormalScale, NormalScale, 1.0))
	#endif
	
	#if !ENABLE_PIXEL_BTN
		#define MUL_TANGENT_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis) normal = normalize(tangentNormal.x * tangentBasis + tangentNormal.y * bitangentBasis + tangentNormal.z * normal);
    #else
		#if HAS_ATTRIBUTE_TANGENT
			#define MUL_TANGENT_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis) float3x3 tbn = GetTangentToWorldTBN(tangentBasis, normal, GetDpDuv(worldPos, uv)); normal = normalize(tbn * tangentNormal);
		#else
			#define MUL_TANGENT_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis) float3x3 tbn = GetTangentToWorldTBN(normal, GetDpDuv(worldPos, uv)); normal = normalize(tbn * tangentNormal);
		#endif
	#endif
	
	#define APPLY_NORMALMAP(normal, uv, worldPos, tangentBasis, bitangentBasis) INIT_TANGENT_NORMAL(uv); MUL_TANGENT_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis)
#else
	#define APPLY_NORMALMAP(normal, uv, worldPos, tangentBasis, bitangentBasis)
#endif
#define SETUP_NORMAL(normal, uv, worldPos, tangentBasis, bitangentBasis, worldNormal) float3 normal; SET_WORLD_NORMAL(normal, worldNormal, worldPos); APPLY_NORMALMAP(normal, uv, worldPos, tangentBasis, bitangentBasis);
#endif

#if LIGHTMODE == LIGHTMODE_FORWARD_BASE || LIGHTMODE == LIGHTMODE_FORWARD_ADD
struct PixelInput
{
	float2 Tex;
	float4 Color;
	float3 ToEye;//world space
	float3 ToLight;//world space
#if HAS_ATTRIBUTE_NORMAL
	float3 Normal;//world space
#endif
#if HAS_ATTRIBUTE_TANGENT
    float3 Tangent;//world space
#endif
#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT && !ENABLE_PIXEL_BTN
	float3 Bitangent;//world space
#endif
	float3 WorldPos;//world space
#if ENABLE_SHADOW_MAP
	float4 PosLight; //ndc space
	float4 ViewPosLight;
#endif
};
#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN_Surface(surf, 0);
	MIR_DECLARE_VS_IN_Skin(skin, 3);
	MIR_DECLARE_VS_OUT(PixelInput, o, 0);

	void StageEntry_VS()
	{
		matrix MW = World * transpose(Model);

		//WorldPos
		float4 skinPos = Skinning(skinBlendWeights, skinBlendIndices, float4(surfPos.xyz, 1.0));
		vec4 oPos = (MW * skinPos);
	
		//normal && tangent && bitangent
	#if HAS_ATTRIBUTE_NORMAL
		float4 skinNormal = Skinning(skinBlendWeights, skinBlendIndices, float4(skinNormal.xyz, 0.0));
		float3 oNormal = normalize((MW * skinNormal).xyz);
		o.Normal = oNormal;
	#endif

	#if HAS_ATTRIBUTE_TANGENT
		float4 skinTangent = Skinning(skinBlendWeights, skinBlendIndices, float4(skinTangent.xyz, 0.0));
		float3 oTangent = normalize((MW * skinTangent).xyz);
		o.Tangent = oTangent;
	#endif

	#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT && !ENABLE_PIXEL_BTN
		o.Bitangent = cross(oNormal.xyz, oTangent.xyz) * skinTangent.w;
	#endif
	
		//ToEye
		o.ToEye = CameraPositionExposure.xyz - oPos.xyz;

		//PosLight
	#if ENABLE_SHADOW_MAP
		o.ViewPosLight = (LightView * oPos);
		o.PosLight = (LightProjection * o.ViewPosLight);
	#endif
	
		//ToLight
		o.ToLight = LightPosition.xyz - oPos.xyz * LightPosition.w;
    
		//WorldPos	
		o.WorldPos = oPos.xyz / oPos.w;
    
	#if ENABLE_SHADOW_MAP_BIAS && HAS_ATTRIBUTE_NORMAL
		float bias = max(0.05 * (1.0 - dot(o.Normal, o.ToLight)), 0.005);
		o.PosLight.z -= bias * o.PosLight.w;	
	#endif
	
		//Pos
		gl_Position = Projection * View * oPos;
		o.Tex = surfTex;
		o.Color = surfColor;
	}
#else
	MIR_DECLARE_PS_IN(PixelInput, i, 0);
	MIR_DECLARE_PS_OUT(vec4, oColor, 0);

	void PS_()
	{
	#if LIGHTMODE == LIGHTMODE_FORWARD_BASE
		bool additive = false;
	#else
		bool additive = true;
	#endif

		SETUP_NORMAL(normal, i.Tex, i.WorldPos, normalize(i.Tangent.xyz), normalize(i.Bitangent.xyz), normalize(i.Normal.xyz));
		float3 toLight = normalize(i.ToLight);
		float3 toEye = normalize(i.ToEye);
	
		LightingInput li;
		li.light_color = LightColor;
		li.albedo = i.Color.rgb * GetAlbedo(i.Tex);
		float4 armt = GetAoRoughnessMetallicTransmission(i.Tex);
		li.ao = armt.x;
		li.percertual_roughness = armt.y;
		li.metallic = armt.z;
		li.transmission_factor = armt.w;
		li.emissive = GetEmissive(i.Tex);
		li.sheen_color_roughness = GetSheenColorRoughness(i.Tex);
		li.clearcoat_color_roughness = GetClearCoatRoughness(i.Tex);
		li.uv = i.Tex;
		li.world_pos = i.WorldPos;
	#if DEBUG_CHANNEL 
		li.uv1 = MakeDummyColor(toEye).xy;
		#if ENABLE_NORMAL_MAP
			li.tangent_normal = MIR_SAMPLE_TEX2D(txNormal, GetUV(i.Tex, NormalTransUV)).xyz * 2.0 - float3(1.0);
			li.tangent_normal = normalize(li.tangent_normal * float3(NormalScale, NormalScale, 1.0));
			li.tangent_normal = li.tangent_normal * 0.5 + float3(0.5);
		#else
			li.tangent_normal = MakeDummyColor(toEye);
		#endif
		#if HAS_ATTRIBUTE_NORMAL
			li.normal_basis = normalize(i.Normal.xyz) * 0.5 + float3(0.5);
		#else
			li.normal_basis = MakeDummyColor(toEye);
		#endif
		#if HAS_ATTRIBUTE_TANGENT
			li.tangent_basis = normalize(i.Tangent.xyz) * 0.5 + float3(0.5);
		#else
			li.tangent_basis = MakeDummyColor(toEye);
		#endif
		#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT
			#if !ENABLE_PIXEL_BTN
				li.bitangent_basis = normalize(i.Bitangent.xyz) * 0.5 + float3(0.5);
			#elif ENABLE_NORMAL_MAP
				li.bitangent_basis = tbn[1] * 0.5 + float3(0.5);
			#else
				float3x3 tbn = GetTangentToWorldTBN(normalize(i.Tangent.xyz), normalize(i.Normal.xyz), GetDpDuv(i.WorldPos, i.Tex));
				li.bitangent_basis = tbn[1] * 0.5 + float3(0.5);
			#endif
		#else
			li.bitangent_basis = MakeDummyColor(toEye);
		#endif
		li.window_pos = gl_FragCoord.xyz;
	#endif
		oColor = Lighting(li, toLight, normal, toEye, additive);
	}
	void StageEntry_PS() {
		PS_();
	}
	void StageEntry_PSAdd() {
	   PS_();
	}
#endif
#endif

#if LIGHTMODE == LIGHTMODE_SHADOW_CASTER
#if SHADOW_MODE != SHADOW_VSM
#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN_Surface(surf, 0);
	MIR_DECLARE_VS_IN_Skin(skin, 3);

	void StageEntry_VSShadowCaster()
	{
		float4 skinPos = float4(surfPos, 1.0);//Skinning(skinBlendWeights, skinBlendIndices, float4(surfPos, 1.0));
		matrix WVP = LightProjection * LightView * World * transpose(Model);
		gl_Position = (WVP * skinPos);
	}
#else
	MIR_DECLARE_PS_OUT(vec4, oColor, 0);

	void StageEntry_PSShadowCasterDebug()
	{
		oColor = float4(1.0, 0, 0, 1.0);
	}
#endif
#else
struct PSGenerateVSMInput
{
	float4 ViewPos;
};
#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN_Surface(surf, 0);
	MIR_DECLARE_VS_IN_Skin(skin, 3);
	MIR_DECLARE_VS_OUT(PSGenerateVSMInput, o, 0);

	void StageEntry_VSGenerateVSM()
	{
		float4 skinPos = Skinning(skinBlendWeights, skinBlendIndices, float4(surfPos, 1.0));
		float4 worldPos = World * transpose(Model) * skinPos;
		o.ViewPos = LightView * worldPos;
		gl_Position = LightProjection * o.ViewPos;
	}
#else
	MIR_DECLARE_PS_IN(PSGenerateVSMInput, i, 0);
	MIR_DECLARE_PS_OUT(vec4, oColor, 0);

	void StageEntry_PSGenerateVSM()
	{
		float depth = length(i.ViewPos);
		oColor = float4(depth, depth * depth, 0.0f, 1.0f);
	}
#endif
#endif
#endif

#if LIGHTMODE == LIGHTMODE_SHADOW_CASTER_POSTPROCESS
struct VSMBlurInput
{
	float4 Color;
	float2 texUV;
};
#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN_Surface(i, 0);
	MIR_DECLARE_VS_OUT(VSMBlurInput, o, 0);

	void StageEntry_VSBlurVSM()
	{
		gl_Position = float4(iPos, 1.0);
		o.Color = iColor;
		o.texUV = float2(iTex.x, 1.0 - iTex.y);
	}
#else
	inline float BoxFilterStart(float fWidth)  //Assumes filter is odd
	{
		return ((fWidth - 1.0f) / 2.0f);
	}
	MIR_DECLARE_PS_IN(VSMBlurInput, i, 0);
	MIR_DECLARE_PS_OUT(vec4, oColor, 0);

	void BlurVSMFunction(bool bVertical)
	{
		const float fFilterWidth = 9.0f;
		const float fStepSize = 1.0f;
	
		float fStartOffset = BoxFilterStart(fFilterWidth);
		float2 fTexelOffset = bVertical ? float2(fStepSize / ShadowMapSize.x, 0.0) : float2(0.0, fStepSize / ShadowMapSize.y);

		float2 fTexStart = i.texUV - (fStartOffset * fTexelOffset);
		oColor = float4(0.0);
    #if 1
		for (int i = 0; i < fFilterWidth; ++i)
			oColor += MIR_SAMPLE_TEX2D(PrePassOutput, float2(fTexStart + fTexelOffset * i));
		oColor = oColor / fFilterWidth;
	#else
		oColor = MIR_SAMPLE_TEX2D(PrePassOutput, i.texUV);
	#endif
	}
	void StageEntry_PSBlurVSMX()
	{
		BlurVSMFunction(false);
	}
	void StageEntry_PSBlurVSMY()
	{
		BlurVSMFunction(true);
	}
#endif
#endif

#if LIGHTMODE == LIGHTMODE_PREPASS_BASE
struct PSPrepassBaseInput
{
	float2 Tex;
#if HAS_ATTRIBUTE_NORMAL
	float3 Normal;
#endif
#if HAS_ATTRIBUTE_TANGENT
	float3 Tangent;
#endif
#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT && !ENABLE_PIXEL_BTN
	float3 Bitangent;
#endif
	float3 WorldPos;
	float4 Color;
};
#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN_Surface(surf, 0);
	MIR_DECLARE_VS_IN_Skin(skin, 3);
	MIR_DECLARE_VS_OUT(PSPrepassBaseInput, o, 0);

	void StageEntry_VSPrepassBase()
	{
		matrix MW = World * transpose(Model);
	
		//Pos && WorldPos
		float4 skinPos = Skinning(skinBlendWeights, skinBlendIndices, float4(surfPos.xyz, 1.0));
		float4 worldPos = (MW * skinPos);
		o.WorldPos = worldPos.xyz / worldPos.w;
	
		//normal && tangent && bitangent
	#if HAS_ATTRIBUTE_NORMAL
		float4 skinNormal = Skinning(skinBlendWeights, skinBlendIndices, float4(skinNormal.xyz, 0.0));
		o.Normal = normalize((MW * skinNormal).xyz);
	#endif

	#if HAS_ATTRIBUTE_TANGENT
		float4 skinTangent = Skinning(skinBlendWeights, skinBlendIndices, float4(skinTangent.xyz, 0.0));
		o.Tangent = normalize((MW * skinTangent).xyz);
	#endif

	#if HAS_ATTRIBUTE_NORMAL && HAS_ATTRIBUTE_TANGENT && !ENABLE_PIXEL_BTN
		o.Bitangent = cross(o.Normal.xyz, o.Tangent.xyz) * skinTangent.w;
	#endif
	
		//Pos
		gl_Position = Projection *  View * worldPos;
	
		//Tex, Color
		o.Tex = surfTex;
		o.Color = surfColor;
	}
#else
	MIR_DECLARE_PS_IN(PSPrepassBaseInput, i, 0);
	MIR_DECLARE_PS_OUT(vec4, oPos, 0);//worldPos(RGB), roughness(A)
	MIR_DECLARE_PS_OUT(vec4, oNormal, 1);//worldNormal(RGB), metallic(A)
	MIR_DECLARE_PS_OUT(vec4, oAlbedo, 2);//albedo(RGB), ao(A)
	MIR_DECLARE_PS_OUT(vec4, oEmissive, 3);//emissive(RGB), transmissionFactor(A)
	MIR_DECLARE_PS_OUT(vec4, oSheen, 4);//sheenColor(RGB), sheenRoughness(A)
	MIR_DECLARE_PS_OUT(vec4, oClearCoat, 5);//clearcoatColor(RGB), clearcoatRoughness(A)

	void StageEntry_PSPrepassBase()
	{
		float4 armt = GetAoRoughnessMetallicTransmission(i.Tex);
		oPos = float4(i.WorldPos * 0.5 + 0.5, armt.y);
		SETUP_NORMAL(normal, i.Tex, i.WorldPos, normalize(i.Tangent.xyz), normalize(i.Bitangent.xyz), normalize(i.Normal.xyz));
		oNormal = float4(normal * 0.5 + 0.5, armt.z);
		oAlbedo = float4(i.Color.rgb * GetAlbedo(i.Tex), armt.x);
		oEmissive = float4(GetEmissive(i.Tex), armt.w);
		oSheen = GetSheenColorRoughness(i.Tex);
		oClearCoat = GetClearCoatRoughness(i.Tex);
	}
#endif
#endif

/************ PrepassFinal && PrepassFinalAdd ************/
#if LIGHTMODE == LIGHTMODE_PREPASS_FINAL || LIGHTMODE == LIGHTMODE_PREPASS_FINAL_ADD
struct PSPrepassFinalInput
{
	float2 Tex;
};
#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN_Surface(i, 0);
	MIR_DECLARE_VS_OUT(PSPrepassFinalInput, o, 0);

	void StageEntry_VSPrepassFinal()
	{
		gl_Position = float4(iPos, 1.0);
		o.Tex = iTex;
	}
#else
	MIR_DECLARE_PS_IN(PSPrepassFinalInput, i, 0);
	MIR_DECLARE_PS_OUT(vec4, oColor, 0);//worldPos(RGB), roughness(A)

	void PSPrepassFinal_()
	{
	#if LIGHTMODE == LIGHTMODE_FORWARD_BASE
		bool additive = false;
	#else
		bool additive = true;
	#endif

		LightingInput li;
		li.light_color = LightColor;
	
		float4 worldPosition = MIR_SAMPLE_TEX2D(_GBufferPos, i.Tex);//worldPos(RGB), roughness(A)
		li.world_pos = worldPosition.xyz * 2.0 - float3(1.0);
		li.percertual_roughness = worldPosition.w;
	
		float4 worldNormal = MIR_SAMPLE_TEX2D(_GBufferNormal, i.Tex);//worldNormal(RGB), metallic(A)
		worldNormal.xyz = worldNormal.xyz * 2.0 - float3(1.0);
		li.metallic = worldNormal.w;
		float3 toLight = normalize(LightPosition.xyz - li.world_pos * LightPosition.w);
		float3 toEye = normalize(CameraPositionExposure.xyz - li.world_pos);
	
		float4 albedo = MIR_SAMPLE_TEX2D(_GBufferAlbedo, i.Tex);//albedo(RGB), ao(A)
		li.albedo = albedo.rgb;
		li.ao = albedo.w;
	
		float4 emissive = MIR_SAMPLE_TEX2D(_GBufferEmissive, i.Tex);//emissive(RGB), transmissionFactor(A)
		li.emissive = emissive.rgb;
		li.transmission_factor = emissive.w;
	
		li.sheen_color_roughness = MIR_SAMPLE_TEX2D(_GBufferSheen, i.Tex);//sheenColor(RGB), sheenRoughness(A)
		li.clearcoat_color_roughness = MIR_SAMPLE_TEX2D(_GBufferClearCoat, i.Tex);
	
	#if DEBUG_CHANNEL == DEBUG_CHANNEL_GBUFFER_POS
		oColor = worldPosition;
	#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GBUFFER_NORMAL
		oColor = float4(worldNormal.xyz * 0.5 + 0.5, worldNormal.w);
	#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GBUFFER_ALBEDO
		oColor = albedo;
	#else
		oColor = Lighting(li, toLight, worldNormal.xyz, toEye, additive);
	#endif
	}
	void StageEntry_PSPrepassFinal() {
		PSPrepassFinal_();
	}
    void StageEntry_PSPrepassFinalAdd() {
	    PSPrepassFinal_();
	}
#endif
#endif