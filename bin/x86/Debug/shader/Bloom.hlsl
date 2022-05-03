#include "Standard.cginc"
#include "Skeleton.cginc"
#include "CommonFunction.cginc"
#include "Lighting.cginc"
#include "LightingPbr.cginc"
#include "ToneMapping.cginc"
#include "Macros.cginc"
#include "BilateralBlur.cginc"

cbuffer cbBloom : register(b3)
{
	float4 _SceneImage_TexelSize;
	float4 BloomThresholdIntensity;
	float4 LensFlareThesholdIntensity;
	float4 BlurIterationSpread;
	float4 LensColors[4];
};
	
struct PixelInput
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD0;
};

PixelInput VS(vbSurface input)
{
	PixelInput output;
	output.Pos = float4(input.Pos, 1.0);
	output.Color = input.Color;
	output.Tex = input.Tex;
	return output;
}

struct UVQuad
{
	float2 uv[5];
};
UVQuad MakeUVQuad(float2 tex, float4 resolution)
{
	UVQuad o;
	o.uv[4] = tex;
	o.uv[0] = tex + resolution.zw * 0.5;
	o.uv[1] = tex - resolution.zw * 0.5;
	o.uv[2] = tex - resolution.zw * float2(1, -1) * 0.5;
	o.uv[3] = tex + resolution.zw * float2(1, -1) * 0.5;
	return o;
}
inline float4 MultiTapMax(UVQuad i, MIR_ARGS_TEX2D(tMainTex))
{
	float4 outColor = MIR_SAMPLE_TEX2D(tMainTex, i.uv[4].xy);
	outColor = max(outColor, MIR_SAMPLE_TEX2D(tMainTex, i.uv[0].xy));
	outColor = max(outColor, MIR_SAMPLE_TEX2D(tMainTex, i.uv[1].xy));
	outColor = max(outColor, MIR_SAMPLE_TEX2D(tMainTex, i.uv[2].xy));
	outColor = max(outColor, MIR_SAMPLE_TEX2D(tMainTex, i.uv[3].xy));
	return outColor;
}
inline float4 MultiTapBlur(UVQuad i, MIR_ARGS_TEX2D(tMainTex))
{
	float4 outColor = 0;
	outColor += MIR_SAMPLE_TEX2D(tMainTex, i.uv[0].xy);
	outColor += MIR_SAMPLE_TEX2D(tMainTex, i.uv[1].xy);
	outColor += MIR_SAMPLE_TEX2D(tMainTex, i.uv[2].xy);
	outColor += MIR_SAMPLE_TEX2D(tMainTex, i.uv[3].xy);
	return outColor / 4;
}

float4 PSDownSample(PixelInput input) : SV_Target
{
	UVQuad quv = MakeUVQuad(input.Tex, _SceneImage_TexelSize);
	return MultiTapMax(quv, MIR_PASS_TEX2D(_SceneImage));
}

float4 PSAvgBlur(PixelInput input) : SV_Target
{
	UVQuad quv = MakeUVQuad(input.Tex, _SceneImage_TexelSize);
	return MultiTapBlur(quv, MIR_PASS_TEX2D(_SceneImage));
}

float4 PSColorThreshold(PixelInput input) : SV_Target
{
	float4 color = MIR_SAMPLE_TEX2D(_SceneImage, input.Tex);
	color.rgb = max(color.rgb - BloomThresholdIntensity.rgb, float3(0, 0, 0));
	return color;
}

inline float BoxFilterStart(float fWidth)  //Assumes filter is odd
{
	return ((fWidth - 1.0f) / 2.0f);
}
inline float4 BlurFunction(float2 uv, float2 texelStep, float4 resolution, MIR_ARGS_TEX2D(txMain))
{
	const float fFilterWidth = 9.0f;
	
	float fStartOffset = BoxFilterStart(fFilterWidth);
	float2 fTexelOffset = texelStep * resolution.zw;
	float2 fTexStart = uv - (fStartOffset * fTexelOffset);
	
	float4 output = (float4) 0.0f;
	for (int i = 0; i < fFilterWidth; ++i)
		output += MIR_SAMPLE_TEX2D(txMain, float2(fTexStart + fTexelOffset * i));
	return output / fFilterWidth;
}

float4 PSBlurX(PixelInput input) : SV_Target
{
	float spreadForPass = (1.0f + (BlurIterationSpread.y * 0.25f)) * BlurIterationSpread.x;
	return BlurFunction(input.Tex, float2(1.0, 0.0) * spreadForPass, _SceneImage_TexelSize, MIR_PASS_TEX2D(_SceneImage));
}

float4 PSBlurY(PixelInput input) : SV_Target
{
	float spreadForPass = (1.0f + (BlurIterationSpread.y * 0.25f)) * BlurIterationSpread.x;
	return BlurFunction(input.Tex, float2(0.0, 1.0) * spreadForPass, _SceneImage_TexelSize, MIR_PASS_TEX2D(_SceneImage));
}

float4 PSLensThreshold(PixelInput input) : SV_Target
{
	float4 color = MIR_SAMPLE_TEX2D(_SceneImage, input.Tex);
	color.rgb = max(color.rgb - LensFlareThesholdIntensity.rgb, float3(0, 0, 0));
	return color;
}

float4 PSLensFlare(PixelInput i) : SV_Target
{
	float4 color = 0;
	color += MIR_SAMPLE_TEX2D(_SceneImage, ((i.Tex - 0.5 ) * -0.85 ) + 0.5) * LensColors[0];
	color += MIR_SAMPLE_TEX2D(_SceneImage, ((i.Tex - 0.5 ) * -1.45 ) + 0.5) * LensColors[1];
	color += MIR_SAMPLE_TEX2D(_SceneImage, ((i.Tex - 0.5 ) * -2.55 ) + 0.5) * LensColors[2];
	color += MIR_SAMPLE_TEX2D(_SceneImage, ((i.Tex - 0.5 ) * -4.15 ) + 0.5) * LensColors[3];
	return color * LensFlareThesholdIntensity.w;
}

float4 PSBlendAdd(PixelInput i) : SV_Target
{
	float4 addedbloom = MIR_SAMPLE_TEX2D(_SceneImage, i.Tex);
	float4 screencolor = MIR_SAMPLE_TEX2D(_GBufferAlbedo, i.Tex);
	return addedbloom * BloomThresholdIntensity.w + screencolor;
}