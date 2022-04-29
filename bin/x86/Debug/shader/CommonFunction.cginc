#ifndef COMMON_FUNCTION_H
#define COMMON_FUNCTION_H
#include "HLSLSupport.cginc"
#include "Macros.cginc"

//world normal coordinate system
inline float3x3 GetTBN(float3 normal, float3 tangent)
{
	float3 bitangent = cross(tangent, normal); //normal朝外, tangent朝右
    return float3x3(tangent, bitangent, normal);
}
inline float3x3 GetTBN(float2 t, float3 worldPos, float3 worldNormal)
{
    float3 dpx = ddx(worldPos);
    float3 dpy = ddy(worldPos);
    float2 dtx = ddx(t);
    float2 dty = ddy(t);

#if 0
    float3 N = normalize(cross(dpx, dpy));
#else
    float3 N = worldNormal;
#endif  
    
    float3 T = - dpx * dty.y + dpy * dtx.y;
    T = normalize(T - dot(T,N) * N);
    float3 B = normalize(cross(T, N));
    
    return float3x3(T, B, N);
}

inline float3x3 GetTBN(float2 uv, float3 worldPos)
{
	float3 dpdx = ddx(worldPos);
	float3 dpdy = ddy(worldPos);
	float2 duvdx = ddx(uv);
	float2 duvdy = ddy(uv);

	float3 N = normalize(cross(dpdy, dpdx));
    
	float3 T = -dpdx * duvdy.y + dpdy * duvdx.y;
	T = normalize(T - dot(T, N) * N);
	float3 B = normalize(cross(T, N));
    
	return float3x3(T, B, N);
}

float2 DepthGradient(float2 uv, float z)
{
	float2 dz_duv = 0;

	float3 duvdist_dx = ddx(float3(uv, z));
	float3 duvdist_dy = ddy(float3(uv, z));

	dz_duv.x = duvdist_dy.y * duvdist_dx.z;
	dz_duv.x -= duvdist_dx.y * duvdist_dy.z;
    
	dz_duv.y = duvdist_dx.x * duvdist_dy.z;
	dz_duv.y -= duvdist_dy.x * duvdist_dx.z;

	float det = (duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x);
	dz_duv /= det;

	return dz_duv;
}

float BiasedZ(float z0, float2 dz_duv, float2 offset)
{
	return z0 + dot(dz_duv, offset);
}

float length2(float3 v)
{
	return dot(v, v);
}
//返回1.0/v.length
float invlength(float2 v)
{
	return rsqrt(dot(v, v));
}

//pl-p与p-pr最小微分
float3 min_diff(float3 P, float3 Pr, float3 Pl)
{
	float3 V1 = Pr - P;
	float3 V2 = P - Pl;
	return (length2(V1) < length2(V2)) ? V1 : V2;
}
//m = [cosθ, -sinθ]
//    [sinθ,  cosθ]
float2 rotate_direction(float2 Dir, float2 CosSin)
{
	return float2(Dir.x * CosSin.x - Dir.y * CosSin.y,
                  Dir.x * CosSin.y + Dir.y * CosSin.x);
}
//投影到<dPdu,dPdv,dPdw>构成的坐标系
//m = [dPdu.x dPdu.y dPdu.z]
//    [dPdv.x dPdv.y dPdv.z]
//    [dPdw.x dPdw.y dPdw.z]
float3 tangent_vector(float2 deltaUV, float3 dPdu, float3 dPdv)
{
	return deltaUV.x * dPdu + deltaUV.y * dPdv;
}
//T投影到xy平面夹角θ的正弦值
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

inline float falloff(float r, float attenuation)
{
	return 1.0f - attenuation * r * r;
}

//uv范围[0,1] #使uv就近对齐整数像素
inline float2 snap_uv_offset(float2 uv, float4 resolution)
{
	return round(uv * resolution.xy) * resolution.zw;
}
//uv范围[0,1] #使uv就近对齐整数+0.5像素
inline float2 snap_uv_coord(float2 uv, float4 resolution)
{
	return uv - (frac(uv * resolution.xy) - 0.5f) * resolution.zw;
}

float3 uv_to_eye(float2 uv, float eye_z, float2 invFocalLen)
{
	uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));
	return float3(uv * invFocalLen * eye_z, eye_z);
}

inline float LinearEyeDepth(float d, float2 depthParam)
{
#if 0
#define FARZ 1000.0
#define NEARZ 0.3
	return FARZ * NEARZ / (FARZ - d * (FARZ - NEARZ));
#else
	return 1.0 / (depthParam.x + depthParam.y * d);
#endif
}
#define ZClipToZEye LinearEyeDepth

inline float fetch_eye_z(float2 uv, float2 depthParam, MIR_ARGS_TEX2D(tDepth))
{
	float d = MIR_SAMPLE_TEX2D(tDepth, uv);
	return LinearEyeDepth(d, depthParam);
}
//获取uv对应的, '相机空间'上的, 离相机最近的表面位置
inline float3 fetch_eye_pos(float2 uv, float4 depthParam, float4 focalLen, MIR_ARGS_TEX2D(tDepth))
{
	float d = MIR_SAMPLE_TEX2D_LEVEL(tDepth, uv, 0);
	return uv_to_eye(uv, LinearEyeDepth(d, depthParam.xy), focalLen.zw);
}
//获取fetch_eye_pos, 并确保其与'法线'同向
inline float3 tangent_eye_pos(float2 uv, float4 tangentPlane, float4 depthParam, float4 focalLen, MIR_ARGS_TEX2D(tDepth))
{
    //view vector going through the surface point at uv
	float3 V = fetch_eye_pos(uv, depthParam, focalLen, MIR_PASS_TEX2D(tDepth));
	//intersect with tangent plane except for silhouette edges
	float NdotV = dot(tangentPlane.xyz, V);
	if (NdotV < 0.0)
		V *= (tangentPlane.w / NdotV);
	return V;
}

#endif