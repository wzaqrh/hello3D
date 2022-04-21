#include "Standard.cginc"
#include "Skeleton.cginc"
#include "Math.cginc"
#include "Lighting.cginc"
#include "LightingPbr.cginc"
#include "ToneMapping.cginc"
#include "Debug.cginc"

#if !defined ENABLE_SHADOW_MAP
#define ENABLE_SHADOW_MAP 1
#endif

MIR_DECLARE_TEX2D(txAlbedo, 0);
MIR_DECLARE_TEX2D(txNormal, 1);
MIR_DECLARE_TEX2D(txMetalness, 2);
MIR_DECLARE_TEX2D(txRoughness, 3);
MIR_DECLARE_TEX2D(txAmbientOcclusion, 4);
MIR_DECLARE_TEX2D(txEmissive, 5);

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
	float3 EmissiveFactor;
	
	bool EnableAlbedoMap;
    bool EnableNormalMap;
	bool EnableAmbientOcclusionMap;
    bool EnableRoughnessMap;
	bool EnableMetalnessMap;
	bool EnableEmissiveMap;
	
	bool AmbientOcclusion_ChannelGRoughness_ChannelBMetalness;
    bool AlbedoMapSRGB;
	bool EmissiveMapSRGB;
    bool HasTangent;
}

inline float2 GetUV(float2 uv, float4 uvTransform) {
	return uvTransform.xy + uv * uvTransform.zw;
}

inline float4 GetAlbedo(float2 uv) 
{
    float4 albedo = AlbedoFactor;
    if (EnableAlbedoMap) {
		float4 color = MIR_SAMPLE_TEX2D(txAlbedo, GetUV(uv, AlbedoUV));
        if (AlbedoMapSRGB) color = sRGBToLinear(color);
		albedo *= color;
    }
    return albedo;
}

inline float3 GetEmissive(float2 uv) 
{
    float3 emissive = EmissiveFactor;
    if (EnableEmissiveMap) {
		float3 color = MIR_SAMPLE_TEX2D(txEmissive, GetUV(uv, EmissiveUV)).rgb;
		if (EmissiveMapSRGB) color = sRGBToLinear(color);
        emissive *= color;
    }
    return emissive;
}

inline float3 GetAmbientOcclusionRoughnessMetalness(float2 uv)
{
    float3 value = float3(1.0, RoughnessFactor, MetallicFactor);
    if (AmbientOcclusion_ChannelGRoughness_ChannelBMetalness) {
		float3 arm = MIR_SAMPLE_TEX2D(txAmbientOcclusion, GetUV(uv, OcclusionUV)).rgb;
		value.x = lerp(1.0, arm.x, OcclusionStrength);
		value.yz *= arm.yz;
	}
    else {
        if (EnableAmbientOcclusionMap) {
			float ao = MIR_SAMPLE_TEX2D(txAmbientOcclusion, GetUV(uv, OcclusionUV)).r;
			value.x = 1.0 + (ao - 1.0) * value.x;
		}
        if (EnableRoughnessMap) value.y *= MIR_SAMPLE_TEX2D(txRoughness, GetUV(uv, RoughnessUV)).r;
        if (EnableMetalnessMap) value.z *= MIR_SAMPLE_TEX2D(txMetalness, GetUV(uv, MetallicUV)).r;
    }
    return value;
}

inline float3 GetNormal(float2 uv, float3 worldPos, float3 worldNormal, float3 tangent)
{
    float3x3 tbn;
    if (HasTangent) tbn = GetTBN(worldNormal, normalize(tangent));
	else tbn = GetTBN(uv, worldPos, worldNormal);
	
    float3 normal;
    if (EnableNormalMap) {
		float3 tangentNormal = MIR_SAMPLE_TEX2D(txNormal, GetUV(uv, NormalUV)).xyz * 2.0 - 1.0;
		tangentNormal = normalize(tangentNormal * float3(NormalScale, NormalScale, 1.0));
        normal = normalize(mul(tangentNormal, tbn));
    }
	else {
		normal = tbn[2];		  
	}
#if DEBUG_CHANNEL == DEBUG_CHANNEL_NORMAL_TEXTURE
    normal = tangentNormal;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_NORMAL
    normal = tbn[2];
    normal.z = -normal.z;//compare gltf-sample-viewer
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_TANGENT
    normal = tbn[0];
	normal.z = -normal.z;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_BITANGENT 
    normal = tbn[1];
	normal.z = -normal.z;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHADING_NORMAL 
    normal.z = -normal.z;//compare gltf-sample-viewer
#endif	
    return normal;
}

/************ ShadowCaster ************/
struct PSShadowCasterInput 
{
	float4 Pos  : SV_POSITION;
	float4 Pos0 : POSITION0;
};

PSShadowCasterInput VSShadowCaster(vbSurface surf, vbWeightedSkin skin)
{
	PSShadowCasterInput output;
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos, 1.0));
	matrix MW = mul(World, transpose(Model));
	matrix MWV = mul(View, MW);
	matrix MWVP = mul(Projection, MWV);
	output.Pos = mul(MWVP, skinPos);
	output.Pos0 = output.Pos;
	return output;
}

float4 PSShadowCasterDebug(PSShadowCasterInput input) : SV_Target
{
	float4 finalColor = float4(0,0,0,1);
	//finalColor.xy = input.Pos0.xy / input.Pos0.w;
	//finalColor.xy = finalColor.xy * 0.5 + 0.5;
	//finalColor.y = 0;
	finalColor.z = input.Pos0.z / input.Pos0.w;
	return finalColor;
}

/************ ForwardBase ************/
struct PixelInput
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL0;//world space
    float3 Tangent : NORMAL1;
	float3 ToEye  : TEXCOORD1;//world space
	float3 ToLight : TEXCOORD2;//world space
#if ENABLE_SHADOW_MAP
	float4 PosInLight : POSITION0;//light's ndc space
#endif    
#if DEBUG_TBN != 2
    float3 SurfPos : POSITION1;//world space
#else
    float3x3 TangentBasis : TEXCOORD3;
#endif
};

PixelInput VS(vbSurface surf, vbWeightedSkin skin)
{
	PixelInput output;
	matrix MW = mul(World, transpose(Model));
	
	//Normal && Tangent
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal = normalize(mul(MW, skinNormal).xyz);
	
    float4 skinTangent = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Tangent.xyz, 0.0));
	output.Tangent = normalize(mul(MW, skinTangent).xyz);
    
	//ToEye
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);
	
	output.ToEye = CameraPosition.xyz - output.Pos.xyz;
    
	//PosInLight
#if ENABLE_SHADOW_MAP
	output.PosInLight = mul(LightView, output.Pos);
	output.PosInLight = mul(LightProjection, output.PosInLight);
#endif
	
    //ToLight
    output.ToLight = unity_LightPosition.xyz - output.Pos.xyz * unity_LightPosition.w;
    
	//SurfPos	
#if DEBUG_TBN != 2 
	output.SurfPos = output.Pos.xyz / output.Pos.w;
#else    
    float3x3 TBN = float3x3(skin.Tangent, skin.BiTangent, skin.Normal);
    output.TangentBasis = mul((float3x3)mul(View, MW), transpose(TBN));    
#endif
    
#if ENABLE_SHADOW_MAP
	float bias = max(0.05 * (1.0 - dot(output.Normal, output.ToLight)), 0.005);
	output.PosInLight.z -= bias * output.PosInLight.w;	
#endif
	
	//Pos
    output.Pos = mul(View, output.Pos);
    output.Pos = mul(Projection, output.Pos);
	output.Tex = surf.Tex;
    return output;
}

float4 PS(PixelInput input) : SV_Target
{	
	float4 finalColor;
    
#if DEBUG_TBN != 2
    float3 normal = GetNormal(input.Tex, input.SurfPos, input.Normal, input.Tangent);
#else        
    float3 normal = normalize(2.0 * MIR_SAMPLE_TEX2D(txNormal, input.Tex).xyz - 1.0);
    float3 basis_tangent = normalize(input.TangentBasis[0]);
    float3 basis_bitangent = normalize(input.TangentBasis[1]);
    float3 basis_normal = normalize(input.TangentBasis[2]);
    normal = normalize(float3(dot(basis_tangent, normal), dot(basis_bitangent, normal), dot(basis_normal, normal)));
    //normal = normalize(mul(input.TangentBasis, normal));        
#endif     
    
	float4 albedo = GetAlbedo(input.Tex);
	float3 toLight = normalize(input.ToLight);
	float3 toEye = normalize(input.ToEye);
#if !PBR_MODE
    finalColor.rgb = BlinnPhongLight(toLight, normal, toEye, albedo.rgb, IsSpotLight);
#elif PBR_MODE == PBR_UNITY
	float3 aorm = GetAmbientOcclusionRoughnessMetalness(input.Tex);
	finalColor.rgb = UnityPbrLight(toLight, normal, toEye, albedo.rgb, aorm);
#elif PBR_MODE == PBR_GLTF
	float3 aorm = GetAmbientOcclusionRoughnessMetalness(input.Tex);
	float3 emissive = GetEmissive(input.Tex);
	finalColor.rgb = gltfPbrLight(toLight, normal, toEye, albedo.rgb, aorm, emissive);
#endif
    finalColor.a = 1.0;

#if ENABLE_SHADOW_MAP
	finalColor.rgb *= CalcShadowFactor(input.PosInLight);
#endif
	
	//finalColor.xyz = input.Normal;
	//finalColor.xyz = normal;
	//finalColor.xyz = toEye;
	//finalColor.xyz = toLight;
	//finalColor.xyz = aorm;
#if DEBUG_CHANNEL == DEBUG_CHANNEL_UV_0
    finalColor.rgb = float3(input.Tex, 0);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_UV_1
    fcolor = MakeDummyColor(normalize(input.ToEye));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_OCCLUSION	
	if (! EnableAmbientOcclusionMap)
		finalColor.rgb = MakeDummyColor(normalize(input.ToEye));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_NORMAL_TEXTURE
	if (EnableNormalMap) {
		finalColor.xyz = normal;
		finalColor.rgb = (finalColor.xyz + 1.0) / 2.0;
	}
	else {
		finalColor.rgb = MakeDummyColor(normalize(input.ToEye));	
	}
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_NORMAL || DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_TANGENT || DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_BITANGENT || DEBUG_CHANNEL == DEBUG_CHANNEL_SHADING_NORMAL
	finalColor.xyz = normal;
	finalColor.rgb = (finalColor.xyz + 1.0) / 2.0;
#elif DEBUG_CHANNEL == DEBUG_WINDOW_POS
	finalColor.xyz = float3(input.Pos.xy * FrameBufferSize.zw, 0);
	finalColor.y = 1.0 - finalColor.y;
#elif DEBUG_CHANNEL == DEBUG_CAMERA_POS
	finalColor.xyz = CameraPosition.xyz;
	finalColor.z = -finalColor.z;
	finalColor.xyz = (finalColor.xyz + 1.0) / 2.0;
#elif DEBUG_CHANNEL == DEBUG_SURFACE_POS
	finalColor.xyz = input.SurfPos;
	finalColor.z = -finalColor.z;
	finalColor.xyz = (finalColor.xyz * 32 + 1.0) / 2.0;
#endif
	return finalColor;
}

/************ ForwardAdd ************/
float4 PSAdd(PixelInput input) : SV_Target
{	
	float4 finalColor;
	float3 normal = GetNormal(input.Tex, input.SurfPos, input.Normal, input.Tangent);
	finalColor.rgb = BlinnPhongLight(input.ToLight, normal, normalize(input.ToEye), GetAlbedo(input.Tex).rgb, IsSpotLight);
	finalColor.a = 1.0;
	return finalColor;
}

/************ PrepassBase ************/
struct PSPrepassBaseInput
{
	float4 SVPos : SV_POSITION;
    float4 Pos : POSITION0;//world space
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL0;
	float3 Tangent : NORMAL1;
#if DEBUG_TBN != 2
	float3 SurfPos : POSITION1; //world space
#else
    float3x3 TangentBasis : TEXCOORD3;
#endif
};
PSPrepassBaseInput VSPrepassBase(vbSurface surf, vbWeightedSkin skin)
{
	PSPrepassBaseInput output;
	matrix MW = mul(World, transpose(Model));
	
	//Normal && Tangent
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal = normalize(mul(MW, skinNormal).xyz);
	
	float4 skinTangent = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Tangent.xyz, 0.0));
	output.Tangent = normalize(mul(MW, skinTangent).xyz);
	
	//Pos && SurfPos
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);
	output.SurfPos = output.Pos.xyz / output.Pos.w;
	
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
#if !DEBUG_PREPASS_BASE
    float4 Normal : SV_Target1;
    float4 Albedo : SV_Target2;
	float4 Emissive : SV_Target3;
#endif
};
PSPrepassBaseOutput PSPrepassBase(PSPrepassBaseInput input)
{
	PSPrepassBaseOutput output;
#if !DEBUG_PREPASS_BASE
	output.Pos = float4(input.Pos.xyz / input.Pos.w * 0.5 + 0.5, 1.0);

	float3 aorm = GetAmbientOcclusionRoughnessMetalness(input.Tex);
	output.Normal = float4(GetNormal(input.Tex, input.SurfPos, input.Normal, input.Tangent) * 0.5 + 0.5, aorm.x);
	output.Albedo = float4(GetAlbedo(input.Tex).xyz, aorm.y);
	output.Emissive = float4(GetEmissive(input.Tex).xyz, aorm.z);
#else
	output.Pos.xyz = float4(normalize(input.Normal) * 0.5 + 0.5, 1.0);
#endif
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
	float Depth : SV_Depth;
};

PSPrepassFinalOutput PSPrepassFinal(PSPrepassFinalInput input)
{
	PSPrepassFinalOutput output;
	output.Depth = MIR_SAMPLE_LEVEL_TEX2D_SAMPLER(_ShadowMapTexture, _GDepth, input.Tex, 0).r;
	
	float4 position = float4(MIR_SAMPLE_TEX2D(_GBufferPos, input.Tex).xyz * 2.0 - 1.0, 1.0);
	position = mul(mul(ViewInv, ProjectionInv), position);
	position /= position.w;
	
	float4 normal = MIR_SAMPLE_TEX2D(_GBufferNormal, input.Tex);
	normal.xyz = normal.xyz * 2.0 - 1.0;
	float4 albedo = MIR_SAMPLE_TEX2D(_GBufferAlbedo, input.Tex);
	float4 emissive = MIR_SAMPLE_TEX2D(_GBufferEmissive, input.Tex);
	float3 aorm = float3(normal.w, albedo.w, emissive.w);
	
	float3 toLight = normalize(unity_LightPosition.xyz - position.xyz * unity_LightPosition.w);
	float3 toEye = normalize(CameraPosition.xyz - position.xyz);

#if !PBR_MODE
    output.Color.rgb = BlinnPhongLight(toLight, normal.xyz, toEye, albedo.xyz, IsSpotLight);
#elif PBR_MODE == PBR_UNITY
	output.Color.rgb = UnityPbrLight(toLight, normal.xyz, toEye, albedo.xyz, aorm);
#elif PBR_MODE == PBR_GLTF
	output.Color.rgb = gltfPbrLight(toLight, normal.xyz, toEye, albedo.rgb, aorm, emissive.xyz);
#endif
	output.Color.a = 1.0;
	
#if ENABLE_SHADOW_MAP1
	float4 PosInLight = mul(LightView, position);
	PosInLight = mul(LightProjection, PosInLight);
	
	float bias = max(0.05 * (1.0 - dot(normal.xyz, toLight)), 0.005);
	PosInLight.z -= bias * PosInLight.w;
	
	finalColor.rgb *= CalcShadowFactor(PosInLight);
#endif
	
	//finalColor.xyz = aorm;
	//finalColor.xyz = toLight;
	//finalColor.xyz = toEye;
	//finalColor.xyz = albedo.xyz;
	//finalColor.xyz = emissive.xyz;
	//finalColor.xyz = normal.xyz;
	return output;
}