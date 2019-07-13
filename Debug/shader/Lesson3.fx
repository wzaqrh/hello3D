/********** Diffuse Light **********/
SamplerState samLinear : register(s0);
Texture2D txDiffuse : register(t0);
Texture2D txSpecular : register(t1);
Texture2D txNormal : register(t2);

struct LIGHT_STRUCT
{
	float4 LightPos;//world space
	float4 DiffuseColor;
	float4 SpecularColor;
	float  SpecularPower;
};

cbuffer cbGlobalParam : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	LIGHT_STRUCT DefLight = {{0.0,0.0,0.0,0.0}, {1.0,1.0,1.0,1.0}, {1.0,1.0,1.0,1.0}, 32.0};
}

static const int MAX_MATRICES = 256;
cbuffer cbWeightedSkin : register(b1)
{
	matrix Model;
	matrix Models[MAX_MATRICES] : WORLDMATRIXARRAY;	
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
	float2 Tex  : TEXCOORD0;
    float4 BlendWeights : BLENDWEIGHT;
    uint4  BlendIndices : BLENDINDICES;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;//eye space
	float3 Light : POSITION0;//eye space
	float3 Eye : POSITION1;//eye space
};

float4 Skinning(float4 iBlendWeights, uint4 iBlendIndices, float4 iPos)
{
    float BlendWeights[4] = (float[4])iBlendWeights;
	BlendWeights[3] = 1.0 - BlendWeights[0] - BlendWeights[1] - BlendWeights[2];
    uint  BlendIndices[4] = (uint[4])iBlendIndices;	
	
    float4	Pos = float4(0.0,0.0,0.0,1.0);   
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
	
	output.Normal = normalize(mul(MWV, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Normal.xyz, 1.0))).xyz);
	
	output.Pos = mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0)));	
	output.Light = normalize(DefLight.LightPos.xyz - output.Pos.xyz);
	output.Light = normalize(mul(View, float4(output.Light,1.0)).xyz);
	
	output.Eye = normalize(-output.Pos.xyz);
	output.Eye = mul(View, float4(output.Eye,1.0));
	
	output.Pos = mul(View, output.Pos);
    output.Pos = mul(Projection, output.Pos);
    
	output.Tex = i.Tex;
    return output;
}

float3 GetDiffuseBySampler(float3 normal, float3 light, float2 texcoord) {
	float diffuseFactor = saturate(dot(normal, light));
	float3 diffuseMat = txDiffuse.Sample(samLinear, texcoord).xyz;
	return diffuseMat * diffuseFactor * DefLight.DiffuseColor.xyz;
}

float3 GetSpecularByDef(float3 normal, float3 light, float3 eye) {
	float3 reflection = reflect(-light, normal);
	float specularFactor = saturate(dot(reflection, eye));
	specularFactor = pow(specularFactor, DefLight.SpecularPower);
	return specularFactor * DefLight.SpecularColor.xyz;
}

float3 GetSpecularBySampler(float3 normal, float3 light, float3 eye, float2 texcoord) {
	float3 specularMat = txSpecular.Sample(samLinear, texcoord).xyz;
	return GetSpecularByDef(normal, light, eye) * specularMat;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float3 normal = normalize(input.Normal);
	float3 light = normalize(input.Light);
	float3 eye = normalize(input.Eye);
	
	float3 diffuse = GetDiffuseBySampler(normal, light, input.Tex);
	float3 specular = GetSpecularBySampler(normal, light, eye, input.Tex);
	float3 color = diffuse + specular;
	
	return float4(color, 1.0);
}



