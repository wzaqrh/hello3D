#ifndef SPHERICAL_HARMONICS_H
#define SPHERICAL_HARMONICS_H
#include "Macros.cginc"

inline float3 LinearToGammaSpace (float3 linRGB)
{
    linRGB = max(linRGB, float3(0,0,0));
    // An almost-perfect approximation from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return max(1.055f * pow(linRGB, 0.416666667f) - 0.055f, 0.0f);

    // Exact version, useful for debugging.
    //return half3(LinearToGammaSpaceExact(linRGB.r), LinearToGammaSpaceExact(linRGB.g), LinearToGammaSpaceExact(linRGB.b));
}

//c0c1 = |c0:0.x c1:-1.x c1:0.x c1:1.x|
//		 |c0:0.y c1:-1.y c1:0.y c1:1.y|
//		 |c0:0.z c1:-1.z c1:0.z c1:1.z|
//		 |	   0	   0	  0		 0|
float3 GetSphericalHarmonicsDgree01(float4 normal, matrix c0c1) {
	float3 color = mul(normal, c0c1).xyz;
#if COLORSPACE_GAMMA
    color = LinearToGammaSpace(color);
#endif
    return color;
}

#endif