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

	XMINT4 LightNum;//directional,point,spot
	TDirectLight DirectLights[MAX_LIGHTS];
	TPointLight PointLights[MAX_LIGHTS];
	TSpotLight SpotLights[MAX_LIGHTS];

	XMMATRIX LightView;
	XMMATRIX LightProjection;
	int HasDepthMap;
public:
	cbGlobalParam();
	static TConstBufferDecl& GetDesc();
	static TConstBufferDecl MKDesc();
};

struct TFogExp {
	XMFLOAT3 FogColor;
	float FogExp;
public:
	TFogExp();
	void SetColor(float r, float g, float b);
	void SetExp(float exp);
	static TConstBufferDecl& GetDesc();
};

const int MAX_MATRICES = 256;
__declspec(align(16)) struct cbWeightedSkin
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

__declspec(align(16)) struct cbUnityMaterial
{
	XMFLOAT4 _SpecColor;
	XMFLOAT4 _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	int _SpecLightOff;

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