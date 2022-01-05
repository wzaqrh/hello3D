#ifndef MATH_H
#define MATH_H
#include "HLSLSupport.cginc"
#include "Debug.cginc"

inline float3 GetNormalFromMap(MIR_ARGS_TEX2D(normalMap), float2 texCoord, float3 worldPos, float3 worldNormal)
{
    float3 tangentNormal = MIR_SAMPLE_TEX2D(normalMap, texCoord).xyz * 2.0 - 1.0;

    float3 dpdx = ddx(worldPos);
    float3 dpdy = ddy(worldPos);
    float2 duvdx = ddx(texCoord);
    float2 duvdy = ddy(texCoord);

#if DEBUG_TBN == 1
    float3 N = normalize(cross(dpx, dpy));
#else
    float3 N = normalize(worldNormal);
#endif  
    
    float3 T = dpdx * duvdy.y - dpdy * duvdx.y;
    T = normalize(T - dot(T,N)*N);
    float3 B = normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);
    
    float3 normal = normalize(mul(tangentNormal, TBN));
#if DEBUG_CHANNEL == DEBUG_CHANNEL_NORMAL_TEXTURE
    normal = tangentNormal;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_NORMAL
    normal = N;
    normal.z = -normal.z;//compare gltf-sample-viewer
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_TANGENT
    normal = T;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_BITANGENT  
    normal = B;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_NORMAL_SHADING 
    normal.z = -normal.z;//compare gltf-sample-viewer
#endif    
    return normal;
}

#endif