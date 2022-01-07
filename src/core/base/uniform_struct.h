#pragma once
#include "core/base/math.h"

namespace mir {

#define UNIFORM_ALIGN _declspec(align(16))

struct UNIFORM_ALIGN cbPerFrame
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	cbPerFrame() {
		World = View = Projection = Eigen::Matrix4f::Identity();
		ViewInv = ProjectionInv = Eigen::Matrix4f::Identity();
		LightView = LightProjection = Eigen::Matrix4f::Identity();
		glstate_lightmodel_ambient = Eigen::Vector4f(0.01, 0.01, 0.01, 0.01);
		_ShadowMapTexture_TexelSize = Eigen::Vector4f::Zero();
		CameraPosition = Eigen::Vector4f::Zero();
	}
public:
	Eigen::Matrix4f World;
	Eigen::Matrix4f View;
	Eigen::Matrix4f Projection;

	Eigen::Matrix4f ViewInv;
	Eigen::Matrix4f ProjectionInv;

	Eigen::Matrix4f LightView;
	Eigen::Matrix4f LightProjection;

	Eigen::Vector4f CameraPosition;
	Eigen::Vector4f glstate_lightmodel_ambient;
	Eigen::Vector4f _ShadowMapTexture_TexelSize;
};

struct UNIFORM_ALIGN cbPerLight
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	cbPerLight() {
		unity_LightPosition = Eigen::Vector4f::Zero();
		unity_LightColor = Eigen::Vector4f::Zero();
		unity_SpecColor = Eigen::Vector4f::Zero();
		unity_LightAtten = Eigen::Vector4f::Zero();
		unity_SpotDirection = Eigen::Vector4f::Zero();
		IsSpotLight = false;
	}
public:
	Eigen::Vector4f unity_LightPosition;//world space
	Eigen::Vector4f unity_LightColor;//w(gloss)
	Eigen::Vector4f unity_SpecColor;//w(shiness)
	Eigen::Vector4f unity_LightAtten;//x(cutoff), y(1/(1-cutoff)), z(atten^2)
	Eigen::Vector4f unity_SpotDirection;
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
		AlbedoFactor = 1;
		AmbientOcclusionFactor = 1;
		RoughnessFactor = 1;
		MetallicFactor = 1;
		EnableAlbedoMap = false;
		EnableNormalMap = false;
		EnableAmbientOcclusionMap = false;
		EnableRoughnessMap = false;
		EnableMetalnessMap = false;
		AmbientOcclusion_ChannelGRoughness_ChannelBMetalness = false;
		AlbedoMapSRGB = true;
		HasTangent = false;
	}
	float AlbedoFactor;
	float AmbientOcclusionFactor;
	float RoughnessFactor;
	float MetallicFactor;
	BOOL EnableAlbedoMap;
	BOOL EnableNormalMap;
	BOOL EnableAmbientOcclusionMap;
	BOOL EnableRoughnessMap;
	BOOL EnableMetalnessMap;
	BOOL AmbientOcclusion_ChannelGRoughness_ChannelBMetalness;
	BOOL AlbedoMapSRGB;
	BOOL HasTangent;
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