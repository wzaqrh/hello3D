#ifndef BILATERAL_BLUR_H
#define BILATERAL_BLUR_H
#include "CommonFunction.cginc"

struct BilateralBlurInput
{
	float4 blurRadiusFallOffSharp;
	float4 resolution;
	float2 depthParam;
};

inline float BlurFunction(float2 uv, float r, float center_c, float center_d, inout float w_total,
					      BilateralBlurInput input, MIR_ARGS_TEX2D(tDepth), MIR_ARGS_TEX2D(tSource))
{
	float c = MIR_SAMPLE_TEX2D(tSource, uv);
	float d = fetch_eye_z(uv, input.depthParam, MIR_PASS_TEX2D(tDepth));

	float ddiff = d - center_d;
	float w = exp(-r * r * input.blurRadiusFallOffSharp.y - ddiff * ddiff * input.blurRadiusFallOffSharp.z);
	w_total += w;

	return w * c;
}

inline float4 BilateralBlurX(float2 tex, BilateralBlurInput input, MIR_ARGS_TEX2D(tDepth), MIR_ARGS_TEX2D(tSource))
{
	float b = 0;
	float w_total = 0;
	float center_c = MIR_SAMPLE_TEX2D(tSource, tex);
	float center_d = fetch_eye_z(tex, input.depthParam, MIR_PASS_TEX2D(tDepth));
    
	for (float r = -input.blurRadiusFallOffSharp.x; r <= input.blurRadiusFallOffSharp.x; ++r)
	{
		float2 uv = tex.xy + float2(r * input.resolution.z, 0);
		b += BlurFunction(uv, r, center_c, center_d, w_total, input, MIR_PASS_TEX2D(tDepth), MIR_PASS_TEX2D(tSource));
	}
	return b / w_total;
	//return MIR_SAMPLE_TEX2D(tSource, tex);
}

inline float4 BilateralBlurY(float2 tex, BilateralBlurInput input, MIR_ARGS_TEX2D(tDepth), MIR_ARGS_TEX2D(tSource))
{
	float b = 0;
	float w_total = 0;
	float center_c = MIR_SAMPLE_TEX2D(tSource, tex);
	float center_d = fetch_eye_z(tex, input.depthParam, MIR_PASS_TEX2D(tDepth));
    
	for (float r = -input.blurRadiusFallOffSharp.x; r <= input.blurRadiusFallOffSharp.x; ++r)
	{
		float2 uv = tex.xy + float2(0, r * input.resolution.w);
		b += BlurFunction(uv, r, center_c, center_d, w_total, input, MIR_PASS_TEX2D(tDepth), MIR_PASS_TEX2D(tSource));
	}
	return b / w_total;
}
#endif