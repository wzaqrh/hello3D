#pragma once
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/base/math.h"

namespace mir {

enum LightType {
	kLightDirectional,
	kLightPoint,
	kLightSpot
};

struct cbDirectLight 
{
	Eigen::Vector4f LightPos;//world space
	Eigen::Vector4f DiffuseColor;
	Eigen::Vector4f SpecularColorPower;
};

struct cbPointLight : public cbDirectLight 
{
	Eigen::Vector4f Attenuation;
};

struct cbSpotLight : public cbPointLight 
{
	Eigen::Vector4f DirectionCutOff;
};

struct cbPerLight {
	Eigen::Matrix4f LightView;
	Eigen::Matrix4f LightProjection;
	cbSpotLight Light;

	unsigned int LightType;//directional=1,point=2,spot=3
	unsigned int HasDepthMap;
};

}