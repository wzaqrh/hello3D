#include "core/scene/light.h"
#include "core/scene/camera.h"

namespace mir {

/********** DirectLight **********/
DirectLight::DirectLight()
{
	mCbLight = cbPerLight{};
	mCbLight.unity_LightPosition = Eigen::Vector4f(0, 0, 1, 0);
	mCbLight.unity_LightColor = Eigen::Vector4f(1, 1, 1, 1/*gloss*/);
	mCbLight.unity_SpecColor = Eigen::Vector4f(1, 1, 1, 1/*shiness*/);
	mCbLight.unity_LightAtten = Eigen::Vector4f(0, 0, 0, 0);
	mCbLight.unity_SpotDirection = Eigen::Vector4f(0, 0, 0, 0);
}

void DirectLight::SetDirection(const Eigen::Vector3f& dir)
{
	mCbLight.unity_LightPosition = -Eigen::Vector4f(dir.x(), dir.y(), dir.z(), 0).normalized();
}

void DirectLight::SetDiffuse(const Eigen::Vector3f& color)
{
	mCbLight.unity_LightColor.head<3>() = color;
}

void DirectLight::SetSpecular(const Eigen::Vector3f& color, float shiness, float luminance)
{
	mCbLight.unity_SpecColor.head<3>() = color;
	mCbLight.unity_SpecColor.w() = shiness;
	mCbLight.unity_LightColor.w() = luminance;
}

void DirectLight::CalculateLightingViewProjection(const Camera& camera, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const {
	view = math::MakeLookAtLH(mCbLight.unity_LightPosition.head<3>(), camera.mLookAtPos, Eigen::Vector3f(0, 1, 0));
	proj = math::MakeOrthographicOffCenterLH(0, camera.mSize.x(), 0, camera.mSize.y(), 0.01, camera.mZFar);
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
}

void PointLight::SetAttenuation(float c)
{
	mCbLight.unity_LightAtten.z() = c;
}

/********** SpotLight **********/
SpotLight::SpotLight()
{
	SetSpotDirection(Eigen::Vector3f(0, 0, 1));
	SetAngle(30.0f / 180 * 3.14);
}

void SpotLight::SetSpotDirection(const Eigen::Vector3f& dir)
{
	mCbLight.unity_SpotDirection.head<3>() = dir;
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

}