#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/math.h"

namespace mir {

#define interface struct

enum LightType {
	kLightDirectional,
	kLightPoint,
	kLightSpot
};

struct _declspec(align(16)) cbPerLight {
	Eigen::Matrix4f LightView;
	Eigen::Matrix4f LightProjection;

	Eigen::Vector4f unity_LightPosition;//world space
	Eigen::Vector4f unity_LightColor;//w(gloss)
	Eigen::Vector4f unity_SpecColor;//w(shiness)
	Eigen::Vector4f unity_LightAtten;//x(cutoff), y(1/(1-cutoff)), z(atten^2)
	Eigen::Vector4f unity_SpotDirection;

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
	void SetDirection(const Eigen::Vector3f& dir);
	void SetDiffuse(const Eigen::Vector3f& color);
	void SetSpecular(const Eigen::Vector3f& color, float shiness, float luminance);

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
	void SetPosition(const Eigen::Vector3f& pos);
	void SetAttenuation(float c);

	LightType GetType() const override { return kLightPoint; }
};

class MIR_CORE_API SpotLight : public PointLight
{
public:
	SpotLight();
	void SetSpotDirection(const Eigen::Vector3f& dir);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);

	LightType GetType() const override { return kLightSpot; }
};

}