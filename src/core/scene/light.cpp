#include "core/scene/light.h"
#include "core/scene/camera.h"

namespace mir {

/********** DirectLight **********/
DirectLight::DirectLight()
{
	SetDirection(0, 0, 1);
	SetDiffuseColor(1, 1, 1, 1);
	SetSpecularColor(1, 1, 1, 1);
	SetSpecularPower(32);
}

void DirectLight::SetDirection(float x, float y, float z)
{
	mCbLight.LightPos = Eigen::Vector4f(x, y, z, 0);
}

void DirectLight::SetDiffuseColor(float r, float g, float b, float a)
{
	mCbLight.DiffuseColor = Eigen::Vector4f(r, g, b, a);
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
	view = math::MakeLookAtLH(mCbLight.LightPos.head<3>(), camera.mLookAtPos, Eigen::Vector3f(0, 1, 0));
	proj = math::MakeOrthographicOffCenterLH(0, camera.mSize.x(), 0, camera.mSize.y(), 0.01, camera.mZFar);
}

/********** PointLight **********/
PointLight::PointLight()
{
	SetPosition(0, 0, -10);
	SetAttenuation(1.0, 0.01, 0.0);
}

void PointLight::SetPosition(float x, float y, float z)
{
	mCbLight.LightPos = Eigen::Vector4f(x, y, z, 1);
}

void PointLight::SetAttenuation(float a, float b, float c)
{
	mCbLight.Attenuation = Eigen::Vector4f(a, b, c, 0);
}

/********** SpotLight **********/
SpotLight::SpotLight()
{
	SetDirection(0, 0, 1);
	SetAngle(3.14 * 30 / 180);
}

void SpotLight::SetDirection(float x, float y, float z)
{
	mCbLight.DirectionCutOff = Eigen::Vector4f(x, y, z, mCbLight.DirectionCutOff.w());
}

void SpotLight::SetCutOff(float cutoff)
{
	mCbLight.DirectionCutOff.w() = cutoff;
}

void SpotLight::SetAngle(float radian)
{
	SetCutOff(cos(radian));
}

}