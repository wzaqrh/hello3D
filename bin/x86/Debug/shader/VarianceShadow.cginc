#ifndef VARIANCE_SHADOW_H
#define VARIANCE_SHADOW_H
#include "CommonFunction.cginc"
#include "Debug.cginc"

inline float VSMShadow(float2 tex, float fragDepth, MIR_ARGS_TEX2D(tVSM))
{
	float2 moments = MIR_SAMPLE_TEX2D(tVSM, tex).xy;   
	float E_x2 = moments.y;
	float Ex_2 = moments.x * moments.x;
	float variance = E_x2 - Ex_2;
	
	float mD = (moments.x - fragDepth);
	float mD_2 = mD * mD;
	
	float p = variance / (variance + mD_2);
	//float lit = fragDepth <= moments.x;
	//float lit = min(p, 1.0);
	//float lit = p;
	float lit = max(p, fragDepth <= moments.x);
	return lit;
}

#endif