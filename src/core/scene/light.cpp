#include "core/scene/light.h"
#include "core/scene/camera.h"

namespace mir {

/********** DirectLight **********/
DirectLight::DirectLight()
{
	memset(&mCbLight, 0, sizeof(mCbLight));
	mCbLight.unity_LightPosition = Eigen::Vector4f(0, 0, 1, 0);
	SetDiffuseColor(1, 1, 1, 1);
	SetSpecularColor(1, 1, 1, 1);
	SetSpecularPower(32);
}

void DirectLight::SetDirection(float x, float y, float z)
{
	mCbLight.unity_LightPosition = -Eigen::Vector4f(x, y, z, 0).normalized();
}

void DirectLight::SetDiffuseColor(float r, float g, float b, float a)
{
	mCbLight.unity_LightColor = Eigen::Vector4f(r, g, b, a);
}

void DirectLight::SetSpecularColor(float r, float g, float b, float a)
{
	mCbLight.SpecularColorPower = Eigen::Vector4f(r, g, b, mCbLight.SpecularColorPower.w());
}

void DirectLight::SetSpecularPower(float power)
{
	mCbLight.SpecularColorPower.w() = power;
}

void DirectLight::CalculateLightingViewProjection(const Camera& camera, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) const {
	view = math::MakeLookAtLH(mCbLight.unity_LightPosition.head<3>(), camera.mLookAtPos, Eigen::Vector3f(0, 1, 0));
	proj = math::MakeOrthographicOffCenterLH(0, camera.mSize.x(), 0, camera.mSize.y(), 0.01, camera.mZFar);
}

/********** PointLight **********/
PointLight::PointLight()
{
	mCbLight.unity_LightPosition = Eigen::Vector4f(0, 0, -10, 1);
	mCbLight.unity_LightAtten = Eigen::Vector4f(0, 0, 0, 0);
}

void PointLight::SetPosition(float x, float y, float z)
{
	mCbLight.unity_LightPosition = Eigen::Vector4f(x, y, z, 1);
}

void PointLight::SetAttenuation(float c)
{
	mCbLight.unity_LightAtten.z() = c;
}

/********** SpotLight **********/
SpotLight::SpotLight()
{
	SetSpotDirection(0, 0, 1);
	SetAngle(3.14 * 30 / 180);
}

void SpotLight::SetDirection(float x, float y, float z)
{
	mCbLight.unity_LightPosition = -Eigen::Vector4f(x, y, z, 0);
	mCbLight.unity_SpotDirection.leftCols<3>() = mCbLight.unity_LightPosition.leftCols<3>();
}

void SpotLight::SetSpotDirection(float x, float y, float z)
{
	mCbLight.unity_SpotDirection = Eigen::Vector4f(x, y, z, mCbLight.unity_SpotDirection.w());
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