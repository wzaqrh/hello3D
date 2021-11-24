#include "core/resource/material_cb.h"
#include "core/rendersys/base_type.h"
#include "core/scene/camera.h"
#include "core/base/math.h"

namespace mir {

/********** cbDirectLight **********/
cbDirectLight::cbDirectLight()
{
	SetDirection(0, 0, 1);
	SetDiffuseColor(1, 1, 1, 1);
	SetSpecularColor(1, 1, 1, 1);
	SetSpecularPower(32);
}

void cbDirectLight::SetDirection(float x, float y, float z)
{
	LightPos = Eigen::Vector4f(x, y, z, 0);
}

void cbDirectLight::SetDiffuseColor(float r, float g, float b, float a)
{
	DiffuseColor = Eigen::Vector4f(r, g, b, a);
}

void cbDirectLight::SetSpecularColor(float r, float g, float b, float a)
{
	SpecularColorPower = Eigen::Vector4f(r, g, b, SpecularColorPower.w());
}

void cbDirectLight::SetSpecularPower(float power)
{
	SpecularColorPower.w() = power;
}

void cbDirectLight::CalculateLightingViewProjection(const Camera& camera, Eigen::Matrix4f& view, Eigen::Matrix4f& proj) {
	view = math::MakeLookAtLH(LightPos.head<3>(), camera.mLookAtPos, Eigen::Vector3f(0, 1, 0));
	proj = math::MakeOrthographicOffCenterLH(0, camera.mSize.x(), 0, camera.mSize.y(), 0.01, camera.mZFar);
}

/********** cbPointLight **********/
cbPointLight::cbPointLight()
{
	SetPosition(0, 0, -10);
	SetAttenuation(1.0, 0.01, 0.0);
}

void cbPointLight::SetPosition(float x, float y, float z)
{
	LightPos = Eigen::Vector4f(x, y, z, 1);
}

void cbPointLight::SetAttenuation(float a, float b, float c)
{
	Attenuation = Eigen::Vector4f(a, b, c, 0);
}

/********** cbSpotLight **********/
cbSpotLight::cbSpotLight()
{
	SetDirection(0, 0, 1);
	SetAngle(3.14 * 30 / 180);
}

void cbSpotLight::SetDirection(float x, float y, float z)
{
	DirectionCutOff = Eigen::Vector4f(x, y, z, DirectionCutOff.w());
}

void cbSpotLight::SetCutOff(float cutoff)
{
	DirectionCutOff.w() = cutoff;
}

void cbSpotLight::SetAngle(float radian)
{
	SetCutOff(cos(radian));
}

}