#pragma once
#include "TBaseTypes.h"

const int MAX_MATRICES = 256;
struct cbWeightedSkin
{
	XMMATRIX mModel;
	XMMATRIX Models[MAX_MATRICES];
	int hasNormal;
	int hasMetalness;
	int hasRoughness;
	int hasAO;
};

struct cbUnityMaterial
{
	XMFLOAT4 _SpecColor;
	XMFLOAT4 _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	int _SpecLightOff;
	cbUnityMaterial();
};

struct cbUnityGlobal
{
	XMFLOAT4 _Unity_IndirectSpecColor;
	XMFLOAT4 _AmbientOrLightmapUV;
	XMFLOAT4 _Unity_SpecCube0_HDR;
	cbUnityGlobal();
};