#pragma once
#include "core/rendersys/material_pred.h"

namespace mir {

struct TCameraBase;
struct TCamera;
struct TConstBufferDecl;
struct TDirectLight {
public:
	XMFLOAT4 LightPos;//world space
	XMFLOAT4 DiffuseColor;
	XMFLOAT4 SpecularColorPower;
public:
	TDirectLight();
	void SetDirection(float x, float y, float z);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);
public:
	TCameraBase* GetLightCamera(TCamera& otherCam);
	static TConstBufferDecl& GetDesc();
};
struct TPointLight : public TDirectLight {
public:
	XMFLOAT4 Attenuation;
public:
	TPointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float a, float b, float c);
public:
	static TConstBufferDecl& GetDesc();
};
struct TSpotLight : public TPointLight {
public:
	XMFLOAT4 DirectionCutOff;
public:
	TSpotLight();
	void SetDirection(float x, float y, float z);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);
public:
	static TConstBufferDecl& GetDesc();
};


#define MAX_LIGHTS 4
__declspec(align(16)) 
struct cbGlobalParam
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;

	XMMATRIX WorldInv;
	XMMATRIX ViewInv;
	XMMATRIX ProjectionInv;

	XMMATRIX LightView;
	XMMATRIX LightProjection;
	TSpotLight Light;
	
	unsigned int LightType;//directional=1,point=2,spot=3
	unsigned int HasDepthMap;
public:
	cbGlobalParam();
	static TConstBufferDecl& GetDesc();
	static TConstBufferDecl MKDesc();
};

struct TConstBufferDecl;
struct TFogExp 
{
	XMFLOAT4 FogColorExp;
public:
	TFogExp();
	void SetColor(float r, float g, float b);
	void SetExp(float exp);
	static TConstBufferDecl& GetDesc();
};


const int MAX_MATRICES = 56;
__declspec(align(16)) 
struct cbWeightedSkin
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


__declspec(align(16)) 
struct cbUnityMaterial
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


__declspec(align(16)) 
struct cbUnityGlobal
{
	XMFLOAT4 _Unity_IndirectSpecColor;
	XMFLOAT4 _AmbientOrLightmapUV;
	XMFLOAT4 _Unity_SpecCube0_HDR;

	cbUnityGlobal();
	static TConstBufferDecl& GetDesc();
	static TConstBufferDecl MKDesc();
};

}