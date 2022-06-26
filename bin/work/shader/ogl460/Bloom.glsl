#include "Standard.glinc"
#include "Skeleton.glinc"
#include "CommonFunction.glinc"
#include "Lighting.glinc"
#include "Lighting.glinc"
#include "ToneMapping.glinc"
#include "Macros.glinc"
#include "BilateralBlur.glinc"
	
struct PixelInput
{
	float4 Color;
	float2 Tex;
};

#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN_Surface(i, 0);
	MIR_DECLARE_VS_OUT(PixelInput, o, 0);

	void StageEntry_VS()
	{
		gl_Position = float4(iPos, 1.0);
		o.Color = iColor;
	#if UV_STARTS_AT_TOP
		o.Tex = iTex;
	#else
		o.Tex = float2(iTex.x, 1.0 - iTex.y);
	#endif
	}
#else
	MIR_DECLARE_PS_IN(PixelInput, i, 0);
	MIR_DECLARE_PS_OUT(vec4, oColor, 0);

	MIR_DECLARE_TEX2D(OrgSceneImage, 9);

	layout (binding = 3, std140) uniform cbBloom
	{
		float4 _SceneImage_TexelSize;
		float4 BloomThresholdIntensity;
		float4 LensFlareThesholdIntensity;
		float4 BlurIterationSpread;
		float4 LensColors[4];
	};

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
		o.uv[2] = tex - resolution.zw * float2(1.0, -1.0) * 0.5;
		o.uv[3] = tex + resolution.zw * float2(1.0, -1.0) * 0.5;
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
		float4 outColor = float4(0.0);
		outColor += MIR_SAMPLE_TEX2D(tMainTex, i.uv[0].xy);
		outColor += MIR_SAMPLE_TEX2D(tMainTex, i.uv[1].xy);
		outColor += MIR_SAMPLE_TEX2D(tMainTex, i.uv[2].xy);
		outColor += MIR_SAMPLE_TEX2D(tMainTex, i.uv[3].xy);
		return outColor / 4;
	}

	void StageEntry_PSDownSample()
	{
		UVQuad quv = MakeUVQuad(i.Tex, _SceneImage_TexelSize);
		oColor = MultiTapMax(quv, MIR_PASS_TEX2D(_SceneImage));
	}

	void StageEntry_PSAvgBlur()
	{
		UVQuad quv = MakeUVQuad(i.Tex, _SceneImage_TexelSize);
		oColor = MultiTapBlur(quv, MIR_PASS_TEX2D(_SceneImage));
	}

	void StageEntry_PSColorThreshold()
	{
		oColor = MIR_SAMPLE_TEX2D(_SceneImage, i.Tex);
		oColor.rgb = max(oColor.rgb - BloomThresholdIntensity.rgb, float3(0.0));
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
	
		float4 color = float4(0.0f);
		for (int i = 0; i < fFilterWidth; ++i)
			color += MIR_SAMPLE_TEX2D(txMain, float2(fTexStart + fTexelOffset * i));
		return color / fFilterWidth;
	}

	void StageEntry_PSBlurX()
	{
		float spreadForPass = (1.0f + (BlurIterationSpread.y * 0.25f)) * BlurIterationSpread.x;
		oColor = BlurFunction(i.Tex, float2(1.0, 0.0) * spreadForPass, _SceneImage_TexelSize, MIR_PASS_TEX2D(_SceneImage));
	}

	void StageEntry_PSBlurY()
	{
		float spreadForPass = (1.0f + (BlurIterationSpread.y * 0.25f)) * BlurIterationSpread.x;
		oColor = BlurFunction(i.Tex, float2(0.0, 1.0) * spreadForPass, _SceneImage_TexelSize, MIR_PASS_TEX2D(_SceneImage));
	}

	void StageEntry_PSLensThreshold()
	{
		float4 color = MIR_SAMPLE_TEX2D(_SceneImage, i.Tex);
		color.rgb = max(color.rgb - LensFlareThesholdIntensity.rgb, float3(0.0));
		oColor = color;
	}

	void StageEntry_PSLensFlare()
	{
		float4 color = float4(0.0);
		color += MIR_SAMPLE_TEX2D(_SceneImage, ((i.Tex - 0.5 ) * -0.85 ) + 0.5) * LensColors[0];
		color += MIR_SAMPLE_TEX2D(_SceneImage, ((i.Tex - 0.5 ) * -1.45 ) + 0.5) * LensColors[1];
		color += MIR_SAMPLE_TEX2D(_SceneImage, ((i.Tex - 0.5 ) * -2.55 ) + 0.5) * LensColors[2];
		color += MIR_SAMPLE_TEX2D(_SceneImage, ((i.Tex - 0.5 ) * -4.15 ) + 0.5) * LensColors[3];
		oColor = color * LensFlareThesholdIntensity.w;
	}

	void StageEntry_PSBlendAdd()
	{
		float4 addedbloom = MIR_SAMPLE_TEX2D(_SceneImage, i.Tex);
		float4 screencolor = MIR_SAMPLE_TEX2D(OrgSceneImage, i.Tex);
		oColor = addedbloom * BloomThresholdIntensity.w + screencolor;
	}
#endif