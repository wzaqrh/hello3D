#pragma once
#include "core/base/math.h"

namespace mir {

struct _declspec(align(16)) cbPerFrame
{
	cbPerFrame() {
		World = View = Projection = Eigen::Matrix4f::Identity();
		WorldInv = ViewInv = ProjectionInv = Eigen::Matrix4f::Identity();
		LightView = LightProjection = Eigen::Matrix4f::Identity();
		glstate_lightmodel_ambient = Eigen::Vector4f(0.01, 0.01, 0.01, 0.01);
		_ShadowMapTexture_TexelSize = Eigen::Vector4f::Zero();
	}
public:
	Eigen::Matrix4f World;
	Eigen::Matrix4f View;
	Eigen::Matrix4f Projection;

	Eigen::Matrix4f WorldInv;
	Eigen::Matrix4f ViewInv;
	Eigen::Matrix4f ProjectionInv;

	Eigen::Matrix4f LightView;
	Eigen::Matrix4f LightProjection;

	Eigen::Vector4f glstate_lightmodel_ambient;
	Eigen::Vector4f _ShadowMapTexture_TexelSize;
};

struct _declspec(align(16)) cbPerLight
{
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

struct _declspec(align(16)) cbWeightedSkin
{
	enum { kModelCount = 56 };
	cbWeightedSkin() {
		Models[0] = Model = Eigen::Matrix4f::Identity();
		hasNormal = false;
		hasMetalness = false;
		hasRoughness = false;
		hasAO = false;
		hasAlbedo = false;
	}
public:
	Eigen::Matrix4f Model;
	Eigen::Matrix4f Models[kModelCount];
	BOOL hasNormal;
	BOOL hasMetalness;
	BOOL hasRoughness;
	BOOL hasAO;
	BOOL hasAlbedo;
};

struct _declspec(align(16)) cbUnityMaterial
{
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

struct _declspec(align(16)) cbUnityGlobal
{
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