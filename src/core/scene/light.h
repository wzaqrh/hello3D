#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/math.h"
#include "core/base/uniform_struct.h"

namespace mir {

#define interface struct

enum LightType 
{
	kLightDirectional,
	kLightPoint,
	kLightSpot
};

interface MIR_CORE_API ILight : boost::noncopyable 
{
	virtual ~ILight() {}
	virtual LightType GetType() const = 0;
	virtual cbPerLight MakeCbLight() const = 0;
	virtual void CalculateLightingViewProjection(const Camera& camera, Eigen::Vector2i size, bool castShadow, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const = 0;
	
	virtual void SetCameraMask(unsigned mask) = 0;
	virtual unsigned GetCameraMask() const = 0;
};

class MIR_CORE_API DirectLight : public ILight 
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	DirectLight();
	void SetDirection(const Eigen::Vector3f& dir);
	void SetDiffuse(const Eigen::Vector3f& color);
	void SetSpecular(const Eigen::Vector3f& color, float shiness, float luminance);

	void SetCameraMask(unsigned mask) override final { mCameraMask = mask; }
	unsigned GetCameraMask() const override final { return mCameraMask; }

	void CalculateLightingViewProjection(const Camera& camera, Eigen::Vector2i size, bool castShadow, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const override;
	LightType GetType() const override { return kLightDirectional; }
	cbPerLight MakeCbLight() const override { return mCbLight; }
protected:
	cbPerLight mCbLight;
	unsigned mCameraMask = -1;
};

class MIR_CORE_API PointLight : public DirectLight
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	PointLight();
	void SetPosition(const Eigen::Vector3f& pos);
	void SetAttenuation(float c);

	LightType GetType() const override { return kLightPoint; }
};

class MIR_CORE_API SpotLight : public PointLight
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	SpotLight();
	void SetSpotDirection(const Eigen::Vector3f& dir);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);

	LightType GetType() const override { return kLightSpot; }
};

}