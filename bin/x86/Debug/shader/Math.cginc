#ifndef MATH_H
#define MATH_H
#include "HLSLSupport.cginc"
#include "Debug.cginc"

//world normal coordinate system
inline float3x3 GetTBN(float3 normal, float3 tangent, float3 bitangent)
{
	bitangent = cross(tangent, normal);//normal≥ØÕ‚, tangent≥Ø”“
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

inline float3 GetNormalFromMap(float3 tangentNormal, float3x3 TBN)
{
    float3 normal = normalize(mul(tangentNormal, TBN));
#if DEBUG_CHANNEL == DEBUG_CHANNEL_NORMAL_TEXTURE
    normal = tangentNormal;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_NORMAL
    normal = TBN[2];
    normal.z = -normal.z;//compare gltf-sample-viewer
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_TANGENT
    normal = TBN[0];
	normal.z = -normal.z;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_BITANGENT 
    normal = TBN[1];
	normal.z = -normal.z;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHADING_NORMAL 
    normal.z = -normal.z;//compare gltf-sample-viewer
#endif
    return normal;
}

#endif