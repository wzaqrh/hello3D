#pragma once
#include "core/base/math.h"

namespace mir {

#define MAKE_CBNAME(V) #V
#define UNIFORM_ALIGN _declspec(align(16))

struct UNIFORM_ALIGN cbPerFrame
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
public:
	Eigen::Matrix4f World = Eigen::Matrix4f::Identity();
	Eigen::Matrix4f View = Eigen::Matrix4f::Identity();
	Eigen::Matrix4f Projection = Eigen::Matrix4f::Identity();
	Eigen::Matrix4f ViewInv = Eigen::Matrix4f::Identity();
	Eigen::Matrix4f ProjectionInv = Eigen::Matrix4f::Identity();

	Eigen::Matrix4f LightView = Eigen::Matrix4f::Identity();
	Eigen::Matrix4f LightProjection = Eigen::Matrix4f::Identity();

	Eigen::Vector4f CameraPositionExposure = Eigen::Vector4f(0, 0, 0, 1);

	Eigen::Matrix4f SHC0C1 = Eigen::Matrix4f::Zero();
	Eigen::Matrix4f SHC2 = Eigen::Matrix4f::Zero();
	Eigen::Vector4f SHC2_2 = Eigen::Vector4f::Zero();
	Eigen::Vector4f EnvDiffuseColor = Eigen::Vector4f::Zero();
	Eigen::Vector4f EnvSpecColorMip = Eigen::Vector4f(0, 0, 0, 1);
	Eigen::Vector4f LightMapUV = Eigen::Vector4f(0, 0, 1, 1);
	Eigen::Vector4f LightMapSizeMip = Eigen::Vector4f::Zero();

	Eigen::Vector4f FrameBufferSize = Eigen::Vector4f::Zero();
	Eigen::Vector4f ShadowMapSize = Eigen::Vector4f::Zero();
};

struct UNIFORM_ALIGN cbPerLight
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	cbPerLight() {
		LightPosition = Eigen::Vector4f::Zero();
		LightColor = Eigen::Vector4f::Ones();
		unity_LightAtten = Eigen::Vector4f::Zero();
		unity_SpotDirection = Eigen::Vector4f::Zero();
		LightRadiusUVNearFar = Eigen::Vector4f(0.0001f, 0.0001f, 0.3f, 1000.0f);
		LightDepthParam = Eigen::Vector4f(3.33333f, 3.33233f, 0.0f, 0.0f);
		IsSpotLight = false;
	}
public:
	Eigen::Vector4f LightPosition;//world space
	Eigen::Vector4f LightColor;
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

}