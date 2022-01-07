#ifndef MATH_H
#define MATH_H
#include "HLSLSupport.cginc"
#include "Debug.cginc"

struct TBNStruct
{
    float3 T, B, N;
};
inline TBNStruct GetTBN(float3 worldNormal, float3 tangent, float3 bitangent)
{
    TBNStruct TBN;
    TBN.N = worldNormal;
    TBN.T = tangent;
    TBN.B = bitangent;
    return TBN;
}
inline TBNStruct GetTBN(float2 texCoord, float3 worldPos, float3 worldNormal)
{
    float3 dpdx = ddx(worldPos);
    float3 dpdy = ddy(worldPos);
    float2 duvdx = ddx(texCoord);
    float2 duvdy = ddy(texCoord);

#if DEBUG_TBN == 1
    float3 N = normalize(cross(dpx, dpy));
#else
    float3 N = worldNormal;
#endif  
    
    float3 T =  - dpdx * duvdy.y + dpdy * duvdx.y;
    T = normalize(T - dot(T,N)*N);
    float3 B = normalize(cross(T, N));
    
    TBNStruct TBN;
    TBN.T = T;
    TBN.T = B;
    TBN.T = N;
    return TBN;
}

inline float3 GetNormalFromMap(MIR_ARGS_TEX2D(normalMap), float2 texCoord, TBNStruct tbn)
{
    float3 tangentNormal = MIR_SAMPLE_TEX2D(normalMap, texCoord).xyz * 2.0 - 1.0;

    float3x3 TBN = float3x3(tbn.T, tbn.B, tbn.N);
    float3 normal = normalize(mul(tangentNormal, TBN));
#if DEBUG_CHANNEL == DEBUG_CHANNEL_NORMAL_TEXTURE
    normal = tangentNormal;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_NORMAL
    normal = tbn.N;
    normal.z = -normal.z;//compare gltf-sample-viewer
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_TANGENT
    normal = tbn.T;
	normal.z = -normal.z;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_GEOMETRY_BITANGENT  
    normal = tbn.B;
	normal.z = -normal.z;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_NORMAL_SHADING 
    normal.z = -normal.z;//compare gltf-sample-viewer
#endif    
	//normal = dpdx * 10;
	//normal = dpdy * 10;
    //normal = float3(duvdx,0) * 10;
	//normal = float3(duvdy,0) * 10;
    return normal;
}

#endif