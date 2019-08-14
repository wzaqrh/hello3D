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

	static TConstBufferDecl& GetDesc();
	static TConstBufferDecl MKDesc();
};

struct cbUnityMaterial
{
	XMFLOAT4 _SpecColor;
	XMFLOAT4 _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	int _SpecLightOff;

	cbUnityMaterial();
	static TConstBufferDecl& GetDesc();
};

struct cbUnityGlobal
{
	XMFLOAT4 _Unity_IndirectSpecColor;
	XMFLOAT4 _AmbientOrLightmapUV;
	XMFLOAT4 _Unity_SpecCube0_HDR;

	cbUnityGlobal();
	static TConstBufferDecl& GetDesc();
};

struct TConstBufferDeclBuilder {
	TConstBufferDecl mDecl;
public:
	TConstBufferDecl Build();
	TConstBufferDeclBuilder& Add(const TConstBufferDeclElement& elem);
};
#define MAKE_CBDESC(CB) (CB::GetDesc())