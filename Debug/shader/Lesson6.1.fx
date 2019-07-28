/********** PBR **********/
SamplerState samLinear : register(s0);

Texture2D albedoTexture : register(t0);//rgb
Texture2D normalTexture : register(t1);//rgb
Texture2D metalnessTexture : register(t2);//r
Texture2D smoothnessTexture : register(t3);//r
Texture2D aoTexture : register(t4);//r

struct LIGHT_DIRECT
{
	float4 LightPos;//world space
	float4 DiffuseColor;
	float4 SpecularColorPower;
};

struct LIGHT_POINT
{
	LIGHT_DIRECT L;
	float4 Attenuation;
};

struct LIGHT_SPOT
{
	LIGHT_POINT Base;
	float3 Direction;
    float Cutoff;
};

static const int MAX_LIGHTS = 4;
cbuffer cbGlobalParam : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	matrix ViewInv;
	
	int4 LightNum;
	LIGHT_DIRECT DirectLights[MAX_LIGHTS];
	LIGHT_POINT PointLights[MAX_LIGHTS];
	LIGHT_SPOT SpotLights[MAX_LIGHTS];
}

static const int MAX_MATRICES = 256;
cbuffer cbWeightedSkin : register(b1)
{
	matrix Model;
	matrix Models[MAX_MATRICES] : WORLDMATRIXARRAY;	
	int hasNormal;
	int hasMetalness;
	int hasRoughness;
	int hasAO;
}

cbuffer cbUnityMaterial : register(b2)
{
	float4 _SpecColor;
	float4 _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	int _SpecLightOff;
}

cbuffer cbUnityGlobal : register(b3)
{
	float4 _Unity_IndirectSpecColor;
	float4 _AmbientOrLightmapUV;
};

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
	float3 Tangent : NORMAL1;
	float2 Tex  : TEXCOORD0;
    float4 BlendWeights : BLENDWEIGHT;
    uint4  BlendIndices : BLENDINDICES;
	float3 BiTangent : NORMAL2;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;//world space
	float3 Tangent : NORMAL1;//world space
	float3 BiTangent : NORMAL2;//world space
	float3 Eye : POSITION0;//world space
	float3 SurfacePosition : POSITION1;//world space
	float3x3 TangentBasis : TBASIS;
};

float4 Skinning(float4 iBlendWeights, uint4 iBlendIndices, float4 iPos)
{
    float BlendWeights[4] = (float[4])iBlendWeights;
	BlendWeights[3] = 1.0 - BlendWeights[0] - BlendWeights[1] - BlendWeights[2];
    uint  BlendIndices[4] = (uint[4])iBlendIndices;	
	
    float4	Pos = float4(0.0,0.0,0.0,iPos.w);   
	const int NumBones = 4;
    for (int iBone = 0; iBone < NumBones; iBone++) {
		uint Indice = BlendIndices[iBone];
		float Weight = BlendWeights[iBone];
			
		float4 bonePos = mul(iPos, Models[Indice]);
        Pos.xyz += bonePos.xyz * Weight;
    }
	return Pos;
}

PS_INPUT VS(VS_INPUT i)
{
	PS_INPUT output = (PS_INPUT)0;

	matrix MW = mul(World, transpose(Model));
	matrix MWV = mul(View, MW);
	
	output.Tangent = normalize(mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Tangent.xyz, 0.0))).xyz);
	output.BiTangent = normalize(mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.BiTangent.xyz, 0.0))).xyz);
	output.Normal = normalize(mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Normal.xyz, 0.0))).xyz);
	
	output.Pos = mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0)));
	output.SurfacePosition = output.Pos.xyz;
	
	output.Pos = mul(View, output.Pos);	
	output.Eye = mul(ViewInv, float4(0.0,0.0,0.0,1.0)).xyz;
    
	output.Pos = mul(Projection, output.Pos);
    
	float3x3 TBN = float3x3(i.Tangent, i.BiTangent, i.Normal);
	output.TangentBasis = mul((float3x3)MW, transpose(TBN));
	
	output.Tex = i.Tex;
    return output;
}

#define SHADER_TARGET 30
#define _ALPHAPREMULTIPLY_ON
#define UNITY_SETUP_BRDF_INPUT MetallicSetup

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
#if defined(_SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A)
    sg.rgb = metalnessTexture.Sample(samLinear, uv).rgb;
    sg.a = smoothnessTexture.Sample(samLinear, uv).r;
#else
    sg.rgb = metalnessTexture.Sample(samLinear, uv).rgb;
	sg.a = smoothnessTexture.Sample(samLinear, uv).r;
#endif
    sg.a *= _GlossMapScale;
    return sg;
}
float3 Albedo(float2 i_tex)//_Color.rgb * tex2D(_MainTex, texcoords.xy).rgb
{
#if defined(_SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A)
    return _Color.a;
#else
    return _Color.rgb * albedoTexture.Sample(samLinear, i_tex).rgb;
	//return _Color.rgb * pow(albedoTexture.Sample(samLinear, i_tex).rgb, 2.2);
#endif
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
        mg.x = tex2D(_MetallicGlossMap, uv).r;//_MetallicGlossMap("Metallic", 2D) = "white" {}
        mg.y = tex2D(_MainTex, uv).a;
    #else
        mg.x = metalnessTexture.Sample(samLinear, uv).r;
		mg.y = smoothnessTexture.Sample(samLinear, uv).r;
    #endif
	mg.y *= _GlossMapScale;
	//_METALLICGLOSSMAP
    return mg;
}
#define unity_ColorSpaceDielectricSpec half4(0.220916301, 0.220916301, 0.220916301, 1.0 - 0.220916301)
float OneMinusReflectivityFromMetallic(float metallic)
{
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
    return _Color.a;
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
    return diffColor * alpha;
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
UnityLight MainLight(LIGHT_DIRECT directLight)//{color=_LightColor0.rgb,dir=_WorldSpaceLightPos0.xyz}
{
    UnityLight l;
    l.color = directLight.DiffuseColor.rgb;
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
    return aoTexture.Sample(samLinear, uv).g;
#else
    float occ = aoTexture.Sample(samLinear, uv).g;
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
float3 UnityGI_IndirectSpecular(UnityGIInput data, float occlusion)//unity_IndirectSpecColor.rgb*occlusion
{
    float3 specular;
    specular = _Unity_IndirectSpecColor.rgb;
    return specular * occlusion;
}
UnityGI UnityGlobalIllumination(UnityGIInput data, float occlusion, Unity_GlossyEnvironmentData glossIn)//std_branch_this
{
    UnityGI o_gi = UnityGI_Base(data, occlusion);
    o_gi.indirect.specular = UnityGI_IndirectSpecular(data, occlusion);
    return o_gi;
}
UnityGI FragmentGI(FragmentCommonData s, float occlusion, UnityLight light)
{
    UnityGIInput d;
    d.light = light;
    d.ambient = _AmbientOrLightmapUV.rgb;
    Unity_GlossyEnvironmentData g = UnityGlossyEnvironmentSetup(s.smoothness);
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

#define UNITY_COLORSPACE_GAMMA
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

float3 CalDirectLight(LIGHT_DIRECT directLight, float3 normal, float3 toLight, float3 toEye, float2 texcoord) 
{
	FragmentCommonData s = FragmentSetup(texcoord);
	UnityLight mainLight = MainLight(directLight);
	float occlusion = Occlusion(texcoord);
	UnityGI gi = FragmentGI(s, occlusion, mainLight);
	float3 c = UNITY_BRDF_PBS(s, gi, normal, toLight, toEye, texcoord);
	//OutputForward
	return c;
}
float3 CalPointLight(LIGHT_POINT pointLight, float3 normal, float3 toLight, float3 toEye, float2 texcoord, float Distance) {
	float3 color = CalDirectLight(pointLight.L, normal, toLight, toEye, texcoord);
	float Attenuation = pointLight.Attenuation.x 
	+ pointLight.Attenuation.y * Distance 
	+ pointLight.Attenuation.z * Distance * Distance;
	return color / Attenuation;
}
float3 CalSpotLight(LIGHT_SPOT spotLight, float3 normal, float3 toLight, float3 toEye, float2 texcoord, float Distance, float3 spotDirection) {
	float3 color = 0.0;
	float spotFactor = dot(toLight, spotDirection);
	if (spotFactor > spotLight.Cutoff) {
		color = CalPointLight(spotLight.Base, normal, toLight, toEye, texcoord, Distance);
        color = color * ((spotFactor - spotLight.Cutoff) / (1.0 - spotLight.Cutoff));
	}
	return color;
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
	float3 bump = normalTexture.Sample(samLinear, texcoord).xyz * 2.0 - 1.0;
	bump = mul(bump, tbn);
	return bump;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float3 toEye = normalize(input.Eye - input.SurfacePosition);
	//float3 toEye = normalize(float3(0.0,0.0,-150.0) - input.SurfacePosition);
	
	float3 normal;
	if (hasNormal > 0) {
		//float3x3 tbn = CalTBN(input.Normal, input.Tangent, input.BiTangent);
		//normal = GetBumpBySampler(tbn, input.Tex);
		float3 rawNormal = normalTexture.Sample(samLinear, input.Tex).xyz;
		normal = normalize(2.0 * rawNormal - 1.0);
		normal = normalize(mul(input.TangentBasis, normal));
	}
	else {
		normal = normalize(input.Normal);
	}
	
	float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
	for (int i = 0; i < LightNum.x; ++i) {//direction light
		float3 toLight = normalize(-DirectLights[i].LightPos.xyz);
		finalColor.xyz += CalDirectLight(DirectLights[i], normal, toLight, toEye, input.Tex);
	}
	for (int i = 0; i < LightNum.y; ++i) {//point light
		float3 toLight = PointLights[i].L.LightPos.xyz - input.SurfacePosition.xyz;
		float Distance = length(toLight);
		toLight = normalize(toLight);
		finalColor.xyz += CalPointLight(PointLights[i], normal, toLight, toEye, input.Tex, Distance);
	}
	for (int i = 0; i < LightNum.z; ++i) {//spot light
		float3 toLight = SpotLights[i].Base.L.LightPos.xyz - input.SurfacePosition.xyz;
		float Distance = length(toLight);
		toLight = normalize(toLight);
		
		float3 spotDirection = -SpotLights[i].Direction.xyz;
		finalColor.xyz += CalSpotLight(SpotLights[i], normal, toLight, toEye, input.Tex, Distance, spotDirection);
	}

#if 0
	{
		float3 ao;
		if (hasAO)
			ao = aoTexture.Sample(samLinear, input.Tex).xyz;
		else
			ao = 0.0;
		
		float3 albedo = albedoTexture.Sample(samLinear, input.Tex).rgb; 
		albedo = pow(albedo, 2.2);
		
		float3 ambient = albedo * ao * 0.03;
		finalColor.xyz += ambient;
	}
#endif

    //finalColor.xyz = finalColor.xyz / (finalColor.xyz + 1.0); // HDR tonemapping
    //finalColor.xyz = pow(finalColor.xyz, 1.0/2.2); // gamma correct
	return finalColor;
}



