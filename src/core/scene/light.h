#pragma once
#include <boost/noncopyable.hpp>
#include "core/rendersys/predeclare.h"
#include "core/resource/material_cb.h"

namespace mir {

#define interface struct

interface MIR_CORE_API ILight : boost::noncopyable 
{
	virtual ~ILight() {}
	virtual LightType GetType() const = 0;
	virtual cbSpotLight MakeCbLight() const = 0;
	virtual void CalculateLightingViewProjection(const Camera& camera, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const = 0;
};

class MIR_CORE_API DirectLight : public ILight 
{
public:
	DirectLight();
	void SetDirection(float x, float y, float z);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);

	void CalculateLightingViewProjection(const Camera& camera, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const override;
	LightType GetType() const override { return kLightDirectional; }
	cbSpotLight MakeCbLight() const override { return mCbLight; }
protected:
	cbSpotLight mCbLight;
};

class MIR_CORE_API PointLight : public DirectLight
{
public:
	PointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float a, float b, float c);

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