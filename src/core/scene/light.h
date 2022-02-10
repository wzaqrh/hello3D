#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/math.h"
#include "core/base/uniform_struct.h"
#include "core/scene/component.h"

namespace mir {
namespace scene {

#define interface struct

enum LightType 
{
	kLightDirectional,
	kLightPoint,
	kLightSpot
};

class MIR_CORE_API Light : public Component
{
public:
	virtual ~Light() {}
	virtual LightType GetType() const = 0;
	virtual void CalculateLightingViewProjection(const Camera& camera, Eigen::Vector2i size, bool castShadow, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const = 0;
	cbPerLight MakeCbLight() const { return mCbLight; }

	void SetCameraMask(unsigned mask) { mCameraMask = mask; }
	unsigned GetCameraMask() const { return mCameraMask; }
protected:
	cbPerLight mCbLight;
	unsigned mCameraMask = -1;
};

class MIR_CORE_API DirectLight : public Light 
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	DirectLight();
	void SetDirection(const Eigen::Vector3f& dir);
	void SetDiffuse(const Eigen::Vector3f& color);
	void SetSpecular(const Eigen::Vector3f& color, float shiness, float luminance);

	LightType GetType() const override { return kLightDirectional; }
	void CalculateLightingViewProjection(const Camera& camera, Eigen::Vector2i size, bool castShadow, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const override;
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
}