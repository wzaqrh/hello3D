/********** Diffuse Light **********/
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

struct LIGHT_STRUCT
{
	float4 LightPos;//world space
	float4 LightColor;
};

cbuffer cbGlobalParam : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	LIGHT_STRUCT DefLight = {{0.0,0.0,0.0,0.0}, {1.0,1.0,1.0,1.0}};
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
	float3 Normal : NORMAL;
	float3 Light : POSITION0;
	float3 Eye : POSITION1;
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

	output.Normal = normalize(mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Normal.xyz, 1.0)))).xyz;
	output.Pos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0));
	output.Pos = mul(MW, output.Pos);
	output.Light = normalize(DefLight.LightPos - output.Pos).xyz;
	
    output.Pos = mul(View, output.Pos);
    output.Pos = mul(Projection, output.Pos);
    
	output.Tex = i.Tex;
    return output;
}

float3 GetDiffuse(float3 normal, float3 light, float2 texcoord) {
	float diffuseFactor = saturate(dot(normal, light));
	float3 diffuseMat = txDiffuse.Sample(samLinear, texcoord).xyz;
	return diffuseMat * diffuseFactor * DefLight.LightColor;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float3 normal = normalize(input.Normal);
	float3 light = normalize(input.Light);
	float3 color = GetDiffuse(normal, light, input.Tex);
	return float4(color, 1.0);
}