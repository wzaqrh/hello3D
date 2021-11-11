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
	Eigen::Vector4f LightPos;//world space
	Eigen::Vector4f DiffuseColor;
	Eigen::Vector4f SpecularColorPower;
public:
	cbDirectLight();
	void SetDirection(float x, float y, float z);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);
public:
	void CalculateLightingViewProjection(const Camera& camera, XMMATRIX& view, XMMATRIX& proj);
#if !defined MATERIAL_FROM_XML
	static ConstBufferDecl& GetDesc();
#endif
};
struct cbPointLight : public cbDirectLight {
public:
	Eigen::Vector4f Attenuation;
public:
	cbPointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float a, float b, float c);
public:
#if !defined MATERIAL_FROM_XML
	static ConstBufferDecl& GetDesc();
#endif
};
struct cbSpotLight : public cbPointLight {
public:
	Eigen::Vector4f DirectionCutOff;
public:
	cbSpotLight();
	void SetDirection(float x, float y, float z);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);
public:
#if !defined MATERIAL_FROM_XML
	static ConstBufferDecl& GetDesc();
#endif
};

struct __declspec(align(16)) cbGlobalParam
{
	Eigen::Matrix4f World;
	Eigen::Matrix4f View;
	Eigen::Matrix4f Projection;

	Eigen::Matrix4f WorldInv;
	Eigen::Matrix4f ViewInv;
	Eigen::Matrix4f ProjectionInv;

	Eigen::Matrix4f LightView;
	Eigen::Matrix4f LightProjection;
	cbSpotLight Light;
	
	unsigned int LightType;//directional=1,point=2,spot=3
	unsigned int HasDepthMap;
public:
	cbGlobalParam();
#if !defined MATERIAL_FROM_XML
	static ConstBufferDecl& GetDesc();
	static ConstBufferDecl MKDesc();
#endif
};

struct __declspec(align(16)) cbFogExp 
{
	Eigen::Vector4f FogColorExp;
public:
	cbFogExp();
	void SetColor(float r, float g, float b);
	void SetExp(float exp);
#if !defined MATERIAL_FROM_XML
	static ConstBufferDecl& GetDesc();
#endif
};

constexpr int MAX_MATRICES = 56;
struct __declspec(align(16)) cbWeightedSkin
{
	Eigen::Matrix4f Model;
	Eigen::Matrix4f Models[MAX_MATRICES];
	unsigned int hasNormal;
	unsigned int hasMetalness;
	unsigned int hasRoughness;
	unsigned int hasAO;

#if !defined MATERIAL_FROM_XML
	static ConstBufferDecl& GetDesc();
	static ConstBufferDecl MKDesc();
#endif
};

struct __declspec(align(16))  cbUnityMaterial
{
	Eigen::Vector4f _SpecColor;
	Eigen::Vector4f _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	unsigned int _SpecLightOff;

	cbUnityMaterial();
#if !defined MATERIAL_FROM_XML
	static ConstBufferDecl& GetDesc();
	static ConstBufferDecl MKDesc();
#endif
};

struct __declspec(align(16)) cbUnityGlobal
{
	Eigen::Vector4f _Unity_IndirectSpecColor;
	Eigen::Vector4f _AmbientOrLightmapUV;
	Eigen::Vector4f _Unity_SpecCube0_HDR;

	cbUnityGlobal();
#if !defined MATERIAL_FROM_XML
	static ConstBufferDecl& GetDesc();
	static ConstBufferDecl MKDesc();
#endif
};

struct cbBloom {
	Eigen::Vector4f SampleOffsets[16];
	Eigen::Vector4f SampleWeights[16];
public:
	static cbBloom CreateDownScale2x2Offsets(int dwWidth, int dwHeight);
	static cbBloom CreateDownScale3x3Offsets(int dwWidth, int dwHeight);
	static cbBloom CreateBloomOffsets(int dwD3DTexSize, float fDeviation, float fMultiplier);
#if !defined MATERIAL_FROM_XML
	static ConstBufferDecl& GetDesc();
#endif
};

}