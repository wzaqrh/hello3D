#include <boost/assert.hpp>
#include "core/scene/light.h"
#include "core/scene/camera.h"

namespace mir {
namespace scene {

/********** Light **********/
Light::Light()
{}

void Light::SetDiffuse(const Eigen::Vector3f& color)
{
	mCbLight.unity_LightColor.head<3>() = color;
}

void Light::SetSpecular(const Eigen::Vector3f& color, float shiness, float luminance)
{
	mCbLight.unity_SpecColor.head<3>() = color;
	mCbLight.unity_SpecColor.w() = shiness;
	mCbLight.unity_LightColor.w() = luminance;
}

/********** LightCamera **********/
void LightCamera::Update(const Eigen::AlignedBox3f& sceneAABB)
{
	const Eigen::Vector3f& forward = Direction;
	const Eigen::Vector3f& eye = Position;
	//Eigen::Vector3f eye = Eigen::Vector3f(3.57088f, 6.989f, -9.19698f);

	View = math::cam::MakeLookForwardLH(eye, forward, Eigen::Vector3f(0, 1, 0));

	Transform3fAffine t(View);
	Eigen::AlignedBox3f frustum = sceneAABB.transformed(t);

	float frustumWidth = (std::max(fabs(frustum.min().x()), fabs(frustum.max().x()))) * 2;
	float frustumHeight = (std::max(fabs(frustum.min().y()), fabs(frustum.max().y()))) * 2;
	float zNear = frustum.min().z();
	float zFar = __max(frustum.max().z(), 32);
	if (IsSpotLight) ShadowCasterProj = math::cam::MakePerspectiveLH(frustumWidth, frustumHeight, zNear, zFar);
	else ShadowCasterProj = math::cam::MakeOrthographicOffCenterLH(-frustumWidth * 0.5f, frustumWidth * 0.5f, -frustumHeight * 0.5f, frustumHeight * 0.5f, zNear, zFar);

#if 0
	Eigen::Vector4f pos[2] = {
		Eigen::Vector4f(0,0,0,1),
		Eigen::Vector4f(0,0.5,0,1)
	};
	for (size_t i = 0; i < 2; ++i) {
		Eigen::Vector4f view = Transform3Projective(mView) * pos[i];
		Eigen::Vector4f perspective1 = Transform3Projective(mShadowCasterProj) * view;
		Eigen::Vector4f perspective = Transform3Projective(mShadowCasterProj * mView) * pos[i];
	}
#endif

	ShadowRecvProj = Transform3Projective(ShadowCasterProj)
		.prescale(Eigen::Vector3f(1, -1, 1))
		.matrix();
	ShadowRecvProj = Transform3Projective(ShadowRecvProj)
		.prescale(Eigen::Vector3f(0.5, 0.5, 1))
		.pretranslate(Eigen::Vector3f(0.5, 0.5, 0))
		.matrix();
}

/********** DirectLight **********/
DirectLight::DirectLight()
{}

void DirectLight::SetDirection(const Eigen::Vector3f& dir)
{
	mCamera.Direction = dir;
}

void DirectLight::SetPosition(const Eigen::Vector3f& pos)
{
	mCamera.Position = pos;
}

void DirectLight::SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at)
{
	SetPosition(eye);
	SetDirection(at - eye);
}

void DirectLight::SetLightRadius(float radius)
{
	mLightRadius = radius;
}

void DirectLight::SetMinPCFRadius(float minPcfRadius)
{
	mMinPcfRadius = minPcfRadius;
}

void DirectLight::UpdateLightCamera(const Eigen::AlignedBox3f& sceneAABB)
{
	mCbLight.unity_LightPosition.head<3>() = mCamera.Direction.normalized();
	mCbLight.unity_LightPosition.w() = 0.0f;

	mCamera.Update(sceneAABB);
	mCbLight.LightRadiusUVNearFar = Eigen::Vector4f(mLightRadius / mCamera.FrustumWidth, mLightRadius / mCamera.FrustumHeight, mCamera.FrustumZNear, mCamera.FrustumZFar);
	mCbLight.LightDepthParam = Eigen::Vector4f(1 / mCamera.FrustumZNear, (mCamera.FrustumZNear - mCamera.FrustumZFar) / (mCamera.FrustumZFar * mCamera.FrustumZNear), mMinPcfRadius, 0);
}

/********** SpotLight **********/
SpotLight::SpotLight()
{
	mCamera.IsSpotLight = true;
	mCbLight.IsSpotLight = true;
	SetDirection(Eigen::Vector3f(0, 0, 1));
	SetAngle(30.0f / 180 * 3.14);
}

void SpotLight::SetCutOff(float cutoff)
{
	mCbLight.unity_LightAtten.x() = cutoff;
	mCbLight.unity_LightAtten.y() = 1.0f / (1.0f - cutoff);
}

void SpotLight::SetAngle(float radian)
{
	SetCutOff(cos(radian));
}

void SpotLight::UpdateLightCamera(const Eigen::AlignedBox3f& sceneAABB)
{
	mCbLight.unity_LightPosition.head<3>() = mCamera.Position;
	mCbLight.unity_LightPosition.w() = 1.0f;

	mCbLight.unity_SpotDirection.head<3>() = mCamera.Direction.normalized();

	mCamera.Update(sceneAABB);
	mCbLight.LightRadiusUVNearFar = Eigen::Vector4f(mLightRadius / mCamera.FrustumWidth, mLightRadius / mCamera.FrustumHeight, mCamera.FrustumZNear, mCamera.FrustumZFar);
	mCbLight.LightDepthParam = Eigen::Vector4f(1 / mCamera.FrustumZNear, (mCamera.FrustumZNear - mCamera.FrustumZFar) / (mCamera.FrustumZFar * mCamera.FrustumZNear), mMinPcfRadius, 0);
}

/********** PointLight **********/
PointLight::PointLight()
{
	SetPosition(Eigen::Vector3f(0, 0, -10));
	SetAttenuation(0);
}

void PointLight::SetPosition(const Eigen::Vector3f& pos)
{
	mCbLight.unity_LightPosition.head<3>() = pos;
	mCbLight.unity_LightPosition.w() = 1.0f;
}

void PointLight::SetAttenuation(float c)
{
	mCbLight.unity_LightAtten.z() = c;
}

void PointLight::UpdateLightCamera(const Eigen::AlignedBox3f& sceneAABB)
{

}

/********** LightFactory **********/
DirectLightPtr LightFactory::CreateDirectLight()
{
	DirectLightPtr light = CreateInstance<DirectLight>();
	light->SetCameraMask(mDefCameraMask);
	return light;
}

PointLightPtr LightFactory::CreatePointLight()
{
	PointLightPtr light = CreateInstance<PointLight>();
	light->SetCameraMask(mDefCameraMask);
	return light;
}

SpotLightPtr LightFactory::CreateSpotLight()
{
	SpotLightPtr light = CreateInstance<SpotLight>();
	light->SetCameraMask(mDefCameraMask);
	return light;
}

}
}