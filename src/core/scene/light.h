#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/tpl/traits.h"
#include "core/base/math.h"
#include "core/base/uniform_struct.h"
#include "core/scene/component.h"

namespace mir {
enum LightType
{
	kLightDirectional,
	kLightPoint,
	kLightSpot
};
namespace scene {

class MIR_CORE_API Light : public Component
{
public:
	Light();
	virtual ~Light() {}
	virtual LightType GetType() const = 0;
	virtual void UpdateLightCamera(const Eigen::AlignedBox3f& sceneAABB) = 0;
	const cbPerLight& MakeCbLight() const { return mCbLight; }
	virtual Eigen::Matrix4f GetView() const = 0;
	virtual Eigen::Matrix4f GetCastShadowProj() const = 0;
	virtual Eigen::Matrix4f GetRecvShadowProj() const = 0;

	void SetCameraMask(unsigned mask) { mCameraMask = mask; }
	unsigned GetCameraMask() const { return mCameraMask; }

	void SetCastShadow(bool castShadow) { mCastShadow = castShadow; }
	bool DidCastShadow() const { return mCastShadow && GetType() != kLightPoint; }

	void SetDiffuse(const Eigen::Vector3f& color);
	void SetSpecular(const Eigen::Vector3f& color, float shiness, float luminance);
protected:
	cbPerLight mCbLight;
	unsigned mCameraMask = -1;
	bool mCastShadow = true;
};

class LightCamera 
{
public:
	void Update(const Eigen::AlignedBox3f& sceneAABB);
public:
	bool IsSpotLight = false;
	Eigen::Vector3f Position, Direction;
	float FrustumWidth, FrustumHeight, FrustumZNear, FrustumZFar;
	Eigen::Matrix4f View, ShadowCasterProj, ShadowRecvProj;
};

class MIR_CORE_API DirectLight : public Light 
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	DirectLight();
	LightType GetType() const override { return kLightDirectional; }

	void SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at);
	void SetPosition(const Eigen::Vector3f& pos);
	void SetDirection(const Eigen::Vector3f& dir);
	void SetLightRadius(float radius);
	void SetMinPCFRadius(float minPcfRadius);

	void UpdateLightCamera(const Eigen::AlignedBox3f& sceneAABB) override;
	Eigen::Matrix4f GetView() const override { return mCamera.View; }
	Eigen::Matrix4f GetCastShadowProj() const override { return mCamera.ShadowCasterProj; }
	Eigen::Matrix4f GetRecvShadowProj() const override { return mCamera.ShadowRecvProj; }
protected:
	float mLightRadius = 0.5f, mMinPcfRadius = 0.008f;
	LightCamera mCamera;
};

class MIR_CORE_API SpotLight : public DirectLight
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	SpotLight();
	LightType GetType() const override { return kLightSpot; }

	void SetCutOff(float cutoff);
	void SetAngle(float radian);

	void UpdateLightCamera(const Eigen::AlignedBox3f& sceneAABB) override;
};

class MIR_CORE_API PointLight : public Light
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	PointLight();
	LightType GetType() const override { return kLightPoint; }

	void SetPosition(const Eigen::Vector3f& pos);
	void SetAttenuation(float c);

	void UpdateLightCamera(const Eigen::AlignedBox3f& sceneAABB) override;
	Eigen::Matrix4f GetView() const override { return Eigen::Matrix4f(); }
	Eigen::Matrix4f GetCastShadowProj() const override { return Eigen::Matrix4f(); }
	Eigen::Matrix4f GetRecvShadowProj() const override { return Eigen::Matrix4f(); }
};

class MIR_CORE_API LightFactory : boost::noncopyable {
public:
	DirectLightPtr CreateDirectLight();
	PointLightPtr CreatePointLight();
	SpotLightPtr CreateSpotLight();

	template<typename LightClass, typename... T> std::shared_ptr<LightClass> CreateLight(T &&...args) {
		return CreateLightFunctor<LightClass>()(*this, std::forward<T>(args)...);
	}
private:
	template<typename LightClass> struct CreateLightFunctor {};
	template<> struct CreateLightFunctor<DirectLight> {
		TemplateArgs DirectLightPtr operator()(LightFactory& __this, T &&...args) const { return __this.CreateDirectLight(std::forward<T>(args)...); }
	};
	template<> struct CreateLightFunctor<PointLight> {
		TemplateArgs PointLightPtr operator()(LightFactory& __this, T &&...args) const { return __this.CreatePointLight(std::forward<T>(args)...); }
	};
	template<> struct CreateLightFunctor<SpotLight> {
		TemplateArgs SpotLightPtr operator()(LightFactory& __this, T &&...args) const { return __this.CreateSpotLight(std::forward<T>(args)...); }
	};
private:
	int mDefCameraMask = -1;
};
}
}