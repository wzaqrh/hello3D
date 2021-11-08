#pragma once
#include "core/rendersys/predeclare.h"

namespace mir {

enum LightType {
	kLightDirectional,
	kLightPoint,
	kLightSpot
};
struct cbDirectLight {
public:
	XMFLOAT4 LightPos;//world space
	XMFLOAT4 DiffuseColor;
	XMFLOAT4 SpecularColorPower;
public:
	cbDirectLight();
	void SetDirection(float x, float y, float z);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);
public:
	CameraBase* GetLightCamera(Camera& otherCam);
	static ConstBufferDecl& GetDesc();
};
struct cbPointLight : public cbDirectLight {
public:
	XMFLOAT4 Attenuation;
public:
	cbPointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float a, float b, float c);
public:
	static ConstBufferDecl& GetDesc();
};
struct cbSpotLight : public cbPointLight {
public:
	XMFLOAT4 DirectionCutOff;
public:
	cbSpotLight();
	void SetDirection(float x, float y, float z);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);
public:
	static ConstBufferDecl& GetDesc();
};

struct __declspec(align(16)) cbGlobalParam
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;

	XMMATRIX WorldInv;
	XMMATRIX ViewInv;
	XMMATRIX ProjectionInv;

	XMMATRIX LightView;
	XMMATRIX LightProjection;
	cbSpotLight Light;
	
	unsigned int LightType;//directional=1,point=2,spot=3
	unsigned int HasDepthMap;
public:
	cbGlobalParam();
	static ConstBufferDecl& GetDesc();
	static ConstBufferDecl MKDesc();
};

struct __declspec(align(16)) cbFogExp 
{
	XMFLOAT4 FogColorExp;
public:
	cbFogExp();
	void SetColor(float r, float g, float b);
	void SetExp(float exp);
	static ConstBufferDecl& GetDesc();
};

constexpr int MAX_MATRICES = 56;
struct __declspec(align(16)) cbWeightedSkin
{
	XMMATRIX Model;
	XMMATRIX Models[MAX_MATRICES];
	unsigned int hasNormal;
	unsigned int hasMetalness;
	unsigned int hasRoughness;
	unsigned int hasAO;

	static ConstBufferDecl& GetDesc();
	static ConstBufferDecl MKDesc();
};

struct __declspec(align(16))  cbUnityMaterial
{
	XMFLOAT4 _SpecColor;
	XMFLOAT4 _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	unsigned int _SpecLightOff;

	cbUnityMaterial();
	static ConstBufferDecl& GetDesc();
	static ConstBufferDecl MKDesc();
};

struct __declspec(align(16)) cbUnityGlobal
{
	XMFLOAT4 _Unity_IndirectSpecColor;
	XMFLOAT4 _AmbientOrLightmapUV;
	XMFLOAT4 _Unity_SpecCube0_HDR;

	cbUnityGlobal();
	static ConstBufferDecl& GetDesc();
	static ConstBufferDecl MKDesc();
};

struct cbBloom {
	XMFLOAT4 SampleOffsets[16];
	XMFLOAT4 SampleWeights[16];
public:
	static cbBloom CreateDownScale2x2Offsets(int dwWidth, int dwHeight);
	static cbBloom CreateDownScale3x3Offsets(int dwWidth, int dwHeight);
	static cbBloom CreateBloomOffsets(int dwD3DTexSize, float fDeviation, float fMultiplier);
	static ConstBufferDecl& GetDesc();
};

}