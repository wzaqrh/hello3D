#include "core/scene/scene_manager.h"
#include "core/scene/camera.h"
#include "core/base/transform.h"
#include "core/resource/material_factory.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"

namespace mir {

SceneManager::SceneManager(RenderSystem& renderSys, MaterialFactory& matFac, const Eigen::Vector2i& screenSize, CameraPtr defCamera)
	:mRenderSys(renderSys)
	,mMaterialFac(matFac)
	,mScreenSize(screenSize)
{
	if (defCamera) mCameras.push_back(defCamera);
}

void SceneManager::RemoveAllCameras()
{
	mCameras.clear();
}
CameraPtr SceneManager::GetDefCamera() const 
{
	return (! mCameras.empty()) ? mCameras[0] : nullptr;
}
CameraPtr SceneManager::AddOthogonalCamera(const Eigen::Vector3f& eyePos, double far1)
{
	CameraPtr camera = Camera::CreateOthogonal(mRenderSys, mScreenSize, eyePos, far1);
	mCameras.push_back(camera);
	return camera;
}
CameraPtr SceneManager::AddPerspectiveCamera(const Eigen::Vector3f& eyePos, double far1, double fov)
{
	CameraPtr camera = Camera::CreatePerspective(mRenderSys, mScreenSize, eyePos, far1, fov);
	mCameras.push_back(camera);
	return camera;
}

SpotLightPtr SceneManager::AddSpotLight()
{
	SpotLightPtr light = std::make_shared<SpotLight>();
	mSpotLights.push_back(light);
	mLightsByOrder.push_back(light);
	return light;
}
PointLightPtr SceneManager::AddPointLight()
{
	PointLightPtr light = std::make_shared<PointLight>();
	mPointLights.push_back(light);
	mLightsByOrder.push_back(light);
	return light;
}
DirectLightPtr SceneManager::AddDirectLight()
{
	DirectLightPtr light = std::make_shared<DirectLight>();
	mDirectLights.push_back(light);
	mLightsByOrder.push_back(light);
	return light;
}

}