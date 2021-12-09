#include "core/scene/scene_manager.h"
#include "core/scene/camera.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"

namespace mir {

SceneManager::SceneManager(ResourceManager& resMng)
	: mResMng(resMng)
	, mCamerasDirty(false)
{
}

void SceneManager::RemoveAllCameras()
{
	mCameras.clear();
}
CameraPtr SceneManager::AddOthogonalCamera(const Eigen::Vector3f& eyePos, const Eigen::Vector3f& near_far_fov, unsigned camMask)
{
	CameraPtr camera = Camera::CreateOthogonal(mResMng, eyePos, math::vec::Forward() * fabs(eyePos.z()), near_far_fov, camMask);
	mCameras.push_back(camera);
	mCamerasDirty = true;
	return camera;
}
CameraPtr SceneManager::AddPerspectiveCamera(const Eigen::Vector3f& eyePos, const Eigen::Vector3f& near_far_fov, unsigned camMask)
{
	CameraPtr camera = Camera::CreatePerspective(mResMng, eyePos, math::vec::Forward() * fabs(eyePos.z()), near_far_fov, camMask);
	mCameras.push_back(camera);
	mCamerasDirty = true;
	return camera;
}

void SceneManager::ResortCameras() const 
{
	if (mCamerasDirty) 
	{
		mCamerasDirty = false;

		struct CompCameraByDepth {
			bool operator()(const CameraPtr& l, const CameraPtr& r) const {
				return l->GetDepth() < r->GetDepth();
			}
		};
		std::stable_sort(mCameras.begin(), mCameras.end(), CompCameraByDepth());
	}
}
const std::vector<CameraPtr>& SceneManager::GetCameras() const 
{
	ResortCameras();
	return mCameras; 
}

void SceneManager::RemoveAllLights()
{
	mLightsByOrder.clear();
}
SpotLightPtr SceneManager::AddSpotLight(unsigned camMask)
{
	SpotLightPtr light = std::make_shared<SpotLight>();
	light->SetCameraMask(camMask);
	mLightsByOrder.push_back(light);
	return light;
}
PointLightPtr SceneManager::AddPointLight(unsigned camMask)
{
	PointLightPtr light = std::make_shared<PointLight>();
	light->SetCameraMask(camMask);
	mLightsByOrder.push_back(light);
	return light;
}
DirectLightPtr SceneManager::AddDirectLight(unsigned camMask)
{
	DirectLightPtr light = std::make_shared<DirectLight>();
	light->SetCameraMask(camMask);
	mLightsByOrder.push_back(light);
	return light;
}

void SceneManager::ResortLights() const 
{
	std::sort(mLightsByOrder.begin(), mLightsByOrder.end(), [](const ILightPtr& l, const ILightPtr& r)->bool {
		return l->GetType() < r->GetType();
	});
}
const std::vector<ILightPtr>& SceneManager::GetLights() const 
{ 
	ResortLights();
	return mLightsByOrder; 
}

}