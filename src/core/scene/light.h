#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/base/math.h"

namespace mir {

#define interface struct

enum LightType {
	kLightDirectional,
	kLightPoint,
	kLightSpot
};

struct cbPerLight {
	Eigen::Matrix4f LightView;
	Eigen::Matrix4f LightProjection;

	Eigen::Vector4f LightPos;//world space
	Eigen::Vector4f DiffuseColor;
	Eigen::Vector4f SpecularColorPower;
	Eigen::Vector4f Attenuation;
	Eigen::Vector4f DirectionCutOff;

	unsigned int LightType;//directional=1,point=2,spot=3
	unsigned int HasDepthMap;
};

interface MIR_CORE_API ILight : boost::noncopyable 
{
	virtual ~ILight() {}
	virtual LightType GetType() const = 0;
	virtual cbPerLight MakeCbLight() const = 0;
	virtual void CalculateLightingViewProjection(const Camera& camera, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const = 0;
};

class MIR_CORE_API DirectLight : public ILight 
{
public:
	DirectLight();
	virtual void SetDirection(float x, float y, float z);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);

	void CalculateLightingViewProjection(const Camera& camera, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const override;
	LightType GetType() const override { return kLightDirectional; }
	cbPerLight MakeCbLight() const override { return mCbLight; }
protected:
	cbPerLight mCbLight;
};

class MIR_CORE_API PointLight : public DirectLight
{
public:
	PointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float c);

	LightType GetType() const override { return kLightPoint; }
};

class MIR_CORE_API SpotLight : public PointLight
{
public:
	SpotLight();
	void SetDirection(float x, float y, float z);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);

	LightType GetType() const override { return kLightSpot; }
};

}