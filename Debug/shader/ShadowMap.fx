SamplerState samLinear : register(s0);
Texture2D txDepthMap : register(t0);

cbuffer cbGlobalParam : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	matrix ViewInv;
	
	int4 LightNum;
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

struct SHADOW_VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
	float3 Tangent : NORMAL1;
	float2 Tex  : TEXCOORD0;
    float4 BlendWeights : BLENDWEIGHT;
    uint4  BlendIndices : BLENDINDICES;
};

struct SHADOW_PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 PosInLight : TEXCOORD0;
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

SHADOW_PS_INPUT VS( SHADOW_VS_INPUT i)
{
	SHADOW_PS_INPUT output;
	
	matrix MW = mul(World, transpose(Model));
	float4 skinPos = Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0));
	
	matrix MWVP = mul(Projection,mul(View, MW));
    output.Pos = mul(MWVP, skinPos);
	
	matrix LightMWVP = mul(LightProjection,mul(LightView, MW));
	output.PosInLight = mul(LightMWVP, skinPos);
	
	return output;
}

static const float bias = 0.001f;
float4 PS(SHADOW_PS_INPUT i) : SV_Target
{
	float4 finalColor = float4(0.0,0.0,0.0,1.0);
	
	float2 projPosInLight = float2(i.PosInLight.x/i.PosInLight.w/2.0 + 0.5, -i.PosInLight.y/i.PosInLight.w/2.0 + 0.5);
	if((saturate(projPosInLight.x) == projPosInLight.x) && (saturate(projPosInLight.y) == projPosInLight.y))
	{
		float4 colorDepth = txDepthMap.Sample(samLinear, projPosInLight);
		float depthValue = colorDepth.a;
		float depthInLight = i.PosInLight.z / i.PosInLight.w - bias;
		if (depthInLight < depthValue) {
			finalColor.rgb = colorDepth.rgb;
		}
	}
	
	return finalColor;
}