#ifndef MATH_H
#define MATH_H
#include "HLSLSupport.cginc"

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

    return normalize(mul(tangentNormal, TBN));
}

#endif