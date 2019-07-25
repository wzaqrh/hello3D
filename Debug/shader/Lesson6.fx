/********** PBR **********/
SamplerState samLinear : register(s0);

Texture2D albedoTexture : register(t0);//rgb
Texture2D normalTexture : register(t1);//rgb
Texture2D metalnessTexture : register(t2);//r
Texture2D roughnessTexture : register(t3);//r
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

//function D,using Trowbridge-Reitz GGX
static const float PI = 3.141592;
float ndfGGX(float NdotHLV, float roughness) {
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (NdotHLV * NdotHLV) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

//function G, using Smith’s Schlick-GGX
float gaSchlickG1(float cosTheta, float k) {
	return cosTheta / (cosTheta * (1.0 - k) + k);
}
float gaSchlickGGX(float NdotL, float NdotV, float roughness) {
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(NdotL, k) * gaSchlickG1(NdotV, k);
}

//function F,using Fresnel-Schlick Approximation
float3 fresnelSchlick(float3 F0, float HLVdotV) {
	return F0 + (1.0 - F0) * pow(1.0 - HLVdotV, 5.0);
}

static const float Epsilon = 0.00001;
static const float3 Fdielectric = 0.04;
float3 CalDirectLight(LIGHT_DIRECT directLight, float3 normal, float3 toLight, float3 toEye, float2 texcoord) 
{
	//反射辐射率(specular+diffuse)
	float3 Lo = 0.0;
	
	float3 albedo = albedoTexture.Sample(samLinear, texcoord).rgb; 
	
	float3 metalness;
	if (hasMetalness > 0)
		metalness = metalnessTexture.Sample(samLinear, texcoord).rgb;
	else
		metalness = 0.01;
	
	float roughness;
	if (hasRoughness > 0)
		roughness = roughnessTexture.Sample(samLinear, texcoord).r;
	else
		roughness = 0.01;
		
	albedo = pow(albedo, 2.2);
	metalness = pow(metalness, 2.2);
	roughness = pow(roughness, 2.2);
	
	//cos
	float NdotL = max(0.0, dot(normal, toLight));
	float NdotV = max(0.0, dot(normal, toEye));
	
	float3 HalfLV = normalize(toLight + toEye);
	float NdotHLV = max(0.0, dot(normal, HalfLV));

	//specularBRDF
	float3 F0 = lerp(Fdielectric, albedo, metalness);
	float3 F  = fresnelSchlick(F0, max(0.0, dot(HalfLV, toEye)));
	float D = ndfGGX(NdotHLV, roughness);
	float G = gaSchlickGGX(NdotL, NdotV, roughness);
	float3 Fcook_torrance = (D * F * G) / max(Epsilon, 4.0 * NdotL * NdotV); 
	float3 kS = F;
	float3 specularBRDF = Fcook_torrance;// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	
	//diffuseBRDF
	float3 kD = 1.0 - kS;
	kD = lerp(kD, 0.0, metalness);
	float3 Flambert = albedo / PI;
	float3 diffuseBRDF = kD * Flambert;
	
	float3 Li = directLight.DiffuseColor.xyz;
	Lo += (diffuseBRDF + specularBRDF) * Li * NdotL;
	
	//Lo = albedo;
	//Lo = metalness;
	//Lo = roughness;
	//Lo = float3(normal.x,normal.y,-normal.z);
	//Lo = -normal.z;
	//Lo = normalTexture.Sample(samLinear, texcoord).xyz;
	//Lo = float3(toLight.x,toLight.y,-toLight.z);
	//Lo = float3(toEye.x,toEye.y,-toEye.z);
	//Lo = float3(HalfLV.x,HalfLV.y,-HalfLV.z);
	//Lo = NdotL;
	//Lo = NdotV;
	//Lo = NdotHLV;
	//Lo = F;
	//Lo = diffuseBRDF;
	//Lo = D;
	//Lo = G;
	//Lo = D * G;
	//Lo = ndfGGX(NdotHLV, 0.1) * G;
	//Lo = specularBRDF;
	return Lo;
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
	
	// HDR tonemapping
    finalColor.xyz = finalColor.xyz / (finalColor.xyz + 1.0);
    // gamma correct
    finalColor.xyz = pow(finalColor.xyz, 1.0/2.2); 
	
	return finalColor;
}



