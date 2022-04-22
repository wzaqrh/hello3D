#ifndef MATH_H
#define MATH_H
#include "HLSLSupport.cginc"
#include "Debug.cginc"

//world normal coordinate system
inline float3x3 GetTBN(float3 normal, float3 tangent)
{
	float3 bitangent = cross(tangent, normal); //normal朝外, tangent朝右
    return float3x3(tangent, bitangent, normal);
}
inline float3x3 GetTBN(float2 uv, float3 worldPos, float3 worldNormal)
{
    float3 dpdx = ddx(worldPos);
    float3 dpdy = ddy(worldPos);
    float2 duvdx = ddx(uv);
    float2 duvdy = ddy(uv);

#if DEBUG_TBN == 1
    float3 N = normalize(cross(dpx, dpy));
#else
    float3 N = worldNormal;
#endif  
    
    float3 T = - dpdx * duvdy.y + dpdy * duvdx.y;
    T = normalize(T - dot(T,N)*N);
    float3 B = normalize(cross(T, N));
    
    return float3x3(T, B, N);
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

float3 uv_to_eye(float2 uv, float eye_z, float2 invFocalLen)
{
	uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));
	return float3(uv * invFocalLen * eye_z, eye_z);
}
#endif