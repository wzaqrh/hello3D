#include "Standard.cginc"
#include "Skeleton.cginc"
#include "CommonFunction.cginc"
#include "Shadow.cginc"
#include "Lighting.cginc"
#include "Lighting.cginc"
#include "ToneMapping.cginc"
#include "Macros.cginc"

#if !defined ENABLE_SHADOW_MAP
#define ENABLE_SHADOW_MAP 1
#endif

#if !defined USE_NORMAL
#define USE_NORMAL 1
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
#if RIGHT_HANDNESS_RESOURCE
	//uv *= float2(1.0,-1.0);
#endif
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

inline float3 GetNormal(float2 uv, float3 worldPos, float3 worldNormal, float3 tangent, float3 bitangent)
{
	float3 normal;
	
#if ! USE_NORMAL
	float3 dpx = ddx(worldPos);
	float3 dpy = ddy(worldPos);
    worldNormal = normalize(cross(dpy, dpx));
#endif

    if (EnableNormalMap) {
	#if NORMAL_PACKED
		float3 tangentNormal = UnpackScaleNormalRGorAG(MIR_SAMPLE_TEX2D(txNormal, GetUV(uv, NormalUV)), NormalScale);
	#else
		float3 tangentNormal = MIR_SAMPLE_TEX2D(txNormal, GetUV(uv, NormalUV)).xyz * 2.0 - 1.0;
		tangentNormal = normalize(tangentNormal * float3(NormalScale, NormalScale, 1.0));
	#endif
	
	#if 1	
		normal = normalize(tangent * tangentNormal.x + bitangent * tangentNormal.y + worldNormal * tangentNormal.z);
    #else
		float3x3 tbn = HasTangent ?  GetTBN(normalize(tangent), worldNormal) : GetTBN(worldPos, uv, worldNormal);
		normal = normalize(mul(tangentNormal, tbn));
	#endif
	}
	else {
		normal = worldNormal;
	}
    return normal;
}

/************ ForwardBase ************/
struct PixelInput
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float4 Color : COLOR;
	float3 ToEye  : TEXCOORD1;//world space
	float3 ToLight : TEXCOORD2;//world space
	float4 Normal : TEXCOORD3;//world space
    float4 Tangent : TEXCOORD4;
	float4 BiTangent : TEXCOORD5;
#if ENABLE_SHADOW_MAP
	float4 ViewPosLight : POSITION0;
	float4 PosLight : POSITION1; //light's ndc space
#endif    
#if DEBUG_TBN != 2
    float3 SurfPos : POSITION2;//world space
#else
    float3x3 TangentBasis : TEXCOORD3;
#endif
};

PixelInput VS(vbSurface surf, vbWeightedSkin skin)
{
	PixelInput output;
	matrix MW = mul(World, transpose(Model));
		
	//WorldPos
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, float4(surf.Pos.xyz, 1.0));
	
	//Normal && Tangent && BiTangent
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal.xyz = normalize(mul(MW, skinNormal).xyz);
	output.Normal.w = output.Pos.z;
	
    float4 skinTangent = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Tangent.xyz, 0.0));
	output.Tangent.xyz = normalize(mul(MW, skinTangent).xyz);
	output.Tangent.w = output.Pos.x;
	
	output.BiTangent.xyz = cross(output.Normal.xyz, output.Tangent.xyz) * skin.Tangent.w;
	output.BiTangent.w = output.Pos.y;
	
	//ToEye
	output.ToEye = CameraPosition.xyz - output.Pos.xyz;// * float3(-1,1,-1);
	
	//output.ToEye = CameraPosition.xyz - mul(transpose(Model), float4(surf.Pos.xyz, 1.0)).xyz;
	
	//PosLight
#if ENABLE_SHADOW_MAP
	output.ViewPosLight = mul(LightView, output.Pos);
	output.PosLight = mul(LightProjection, output.ViewPosLight);
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
    
#if ENABLE_SHADOW_MAP_BIAS
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
	float4 finalColor;
    
#if DEBUG_TBN != 2
	float3 bitangent = input.BiTangent.xyz;
	float3 vp = mul(View, float4(input.SurfPos, 1.0)).xyz;
    float3 normal = GetNormal(input.Tex, input.SurfPos, input.Normal.xyz, input.Tangent.xyz, bitangent);
#else        
    float3 normal = normalize(2.0 * MIR_SAMPLE_TEX2D(txNormal, input.Tex).xyz - 1.0);
    float3 basis_tangent = normalize(input.TangentBasis[0]);
    float3 basis_bitangent = normalize(input.TangentBasis[1]);
    float3 basis_normal = normalize(input.TangentBasis[2]);
    normal = normalize(float3(dot(basis_tangent, normal), dot(basis_bitangent, normal), dot(basis_normal, normal)));     
#endif
    
	float4 albedo = GetAlbedo(input.Tex);
	float3 toLight = normalize(input.ToLight);
	float3 toEye = normalize(input.ToEye);
	//float3 toEye = normalize(CameraPosition.xyz - input.SurfPos);
	float3 aorm = GetAmbientOcclusionRoughnessMetalness(input.Tex);
	float3 emissive = GetEmissive(input.Tex);
#if !PBR_MODE
    finalColor.rgb = BlinnPhongLight(toLight, normal, toEye, albedo.rgb, IsSpotLight);
#elif PBR_MODE == PBR_UNITY
	finalColor.rgb = UnityPbrLight(toLight, normal, toEye, albedo.rgb, aorm);
#elif PBR_MODE == PBR_GLTF
	finalColor.rgb = GltfPbrLight(toLight, normal, toEye, albedo.rgb, aorm, emissive);
#endif
    finalColor.a = 1.0;

#if ENABLE_SHADOW_MAP
	//float depth = length(unity_LightPosition.xyz - input.SurfPos.xyz * unity_LightPosition.w);
	//finalColor.rgb *= CalcShadowFactor(input.PosLight.xyz / input.PosLight.w, input.ViewPosLight.xyz);
#endif
	
#if DEBUG_CHANNEL == DEBUG_CHANNEL_UV_0
    finalColor.rgb = float3(input.Tex, 0);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_UV_1
    fcolor = MakeDummyColor(normalize(input.ToEye));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_OCCLUSION	
	if (! EnableAmbientOcclusionMap)
		finalColor.rgb = MakeDummyColor(normalize(input.ToEye));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_NORMAL_TEXTURE
	finalColor.rgb = EnableNormalMap ? MIR_SAMPLE_TEX2D(txNormal, GetUV(input.Tex, NormalUV)).xyz : MakeDummyColor(normalize(input.ToEye));
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_NORMAL
	finalColor.rgb = input.Normal * float3(1.0,1.0,-1.0) * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_TANGENT
	finalColor.rgb = input.Tangent * float3(1.0,1.0,-1.0) * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_BITANGENT
	finalColor.rgb = MakeDummyColor(normalize(input.ToEye));
#elif DEBUG_CHANNEL == DEBUG_WINDOW_POS
	finalColor.xyz = float3(input.Pos.xy * FrameBufferSize.zw, 0);
	finalColor.y   = 1.0 - finalColor.y;
#elif DEBUG_CHANNEL == DEBUG_CAMERA_POS
	finalColor.xyz = CameraPosition.xyz;
	finalColor.z   = -finalColor.z;
	finalColor.xyz = finalColor.xyz * 0.5 + 0.5;
#elif DEBUG_CHANNEL == DEBUG_SURFACE_POS
	finalColor.xyz = input.SurfPos * 32 * 0.5 + 0.5;
	finalColor.z   = -finalColor.z;
#endif
	//finalColor.xyz = albedo.xyz;
	//finalColor.xyz = input.Normal * 0.5 + 0.5;
	//finalColor.xyz = normal * 0.5 + 0.5;
	//finalColor.xyz = toEye;
	//finalColor.xyz = toLight;
	//finalColor.xyz = aorm;
	return finalColor/* * input.Color*/;
}

/************ ForwardAdd ************/
float4 PSAdd(PixelInput input) : SV_Target
{	
	float4 finalColor;
	float3 normal = GetNormal(input.Tex, input.SurfPos, input.Normal.xyz, input.Tangent.xyz, input.BiTangent.xyz);
	finalColor.rgb = BlinnPhongLight(input.ToLight, normal, normalize(input.ToEye), GetAlbedo(input.Tex).rgb, IsSpotLight);
	finalColor.a = 1.0;
	return finalColor;
}

/************ ShadowCaster ************/
struct PSShadowCasterInput 
{
	float4 Pos  : SV_POSITION;
	//float4 Pos0 : POSITION0;
	//_//float2 Tex : TEXCOORD0;
};
PSShadowCasterInput VSShadowCaster(vbSurface surf, vbWeightedSkin skin)
{
	PSShadowCasterInput output;
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos, 1.0));
	output.Pos = mul(mul(LightProjection, mul(LightView, mul(World, transpose(Model)))), skinPos);
	//output.Pos0 = output.Pos;
	//_//output.Tex = surf.Tex;
	return output;
}
float4 PSShadowCasterDebug(PSShadowCasterInput input) : SV_Target
{
	float4 finalColor = float4(1.0,0,0,1);
	//finalColor.xy = input.Pos0.xy / input.Pos0.w;
	//finalColor.xy = finalColor.xy * 0.5 + 0.5;
	//_//finalColor = GetAlbedo(input.Tex);
	return finalColor;
}

struct PSGenerateVSMInput
{
	float4 Pos : SV_POSITION;
	float4 ViewPos : POSITION0;
	float4 WorldPos : POSITION1;
};
PSGenerateVSMInput VSGenerateVSM(vbSurface surf, vbWeightedSkin skin)
{
	PSGenerateVSMInput output;
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos, 1.0));
	output.WorldPos = mul(mul(World, transpose(Model)), skinPos);
	output.ViewPos = mul(LightView, output.WorldPos);
	output.Pos = mul(LightProjection, output.ViewPos);
	return output;
}
float4 PSGenerateVSM(PSGenerateVSMInput input) : SV_Target
{
	float depth = length(input.ViewPos);
	//float worldPos = input.WorldPos.xyz / input.WorldPos.w;
	//float depth = length(unity_LightPosition.xyz - worldPos * unity_LightPosition.w);
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
    float4 Normal : SV_Target1;
    float4 Albedo : SV_Target2;
	float4 Emissive : SV_Target3;
};
PSPrepassBaseOutput PSPrepassBase(PSPrepassBaseInput input)
{
	PSPrepassBaseOutput output;
	output.Pos = float4(input.Pos.xyz / input.Pos.w * 0.5 + 0.5, 1.0);
	float3 aorm = GetAmbientOcclusionRoughnessMetalness(input.Tex);
	output.Normal = float4(GetNormal(input.Tex, input.SurfPos, input.Normal, input.Tangent, input.Tangent) * 0.5 + 0.5, aorm.x);
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
	//float Depth : SV_Depth;
};

PSPrepassFinalOutput PSPrepassFinal(PSPrepassFinalInput input)
{
	PSPrepassFinalOutput output;
	//output.Depth = MIR_SAMPLE_TEX2D_LEVEL(_GDepth, input.Tex, 0).r;
	
	float4 position = float4(MIR_SAMPLE_TEX2D(_GBufferPos, input.Tex).xyz * 2.0 - 1.0, 1.0);
	float4 worldPosition = mul(mul(ViewInv, ProjectionInv), position);
	worldPosition /= worldPosition.w;
	
	float4 normal = MIR_SAMPLE_TEX2D(_GBufferNormal, input.Tex);
	normal.xyz = normal.xyz * 2.0 - 1.0;
	float4 albedo = MIR_SAMPLE_TEX2D(_GBufferAlbedo, input.Tex);
	float4 emissive = MIR_SAMPLE_TEX2D(_GBufferEmissive, input.Tex);
	float3 aorm = float3(normal.w, albedo.w, emissive.w);
	
	float3 toLight = normalize(unity_LightPosition.xyz - worldPosition.xyz * unity_LightPosition.w);
	float3 toEye = normalize(CameraPosition.xyz - worldPosition.xyz);

#if !PBR_MODE
    output.Color.rgb = BlinnPhongLight(toLight, normal.xyz, toEye, albedo.xyz, IsSpotLight);
#elif PBR_MODE == PBR_UNITY
	output.Color.rgb = UnityPbrLight(toLight, normal.xyz, toEye, albedo.xyz, aorm);
#elif PBR_MODE == PBR_GLTF
	output.Color.rgb = GltfPbrLight(toLight, normal.xyz, toEye, albedo.rgb, aorm, emissive.xyz);
#endif
	output.Color.a = 1.0;
	
#if ENABLE_SHADOW_MAP
	float4 viewPosLight = mul(LightView, worldPosition);
	float4 posLight = mul(LightProjection, viewPosLight);
#if ENABLE_SHADOW_MAP_BIAS
	float bias = max(0.001 * (1.0 - dot(normal.xyz, toLight)), 1e-5);
	posLight.z -= bias * posLight.w;
#endif
	output.Color.rgb *= CalcShadowFactor(posLight.xyz / posLight.w, viewPosLight.xyz);
#endif
	
	//output.Color.xyz = aorm;
	//output.Color.xyz = toLight;
	//output.Color.xyz = toEye;
	//output.Color.xyz = albedo.xyz;
	//output.Color.xyz = emissive.xyz;
	//output.Color.xyz = normal.xyz;
	return output;
}