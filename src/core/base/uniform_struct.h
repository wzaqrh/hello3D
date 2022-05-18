#pragma once
#include "core/base/math.h"

namespace mir {

#define MAKE_CBNAME(V) #V
#define UNIFORM_ALIGN _declspec(align(16))

struct UNIFORM_ALIGN cbPerFrame
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	cbPerFrame() {
		World = View = Projection = Eigen::Matrix4f::Identity();
		ViewInv = ProjectionInv = Eigen::Matrix4f::Identity();
		LightView = LightProjection = Eigen::Matrix4f::Identity();
		SHC0C1 = Eigen::Matrix4f::Zero();
		FrameBufferSize = Eigen::Vector4f::Zero();
		ShadowMapSize = Eigen::Vector4f::Zero();
		CameraPosition = Eigen::Vector4f::Zero();
		glstate_lightmodel_ambient = Eigen::Vector4f(0.01, 0.01, 0.01, 0.01);
		Exposure = 1;
	}
public:
	Eigen::Matrix4f World;
	Eigen::Matrix4f View;
	Eigen::Matrix4f Projection;

	Eigen::Matrix4f ViewInv;
	Eigen::Matrix4f ProjectionInv;

	Eigen::Matrix4f LightView;
	Eigen::Matrix4f LightProjection;

	Eigen::Matrix4f SHC0C1;

	Eigen::Vector4f CameraPosition;
	Eigen::Vector4f FrameBufferSize;
	Eigen::Vector4f ShadowMapSize;
	Eigen::Vector4f glstate_lightmodel_ambient;
	float Exposure;
};

struct UNIFORM_ALIGN cbPerLight
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	cbPerLight() {
		unity_LightPosition = Eigen::Vector4f::Zero();
		unity_LightColor = Eigen::Vector4f::Ones();
		unity_SpecColor = Eigen::Vector4f::Ones();
		unity_LightAtten = Eigen::Vector4f::Zero();
		unity_SpotDirection = Eigen::Vector4f::Zero();
		LightRadiusUVNearFar = Eigen::Vector4f(0.0001f, 0.0001f, 0.3f, 1000.0f);
		LightDepthParam = Eigen::Vector4f(3.33333f, 3.33233f, 0.0f, 0.0f);
		IsSpotLight = false;
	}
public:
	Eigen::Vector4f unity_LightPosition;//world space
	Eigen::Vector4f unity_LightColor;//w(gloss)
	Eigen::Vector4f unity_SpecColor;//w(shiness)
	Eigen::Vector4f unity_LightAtten;//x(cutoff), y(1/(1-cutoff)), z(atten^2)
	Eigen::Vector4f unity_SpotDirection;
	Eigen::Vector4f LightRadiusUVNearFar;
	Eigen::Vector4f LightDepthParam;
	BOOL IsSpotLight;
};

struct UNIFORM_ALIGN cbWeightedSkin
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	enum { kModelCount = 56 };
	cbWeightedSkin() {
		Models[0] = Model = Eigen::Matrix4f::Identity();
	}
public:
	Eigen::Matrix4f Model;
	Eigen::Matrix4f Models[kModelCount];
};

struct UNIFORM_ALIGN cbModel
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	cbModel() {
		AlbedoFactor = Eigen::Vector4f::Ones();
		MetallicFactor = RoughnessFactor = OcclusionStrength = NormalScale = 1;
		EmissiveFactor = Eigen::Vector3f::Zero();
		
		EmissiveUV = MetallicUV = RoughnessUV = OcclusionUV = NormalUV = AlbedoUV = Eigen::Vector4f::Zero();
	}
	Eigen::Vector4f AlbedoUV;
	Eigen::Vector4f NormalUV;
	Eigen::Vector4f OcclusionUV;
	Eigen::Vector4f RoughnessUV;
	Eigen::Vector4f MetallicUV;
	Eigen::Vector4f EmissiveUV;

	Eigen::Vector4f AlbedoFactor;
	float NormalScale;
	float OcclusionStrength;
	float RoughnessFactor;
	float MetallicFactor;
	Eigen::Vector3f EmissiveFactor;
};

struct UNIFORM_ALIGN cbUnityMaterial
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	cbUnityMaterial() {
		_SpecColor = Eigen::Vector4f::Ones();
		_Color = Eigen::Vector4f::Ones();
		_GlossMapScale = 1;
		_OcclusionStrength = 1;
		_SpecLightOff = 0;
	}
public:
	Eigen::Vector4f _SpecColor;
	Eigen::Vector4f _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	unsigned _SpecLightOff;
};

struct UNIFORM_ALIGN cbUnityGlobal
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	cbUnityGlobal() {
		_Unity_IndirectSpecColor = Eigen::Vector4f::Zero();
		_AmbientOrLightmapUV = Eigen::Vector4f(0.01, 0.01, 0.01, 1);
		_Unity_SpecCube0_HDR = Eigen::Vector4f(0.5, 1, 0, 0);
	}
public:
	Eigen::Vector4f _Unity_IndirectSpecColor;
	Eigen::Vector4f _AmbientOrLightmapUV;
	Eigen::Vector4f _Unity_SpecCube0_HDR;
};

}