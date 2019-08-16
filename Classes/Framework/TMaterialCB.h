#pragma once
#include "TBaseTypes.h"

#define MAX_LIGHTS 4
__declspec(align(16)) struct cbGlobalParam
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;

	XMMATRIX WorldInv;
	XMMATRIX ViewInv;
	XMMATRIX ProjectionInv;

	TSpotLight Light;
	unsigned int LightType;//directional=1,point=2,spot=3
	
	unsigned int HasDepthMap;
	XMMATRIX LightView;
	XMMATRIX LightProjection;
public:
	cbGlobalParam();
	static TConstBufferDecl& GetDesc();
	static TConstBufferDecl MKDesc();
};

struct TFogExp {
	XMFLOAT4 FogColorExp;
public:
	TFogExp();
	void SetColor(float r, float g, float b);
	void SetExp(float exp);
	static TConstBufferDecl& GetDesc();
};

const int MAX_MATRICES = 56;
__declspec(align(16)) struct cbWeightedSkin
{
	XMMATRIX Model;
	XMMATRIX Models[MAX_MATRICES];
	unsigned int hasNormal;
	unsigned int hasMetalness;
	unsigned int hasRoughness;
	unsigned int hasAO;

	static TConstBufferDecl& GetDesc();
	static TConstBufferDecl MKDesc();
};

__declspec(align(16)) struct cbUnityMaterial
{
	XMFLOAT4 _SpecColor;
	XMFLOAT4 _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	unsigned int _SpecLightOff;

	cbUnityMaterial();
	static TConstBufferDecl& GetDesc();
	static TConstBufferDecl MKDesc();
};

__declspec(align(16)) struct cbUnityGlobal
{
	XMFLOAT4 _Unity_IndirectSpecColor;
	XMFLOAT4 _AmbientOrLightmapUV;
	XMFLOAT4 _Unity_SpecCube0_HDR;

	cbUnityGlobal();
	static TConstBufferDecl& GetDesc();
	static TConstBufferDecl MKDesc();
};

struct TConstBufferDeclBuilder {
	TConstBufferDecl mDecl;
public:
	TConstBufferDecl& Build();
	TConstBufferDeclBuilder& Add(const TConstBufferDeclElement& elem);
};
#define MAKE_CBDESC(CB) (CB::GetDesc())