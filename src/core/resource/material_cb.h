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
struct MIR_CORE_API cbDirectLight {
public:
	cbDirectLight();
	void SetDirection(float x, float y, float z);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);
	void CalculateLightingViewProjection(const Camera& camera, Eigen::Matrix4f& view, Eigen::Matrix4f& proj);
public:
	Eigen::Vector4f LightPos;//world space
	Eigen::Vector4f DiffuseColor;
	Eigen::Vector4f SpecularColorPower;
};
struct MIR_CORE_API cbPointLight : public cbDirectLight {
public:
	cbPointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float a, float b, float c);
public:
	Eigen::Vector4f Attenuation;
};
struct MIR_CORE_API cbSpotLight : public cbPointLight {
public:
	cbSpotLight();
	void SetDirection(float x, float y, float z);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);
public:
	Eigen::Vector4f DirectionCutOff;
};

}