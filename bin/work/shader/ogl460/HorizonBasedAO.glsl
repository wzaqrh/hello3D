#include "Standard.glinc"
#include "Skeleton.glinc"
#include "CommonFunction.glinc"
#include "Lighting.glinc"
#include "Lighting.glinc"
#include "ToneMapping.glinc"
#include "WhiteNoise.glinc"
#include "BilateralBlur.glinc"
#include "Macros.glinc"
	
#if !defined USE_GNORMAL
	#define USE_GNORMAL 1
#endif

#define DEBUG_SSAO_CHANNEL_AO 1
#define DEBUG_SSAO_CHANNEL_BLUR 2
#if !defined DEBUG_SSAO_CHANNEL
	#define DEBUG_SSAO_CHANNEL 0
#endif

layout (binding = 3, std140) uniform cbHBAo
{
	float4 DepthParam;//f/(fn), (n-f)/(fn)
	float4 FocalLen; //g_FocalLen, g_InvFocalLen
	float4 Radius; //g_R, g_sqr_R, g_inv_R
	float4 NumStepDirContrast; //g_NumSteps, g_NumDir, g_Contrast
	float4 AttenTanBias; //m_Attenuation, g_TanAngleBias
	float4 BlurRadiusFallOffSharp; //g_BlurRadius, g_BlurFalloff, g_Sharpness
};

struct PixelInput
{
	float4 Color;
	float2 texUV;
};
#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN_Surface(i, 0);
	MIR_DECLARE_VS_OUT(PixelInput, o, 0);

	void StageEntry_VS()
	{
		gl_Position = float4(iPos, 1.0);
		o.Color = iColor;
	#if UV_STARTS_AT_TOP
		o.texUV = iTex;
	#else
		o.texUV = float2(iTex.x, 1.0 - iTex.y);
	#endif
	}
#else
	MIR_DECLARE_PS_IN(PixelInput, i, 0);
	MIR_DECLARE_PS_OUT(vec4, oColor, 0);

	MIR_DECLARE_TEX2D(PrePassOutput, 9);

	//探索半径 <- deltaUV * numSteps
	inline float AccumulatedHorizonOcclusion_Quality(float2 deltaUV, float2 uv0, float3 P,
													 float numSteps, float randstep, float3 dPdu, float3 dPdv, 
													 float4 depthParam, float4 focalLen, MIR_ARGS_TEX2D(tDepth))
	{
		float2 uv = (uv0 + deltaUV) + randstep * deltaUV;//Jitter starting point within the first sample distance
    
		float2 snapped_duv = snap_uv_offset(uv - uv0, FrameBufferSize);
		float3 T = tangent_vector(snapped_duv, dPdu, dPdv);//snapped_duv投影到'相机空间'作为'horizon tangent水平切线'
		float tanH = tangent(T) + AttenTanBias.y;

		float ao = 0;
		float h0 = 0;
		for (float j = 0; j < numSteps; ++j)
		{
			float2 snapped_uv = snap_uv_coord(uv, FrameBufferSize);
			float3 S = fetch_eye_pos(snapped_uv, depthParam, focalLen, MIR_PASS_TEX2D(tDepth));
			uv += deltaUV;

			// Ignore any samples outside the radius of influence
			float d2 = length2(S - P);
			if (d2 < Radius.y)
			{
				float tanS = tangent(P, S);
				if (tanS > tanH)
				{
					// Compute tangent vector associated with snapped_uv
					float2 snapped_duv = snapped_uv - uv0;
					float3 T = tangent_vector(snapped_duv, dPdu, dPdv);
					float tanT = tangent(T) + AttenTanBias.y;

					// Compute AO between tangent T and sample S
					float sinS = tan_to_sin(tanS);
					float sinT = tan_to_sin(tanT);
					float r = sqrt(d2) * Radius.z;
					float h = sinS - sinT;
					ao += falloff(r, AttenTanBias.x) * (h - h0);
					h0 = h;

					// Update the current horizon angle
					tanH = tanS;
				}
			}
		}
		return ao;
	}

	void StageEntry_PSAO()
	{
		float3 P = fetch_eye_pos(i.texUV, DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
		//oColor = float4(P * 0.1, 1.0); return ;

		// Radius从'相机空间'投影到'切线空间'
		// 乘以0.5是为了改变范围[-1,1]到[0,1] #1/h = g_FocalLen / P.z
		// step_size表示探索半径
		float2 step_size = 0.5 * Radius.x * FocalLen.xy / P.z; 

		float numSteps = min(NumStepDirContrast.x, min(step_size.x * FrameBufferSize.x, step_size.y * FrameBufferSize.y));
		if (numSteps < 1.0) {//当projected radius小于1像素, 马上结束
			oColor = float4(1.0);
			return ;
		} 
		step_size = step_size / (numSteps + 1);

	#if USE_GNORMAL
		float3 N = normalize(MIR_SAMPLE_TEX2D_LEVEL(_GBufferNormal, i.texUV, 0).xyz * 2.0 - float3(1.0));
		N = mat3(View) * N;
	
		float4 tangentPlane = float4(N, dot(P, N));
		//'tangent plane切平面'上最近点
		float3 Pr = tangent_eye_pos(i.texUV + float2(FrameBufferSize.z, 0), tangentPlane, DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
		float3 Pl = tangent_eye_pos(i.texUV + float2(-FrameBufferSize.z, 0), tangentPlane, DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
		float3 Pt = tangent_eye_pos(i.texUV + float2(0, FrameBufferSize.w), tangentPlane, DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
		float3 Pb = tangent_eye_pos(i.texUV + float2(0, -FrameBufferSize.w), tangentPlane, DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
	#else
		float3 Pr = fetch_eye_pos(i.texUV + float2(g_InvResolution.x, 0), DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
		float3 Pl = fetch_eye_pos(i.texUV + float2(-g_InvResolution.x, 0), DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
		float3 Pt = fetch_eye_pos(i.texUV + float2(0, g_InvResolution.y), DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
		float3 Pb = fetch_eye_pos(i.texUV + float2(0, -g_InvResolution.y), DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
		float3 N = normalize(cross(Pr - Pl, Pt - Pb));
		float4 tangentPlane = float4(N, dot(P, N));
	#endif
		//oColor = float4(N * 0.5 + float3(0.5), 1.0); return ;

		//切平面的'Screen-aligned basis屏幕坐标系'
		float3 dPdu = min_diff(P, Pr, Pl);
		float3 dPdv = min_diff(P, Pt, Pb) * (FrameBufferSize.y * FrameBufferSize.z);

		float2 noise2 = rand2dTo2d(i.texUV);
		float dangle = 2.0 * C_PI / NumStepDirContrast.y * noise2.x;
		float3 rand = float3(cos(dangle), sin(dangle), noise2.y);//(cos(alpha),sin(alpha),jitter)
	
		float ao = 0;
		float alpha = 2.0 * C_PI / NumStepDirContrast.y;
		for (float d = 0; d < NumStepDirContrast.y; d++)
		{
			float angle = alpha * d;
			float2 dir = float2(cos(angle), sin(angle));
			float2 deltaUV = rotate_direction(dir, rand.xy) * step_size.xy;
			ao += AccumulatedHorizonOcclusion_Quality(deltaUV, i.texUV, P, numSteps, rand.z, dPdu, dPdv, 
													  DepthParam, FocalLen, MIR_PASS_TEX2D(_GDepth));
		}

		//return float4(i.texUV, 0.0, 1.0);
		//return float4(noise2, 0.0, 1.0);
		//return float4(N, 1.0);
		//return float4(P, 1.0);
		//return float4(dPdu, 1.0);
		oColor = float4(1.0 - ao / NumStepDirContrast.y * NumStepDirContrast.z);
	}

	void StageEntry_PSBlurX()
	{
		BilateralBlurInput blurIn;
		blurIn.blurRadiusFallOffSharp = BlurRadiusFallOffSharp;
		blurIn.resolution = FrameBufferSize;
		blurIn.depthParam = DepthParam;
	
	#if DEBUG_SSAO_CHANNEL == DEBUG_SSAO_CHANNEL_AO
		oColor = MIR_SAMPLE_TEX2D(PrePassOutput, i.texUV);
	#else
		oColor = BilateralBlurX(i.texUV, blurIn, MIR_PASS_TEX2D(_GDepth), MIR_PASS_TEX2D(PrePassOutput));
	#endif
	}

	void StageEntry_PSBlurY()
	{
		BilateralBlurInput blurIn;
		blurIn.blurRadiusFallOffSharp = BlurRadiusFallOffSharp;
		blurIn.resolution = FrameBufferSize;
		blurIn.depthParam = DepthParam;
	#if DEBUG_SSAO_CHANNEL == DEBUG_SSAO_CHANNEL_AO
		oColor = MIR_SAMPLE_TEX2D(PrePassOutput, i.texUV);
	#elif DEBUG_SSAO_CHANNEL == DEBUG_SSAO_CHANNEL_BLUR
		oColor = BilateralBlurY(i.texUV, blurIn, MIR_PASS_TEX2D(_GDepth), MIR_PASS_TEX2D(PrePassOutput));
	#else
		oColor = BilateralBlurY(i.texUV, blurIn, MIR_PASS_TEX2D(_GDepth), MIR_PASS_TEX2D(PrePassOutput));
		oColor *= MIR_SAMPLE_TEX2D(_SceneImage, i.texUV);
	#endif
	}
#endif