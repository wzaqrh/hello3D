/********** Multi Light(Direct Point Spot) (eye space) (SpecularMap NormalMapping) **********/
SamplerState samLinear : register(s0);
Texture2D txDiffuse : register(t0);
Texture2D txSpecular : register(t1);
Texture2D txNormal : register(t2);
Texture2D txDepthMap : register(t3);

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
}

cbuffer cbShadowMap : register(b2)
{
	matrix LightView;
	matrix LightProjection;
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
	float3 Normal : NORMAL;//eye space
	float3 Tangent : NORMAL1;//eye space
	float3 Eye : POSITION0;//eye space
	float3 SurfacePos : POSITION1;//eye space
	float4 PosInLight : POSITION2;
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
	
	output.Tangent = normalize(mul(MWV, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Tangent.xyz, 0.0))).xyz);
	output.Normal = normalize(mul(MWV, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Normal.xyz, 0.0))).xyz);
	
	float4 skinPos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0));
	output.Pos = mul(MWV, skinPos);	
	output.SurfacePos = output.Pos.xyz / output.Pos.w;
	output.Eye = -output.Pos;
	
    output.Pos = mul(Projection, output.Pos);
	
	matrix LightMWVP = mul(LightProjection,mul(LightView, MW));
	output.PosInLight = mul(LightMWVP, skinPos);
	
	output.Tex = i.Tex;//float2(i.Tex.x, 1.0 - i.Tex.y);
    return output;
}

float3 GetDiffuseBySampler(float3 normal, float3 light, float3 lightDiffuseColor, float2 texcoord) {
	float diffuseFactor = saturate(dot(normal, light));
	float3 diffuseMat = txDiffuse.Sample(samLinear, texcoord).xyz;
	return diffuseMat * diffuseFactor * lightDiffuseColor;
}

float3 GetSpecularByDef(float3 normal, float3 light, float3 eye, float4 SpecColorPower) {
	float3 reflection = reflect(-light, normal);
	float specularFactor = saturate(dot(reflection, eye));
	specularFactor = pow(specularFactor, SpecColorPower.w);
	return specularFactor * SpecColorPower.xyz;
}
float3 GetSpecularBySampler(float3 normal, float3 light, float3 eye, float4 SpecColorPower, float2 texcoord) {
	float3 specularMat = txSpecular.Sample(samLinear, texcoord).xyz;
	return GetSpecularByDef(normal, light, eye, SpecColorPower) * specularMat;
}

float3 CalDirectLight(LIGHT_DIRECT directLight, float3 normal, float3 light, float3 eye, float2 texcoord) {
	float3 color = 0.0;
	if (dot(normal, light) > 0.0) {
		float3 diffuse = GetDiffuseBySampler(normal, light, directLight.DiffuseColor.xyz, texcoord);
		float3 specular = GetSpecularBySampler(normal, light, eye, directLight.SpecularColorPower, texcoord);
		color = diffuse + specular;	
	}
	return color;
}

float3 CalPointLight(LIGHT_POINT pointLight, float3 normal, float3 light, float3 eye, float2 texcoord, float Distance) {
	float3 color = CalDirectLight(pointLight.L, normal, light, eye, texcoord);
	float Attenuation = pointLight.Attenuation.x 
	+ pointLight.Attenuation.y * Distance 
	+ pointLight.Attenuation.z * Distance * Distance;
	return color / Attenuation;
}

float3 CalSpotLight(LIGHT_SPOT spotLight, float3 normal, float3 light, float3 eye, float2 texcoord, float Distance, float3 spotDirection) {
	float3 color = 0.0;
	float spotFactor = dot(light, spotDirection);
	if (spotFactor > spotLight.Cutoff) {
		color = CalPointLight(spotLight.Base, normal, light, eye, texcoord, Distance);
        color = color * ((spotFactor - spotLight.Cutoff) / (1.0 - spotLight.Cutoff));
	}
	return color;
}

float3x3 CalTBN(float3 normal, float3 tangent) {
	float3 n = normalize(normal);
	float3 t = normalize(tangent - normal * dot(normal,tangent));
	float3 b = normalize(cross(n,t));
	return float3x3(t,b,n);
}
float3 GetBumpBySampler(float3x3 tbn, float2 texcoord) {
	float3 bump = txNormal.Sample(samLinear, texcoord).xyz * 2.0 - 1.0;
	bump = mul(bump, tbn);
	return bump;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float3 eye = normalize(input.Eye);
	
	//float3 normal = normalize(input.Normal);
	float3x3 tbn = CalTBN(input.Normal, input.Tangent);
	float3 normal = GetBumpBySampler(tbn, input.Tex);
	
	float4 color = float4(0.0, 0.0, 0.0, 1.0);
	for (int i = 0; i < LightNum.x; ++i) {
		float3 light = normalize(mul(View, float4(-DirectLights[i].LightPos.xyz,0.0)).xyz);
		color.xyz += CalDirectLight(DirectLights[i], normal, light, eye, input.Tex);
	}
	for (int i = 0; i < LightNum.y; ++i) {
		float3 light = mul(View, float4(PointLights[i].L.LightPos.xyz,1.0)).xyz - input.SurfacePos.xyz;
		float Distance = length(light);
		light = normalize(light);
		color.xyz += CalPointLight(PointLights[i], normal, light, eye, input.Tex, Distance);
	}
	for (int i = 0; i < LightNum.z; ++i) {
		float3 light = mul(View, float4(SpotLights[i].Base.L.LightPos.xyz,1.0)).xyz - input.SurfacePos.xyz;
		float Distance = length(light);
		light = normalize(light);
		
		float3 spotDirection = normalize(mul(View, float4(-SpotLights[i].Direction,0.0)).xyz);
		color.xyz += CalSpotLight(SpotLights[i], normal, light, eye, input.Tex, Distance, spotDirection);
	}
	
	float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
	vec4 posInLight = input.PosInLight;
	float2 projPosInLight = float2(posInLight.x/posInLight.w/2.0 + 0.5, -posInLight.y/posInLight.w/2.0 + 0.5);
	if((saturate(projPosInLight.x) == projPosInLight.x) && (saturate(projPosInLight.y) == projPosInLight.y))
	{
		float depthInLight = posInLight.z / posInLight.w - bias;
		float depthMap = txDepthMap.Sample(samLinear, projPosInLight).r;
		if (depthInLight < depthMap) {
			finalColor.rgb = color.rgb;
		}
	}
	
	return color;
}


