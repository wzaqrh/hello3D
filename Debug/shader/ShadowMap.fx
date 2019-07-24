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

struct SHADOW_VS_INPUT
{
    float3 Pos : POSITION;
};

struct SHADOW_PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Depth : TEXCOORD0;
};

SHADOW_PS_INPUT VS( SHADOW_VS_INPUT i)
{
	SHADOW_PS_INPUT o;
    o.Pos = mul(World, float4(i.Pos,1.0));
	o.Pos = mul(View, o.Pos);
	o.Pos = mul(Projection, o.Pos);
    o.Depth.xy = o.Pos.zw;
	return o;
}

float4 PS(SHADOW_PS_INPUT i) : SV_Target
{
    float4 Color = i.Depth.x / i.Depth.y;
	//return Color;
	return float4(1.0,0.0,0.0,1.0);
}