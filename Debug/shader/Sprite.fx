/********** Multi Light(Direct Point Spot) (eye space) (SpecularMap NormalMapping) **********/
SamplerState samLinear : register(s0);
Texture2D txDiffuse : register(t0);

static const int MAX_LIGHTS = 1;
cbuffer cbGlobalParam : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	matrix ViewInv;
}

struct VS_INPUT
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
	float2 Tex  : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	matrix WVP = mul(Projection,mul(View, World));
	
    output.Pos = mul(WVP, float4(input.Pos,1.0));
	output.Color = input.Color;
	output.Tex = input.Tex;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float4 diffuseColor = txDiffuse.Sample(samLinear, input.Tex);
	float4 finalColor = diffuseColor * input.Color;
	//float4 finalColor = input.Color;
	return finalColor;
}


