/********** Multi Light(Direct Point) (eye space) (SpecularMap) **********/
#include "Standard.cginc"
#include "Skeleton.cginc"
#include "Lighting.cginc"
#if DEBUG_PBR > 0
#include "LightingPbr.cginc"
#endif

MIR_DECLARE_TEX2D(txAlbedo, 0);
MIR_DECLARE_TEX2D(txNormal, 1);
MIR_DECLARE_TEX2D(txMetalness, 2);
MIR_DECLARE_TEX2D(txSmoothness, 3);
MIR_DECLARE_TEX2D(txAmbientOcclusion, 4);

inline float3 GetAlbedo(float2 uv) 
{
    float3 albedo = float3(1.0, 1.0, 1.0);
	if (hasAlbedo) albedo = MIR_SAMPLE_TEX2D(txAlbedo, uv).rgb;
    return albedo * BaseColorFactor;
}

inline float GetMetalness(float2 uv) 
{
    float metalness = 0.0;
    #if OCCLUSION_ROUGHNESS_METALLIC
    if (hasMetalness) metalness = MIR_SAMPLE_TEX2D(txMetalness, uv).b;
    #else 
    if (hasMetalness) metalness = MIR_SAMPLE_TEX2D(txMetalness, uv).rgb;
    #endif
    return metalness * MetallicFactor;
}

inline float GetSmoothness(float2 uv) 
{
    float smoothness = 0.0;
    #if OCCLUSION_ROUGHNESS_METALLIC
    if (hasMetalness) smoothness = 1.0 - MIR_SAMPLE_TEX2D(txMetalness, uv).g * RoughnessFactor;
    #else
    if (hasRoughness) smoothness = lerp(1.0, MIR_SAMPLE_TEX2D(txSmoothness, uv).rgb, RoughnessFactor);
    #endif
    return smoothness;
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
	float3 Normal : NORMAL0;//eye space
	float3 ToEye  : TEXCOORD1;//eye space
	float3 ToLight : TEXCOORD2;//eye space
    float3x3 TangentBasis : TEXCOORD3;
#if ENABLE_SHADOW_MAP
	float4 PosInLight : TEXCOORD4;//light's ndc space
#endif
};

PixelInput VS(vbSurface surf, vbWeightedSkin skin)
{
	PixelInput output;
	matrix MW = mul(World, transpose(Model));
	
	//Normal
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal = normalize(mul(mul(View, MW), skinNormal).xyz);
	
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);

	//PosInLight
#if ENABLE_SHADOW_MAP
	output.PosInLight = mul(LightView, output.Pos);
	output.PosInLight = mul(LightProjection, output.PosInLight);
#endif
	
	//ToLight
	output.Pos = mul(View, output.Pos);	
	float4 lightPosition = mul(View, unity_LightPosition);
	output.ToLight = lightPosition.xyz - output.Pos.xyz * lightPosition.w;

#if ENABLE_SHADOW_MAP
	float bias = max(0.05 * (1.0 - dot(output.Normal, output.ToLight)), 0.005);
	output.PosInLight.z -= bias * output.PosInLight.w;	
#endif

    float3x3 TBN = float3x3(skin.Tangent, skin.BiTangent, skin.Normal);
    output.TangentBasis = mul((float3x3)MW, transpose(TBN));
    
	//ToEye
	output.ToEye = -output.Pos;
	
	//Pos
    output.Pos = mul(Projection, output.Pos);
	output.Tex = surf.Tex;
    return output;
}

float4 PS(PixelInput input) : SV_Target
{	
	float4 finalColor;
    
    float3 normal;
    if (hasNormal > 0)
    {
        float3 rawNormal = MIR_SAMPLE_TEX2D(txNormal, input.Tex).xyz;
        normal = normalize(2.0 * rawNormal - 1.0);
        normal = normalize(mul(input.TangentBasis, normal));
    }
    else 
    {
        normal = normalize(input.Normal);
    }
#if DEBUG_PBR == 0
	finalColor.rgb = MirBlinnPhongLight(input.ToLight, normalize(input.Normal), normalize(input.ToEye), GetAlbedo(input.Tex), IsSpotLight);
#elif DEBUG_PBR == 1
	finalColor.rgb = UnityPbrLight(input.ToLight, normalize(input.Normal), normalize(input.ToEye), 
		GetAlbedo(input.Tex), GetMetalness(input.Tex), GetSmoothness(input.Tex));
#else
    finalColor.rgb = GltfPbrLight(input.ToLight, normalize(input.Normal), normalize(input.ToEye),
		GetAlbedo(input.Tex), GetMetalness(input.Tex), GetSmoothness(input.Tex));
#endif
	finalColor.a = 1.0;
#if ENABLE_SHADOW_MAP
	finalColor.rgb *= CalcShadowFactor(input.PosInLight);
#endif
	return finalColor;
}

/************ ForwardAdd ************/
float4 PSAdd(PixelInput input) : SV_Target
{	
	float4 finalColor;
	finalColor.rgb = MirBlinnPhongLight(input.ToLight, normalize(input.Normal), normalize(input.ToEye), GetAlbedo(input.Tex), IsSpotLight);
	finalColor.a = 1.0;
	return finalColor;
}

/************ PrepassBase ************/
struct PSPrepassBaseInput
{
	float4 Pos  : SV_POSITION;
    float4 Pos0 : POSITION0;//world space
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL0;
};
PSPrepassBaseInput VSPrepassBase(vbSurface surf, vbWeightedSkin skin)
{
	PSPrepassBaseInput output;
	matrix MW = mul(World, transpose(Model));
	
	//Normal
	float4 skinNormal = Skinning(skin.BlendWeights, skin.BlendIndices, float4(skin.Normal.xyz, 0.0));
	output.Normal = mul(MW, skinNormal).xyz;
	
	//Pos
	float4 skinPos = Skinning(skin.BlendWeights, skin.BlendIndices, float4(surf.Pos.xyz, 1.0));
	output.Pos = mul(MW, skinPos);
	output.Pos = mul(View, output.Pos);
    output.Pos = mul(Projection, output.Pos);
	output.Pos0 = output.Pos;
	
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
#endif
};
PSPrepassBaseOutput PSPrepassBase(PSPrepassBaseInput input)
{
	PSPrepassBaseOutput output;
#if !DEBUG_PREPASS_BASE
	output.Pos = input.Pos0 / input.Pos0.w;
	output.Pos.xyz = output.Pos.xyz * 0.5 + 0.5;

	output.Normal.xyz = normalize(input.Normal);
	output.Normal.xyz = output.Normal.xyz * 0.5 + 0.5;
	output.Normal.w = 1.0;
	
	output.Albedo.rgb = GetAlbedo(input.Tex);
	output.Albedo.w = 1.0;
#else
	output.Pos.xyz = normalize(input.Normal);
	output.Pos.xyz = output.Pos.xyz * 0.5 + 0.5;
	output.Pos.w = 1.0;
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

float4 PSPrepassFinal(PSPrepassFinalInput input) : SV_Target
{
	float4 finalColor;
	
	float4 position;
	position.xyz = MIR_SAMPLE_TEX2D(_GBufferPos, input.Tex).xyz*2.0-1.0;
	position.w = 1.0;
	position = mul(mul(ViewInv, ProjectionInv), position);
	position /= position.w;
	
	float3 normal = MIR_SAMPLE_TEX2D(_GBufferNormal, input.Tex).rgb*2.0-1.0;
	float3 albedo = MIR_SAMPLE_TEX2D(_GBufferAlbedo, input.Tex).rgb;
	
	float3 toLight_ = unity_LightPosition.xyz - position.xyz * unity_LightPosition.w;
	
	float4 toEye = mul(View, position);
	toEye /= toEye.w;
	toEye = normalize(-toEye);
	finalColor.rgb = MirBlinnPhongLight(toLight_, normal, toEye, albedo, IsSpotLight);
	finalColor.a = 1.0;
	
#if ENABLE_SHADOW_MAP
	//PosInLight
	float4 posInLight = mul(LightView, position);
	posInLight = mul(LightProjection, posInLight);
	float bias = max(0.05 * (1.0 - dot(normal, normalize(toLight_))), 0.005);
	posInLight.z -= bias * posInLight.w;
	
	finalColor.rgb *= CalcShadowFactor(posInLight);
#endif
#if DEBUG_PREPASS_FINAL
	//finalColor.rgb = float3(input.Tex, 0);
	//finalColor.rgb = MIR_SAMPLE_TEX2D(_GBufferPos, input.Tex);
	//finalColor.rgb = MIR_SAMPLE_TEX2D(_GBufferAlbedo, input.Tex).rgb;
	//finalColor.rgb = MIR_SAMPLE_TEX2D(_GBufferNormal, input.Tex).rgb;
	//finalColor.rgb = normalize(toLight_) + 0.5;
	//finalColor.rgb = toEye;
#endif
	return finalColor;
}