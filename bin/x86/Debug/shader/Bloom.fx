/********** Bloom **********/
#include "Standard.h"

#if SHADER_MODEL > 30000
Texture2D txFirst : register(t0);
Texture2D txSecond : register(t1);
Texture2D txThird : register(t2);
#else
texture  textureFirst : register(t0);
sampler2D txFirst : register(s0) = sampler_state { 
	Texture = <textureFirst>; 
};

texture  textureSecond : register(t1);
sampler2D txSecond : register(s1) = sampler_state { 
	Texture = <textureSecond>; 
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;
};

texture  textureThird : register(t2);
sampler2D txThird : register(s2) = sampler_state { 
	Texture = <textureThird>; 
};
#endif

static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);
static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;
static const float  BRIGHT_THRESHOLD = 0.5f;

cbuffer cbBloom : register(b1)
{
	float4 SampleOffsets[16];
	float4 SampleWeights[16];
}

struct VS_INPUT
{
    float4 Pos : POSITION;
	float2 Tex  : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
    output.Pos = input.Pos;
	output.Tex = input.Tex;
	return output;
}

float4 DownScale2x2(PS_INPUT input) : SV_Target
{
#if 0
    float4 vColor = 0.0f;
    float  fAvg = 0.0f;
    for( int i = 0; i < 9; i++ )
    {
		vColor = GetTexture2D(txFirst, samLinear, input.Tex + SampleOffsets[i].xy);
        fAvg += dot(vColor.rgb, LUMINANCE_VECTOR);
    }
    fAvg /= 9;
#else
	float4 vColor = GetTexture2D(txFirst, samLinear, input.Tex);
    float fAvg = dot(vColor.rgb, LUMINANCE_VECTOR);
#endif
    return float4(fAvg, fAvg, fAvg, 1.0f);
}

float4 DownScale3x3(PS_INPUT input) : SV_Target
{
#if 0
    float fAvg = 0.0f; 
    float4 vColor;
    for( int i = 0; i < 9; i++ )
    {
        vColor = GetTexture2D(txFirst, samPoint, input.Tex + SampleOffsets[i].xy);
        fAvg += vColor.r; 
    }
    fAvg /= 9;
#else
	float4 vColor = GetTexture2D(txFirst, samLinear, input.Tex);
    float fAvg = vColor.r;
#endif
    return float4(fAvg, fAvg, fAvg, 1.0f);
}

float4 DownScale3x3_BrightPass(PS_INPUT input) : SV_Target
{
    float3 vColor = 0.0f;
    float  fLum = GetTexture2D(txSecond, samPoint, float2(0.5f, 0.5f)).r;
    for (int i = 0; i < 9; i++ )
    {
        float4 vSample = GetTexture2D(txFirst, samPoint, input.Tex + SampleOffsets[i].xy);
        vColor += vSample.rgb;
    }
    vColor /= 9;
	
    vColor = max(0.0f, vColor - BRIGHT_THRESHOLD);
    vColor *= MIDDLE_GRAY / (fLum + 0.001f);
    vColor *= (1.0f + vColor/LUM_WHITE);
	vColor /= (1.0f + vColor);
	
    return float4(vColor, 1.0f);
}

float4 BloomPS(PS_INPUT input) : SV_Target
{
    float4 vSample = 0.0f;
    for( int iSample = 0; iSample < 15; iSample++ )
    {
        float4 vColor = GetTexture2D(txFirst, samPoint, input.Tex + SampleOffsets[iSample].xy);
        vSample += SampleWeights[iSample] * vColor;
    }
    return vSample;
}

float4 FinalPass(PS_INPUT input) : SV_Target
{
    float4 vColor = GetTexture2D(txFirst, samPoint, input.Tex );
    float fLum = GetTexture2D(txSecond, samPoint, float2(0.5f,0.5f)).r;
    float3 vBloom = GetTexture2D(txThird, samLinear, input.Tex);
     
    // Tone mapping
    vColor.rgb *= MIDDLE_GRAY / (fLum + 0.001f);
    vColor.rgb *= (1.0f + vColor/LUM_WHITE);
	vColor.rgb /= (1.0f + vColor);
	
	vColor.rgb += 0.6f * vBloom;
	vColor.a = 1.0f;
	
	return vColor;
}



















