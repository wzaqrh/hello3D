#include "Standard.cginc"
#include "Skeleton.cginc"
#include "Math.cginc"
#include "Lighting.cginc"
#include "LightingPbr.cginc"
#include "ToneMapping.cginc"
#include "WhiteNoise.cginc"
#include "Debug.cginc"
	
#if !defined USE_GNORMAL
#define USE_GNORMAL 1
#endif

cbuffer cbHBAo : register(b3)
{
	float4 DepthParam;//f/(fn), (n-f)/(fn)
	float4 FocalLen; //g_FocalLen, g_InvFocalLen
	float4 Radius; //g_R, g_sqr_R, g_inv_R
	float4 NumStepDirContrast; //g_NumSteps, g_NumDir, g_Contrast
	float4 AttenTanBias; //m_Attenuation, g_TanAngleBias
};

struct PixelInput
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 texUV : TEXCOORD0;
};

PixelInput VS(vbSurface input)
{
	PixelInput output;
	output.Pos = float4(input.Pos, 1.0);
	output.Color = input.Color;
	output.texUV = input.Tex;
	return output;
}

inline float LinearEyeDepth(float d)
{
#if 0
	#define FARZ 1000.0
	#define NEARZ 0.3
	return FARZ * NEARZ / (FARZ - d * (FARZ - NEARZ));
#else
	return 1.0 / (DepthParam.x + DepthParam.y * d);
#endif
}
float3 uv_to_eye(float2 uv, float eye_z)
{
	uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));
	return float3(uv * FocalLen.zw * eye_z, eye_z);
}
float3 fetch_eye_pos(float2 uv)
{
	float d = MIR_SAMPLE_LEVEL_TEX2D_SAMPLER(_ShadowMapTexture, _GDepth, uv, 0).r;
	return uv_to_eye(uv, LinearEyeDepth(d));
}
float3 tangent_eye_pos(float2 uv, float4 tangentPlane)
{
    //view vector going through the surface point at uv
	float3 V = fetch_eye_pos(uv);
	//intersect with tangent plane except for silhouette edges
	float NdotV = dot(tangentPlane.xyz, V);
	if (NdotV < 0.0) V *= (tangentPlane.w / NdotV);
	return V;
}
float length2(float3 v) { return dot(v, v); }
float3 min_diff(float3 P, float3 Pr, float3 Pl)
{
	float3 V1 = Pr - P;
	float3 V2 = P - Pl;
	return (length2(V1) < length2(V2)) ? V1 : V2;
}
float2 rotate_direction(float2 Dir, float2 CosSin)
{
	return float2(Dir.x * CosSin.x - Dir.y * CosSin.y,
                  Dir.x * CosSin.y + Dir.y * CosSin.x);
}

float2 snap_uv_offset(float2 uv)
{
	return round(uv * FrameBufferSize.xy) * FrameBufferSize.zw;
}
float2 snap_uv_coord(float2 uv)
{
	return uv - (frac(uv * FrameBufferSize.xy) - 0.5f) * FrameBufferSize.zw;
}
float3 tangent_vector(float2 deltaUV, float3 dPdu, float3 dPdv)
{
	return deltaUV.x * dPdu + deltaUV.y * dPdv;
}
float invlength(float2 v)
{
	return rsqrt(dot(v, v));
}
float tangent(float3 T)
{
	return -T.z * invlength(T.xy);
}
float tangent(float3 P, float3 S)
{
	return (P.z - S.z) / length(S.xy - P.xy);
}
float tan_to_sin(float x)
{
	return x * rsqrt(x * x + 1.0f);
}
float falloff(float r)
{
	return 1.0f - AttenTanBias.x * r * r;
}
float AccumulatedHorizonOcclusion_Quality(float2 deltaUV,
                                          float2 uv0,
                                          float3 P,
                                          float numSteps,
                                          float randstep,
                                          float3 dPdu,
                                          float3 dPdv)
{
    // Jitter starting point within the first sample distance
	float2 uv = (uv0 + deltaUV) + randstep * deltaUV;
    
    // Snap first sample uv and initialize horizon tangent
	float2 snapped_duv = snap_uv_offset(uv - uv0);
	float3 T = tangent_vector(snapped_duv, dPdu, dPdv);
	float tanH = tangent(T) + AttenTanBias.y;

	float ao = 0;
	float h0 = 0;
	for (float j = 0; j < numSteps; ++j)
	{
		float2 snapped_uv = snap_uv_coord(uv);
		float3 S = fetch_eye_pos(snapped_uv);
		uv += deltaUV;

        // Ignore any samples outside the radius of influence
		float d2 = length2(S - P);
		if (d2 < Radius.y)
		{
			float tanS = tangent(P, S);

            [branch]
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
				ao += falloff(r) * (h - h0);
				h0 = h;

                // Update the current horizon angle
				tanH = tanS;
			}
		}
	}
	return ao;
}

float4 PS(PixelInput IN) : SV_Target
{
	float3 P = fetch_eye_pos(IN.texUV);
    
    // Project the radius of influence g_R from eye space to texture space.
    // The scaling by 0.5 is to go from [-1,1] to [0,1].
	float2 step_size = 0.5 * Radius.x * FocalLen.xy / P.z; //1/h = g_FocalLen / P.z

    // Early out if the projected radius is smaller than 1 pixel.
	float numSteps = min(NumStepDirContrast.x, min(step_size.x * FrameBufferSize.x, step_size.y * FrameBufferSize.y));
	if (numSteps < 1.0) return 1.0;
	step_size = step_size / (numSteps + 1);

    // Nearest neighbor pixels on the tangent plane
#if USE_GNORMAL
	float3 N = normalize(MIR_SAMPLE_TEX2D_LEVEL(_GBufferNormal, IN.texUV, 0).xyz * 2.0 - 1.0);
	N = mul((float3x3)View, N);
	
	float4 tangentPlane = float4(N, dot(P, N));
	float3 Pr = tangent_eye_pos(IN.texUV + float2(FrameBufferSize.z, 0), tangentPlane);
	float3 Pl = tangent_eye_pos(IN.texUV + float2(-FrameBufferSize.z, 0), tangentPlane);
	float3 Pt = tangent_eye_pos(IN.texUV + float2(0, FrameBufferSize.w), tangentPlane);
	float3 Pb = tangent_eye_pos(IN.texUV + float2(0, -FrameBufferSize.w), tangentPlane);
#else
	float3 Pr = fetch_eye_pos(IN.texUV + float2(g_InvResolution.x, 0));
	float3 Pl = fetch_eye_pos(IN.texUV + float2(-g_InvResolution.x, 0));
	float3 Pt = fetch_eye_pos(IN.texUV + float2(0, g_InvResolution.y));
	float3 Pb = fetch_eye_pos(IN.texUV + float2(0, -g_InvResolution.y));
	float3 N = normalize(cross(Pr - Pl, Pt - Pb));
	float4 tangentPlane = float4(N, dot(P, N));
#endif
    
	// Screen-aligned basis for the tangent plane
	float3 dPdu = min_diff(P, Pr, Pl);
	float3 dPdv = min_diff(P, Pt, Pb) * (FrameBufferSize.y * FrameBufferSize.z);

    //(cos(alpha),sin(alpha),jitter)
	//float3 rand = noise(int3((int) IN.pos.x & 63, (int) IN.pos.y & 63, 0)).xyz;
	float2 noise2 = rand2dTo2d(IN.texUV);
	float angle = 2.0 * MIR_PI / NumStepDirContrast.y * noise2.x;
	float3 rand = float3(cos(angle), sin(angle), noise2.y);
	
	float ao = 0;
	float alpha = 2.0 * MIR_PI / NumStepDirContrast.y;
	for (float d = 0; d < NumStepDirContrast.y; d++)
	{
		float angle = alpha * d;
		float2 dir = float2(cos(angle), sin(angle));
		float2 deltaUV = rotate_direction(dir, rand.xy) * step_size.xy;
		ao += AccumulatedHorizonOcclusion_Quality(deltaUV, IN.texUV, P, numSteps, rand.z, dPdu, dPdv);
	}

	//return float4(IN.texUV, 0.0, 1.0);
	//return float4(rand, 1.0);
	//return float4(n2, 0.0, 1.0);
	//float3 N = normalize(MIR_SAMPLE_TEX2D_LEVEL(_GBufferNormal, IN.texUV, 0).xyz * 2.0 - 1.0);
	//return float4(N, 1.0);
	//return float4(P, 1.0);
	//return float4(P.z / 10.0, 0.0, 0.0, 1.0);
	//return float4(dPdu, 1.0);
	return 1.0 - ao / NumStepDirContrast.y * NumStepDirContrast.z;
}