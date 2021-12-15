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
	, mLightsDirty(false)
{
}

void SceneManager::RemoveAllCameras()
{
	mCameras.clear();
}
CameraPtr SceneManager::AddPerspectiveCamera(const Eigen::Vector3f& eyePos, unsigned camMask)
{
	CameraPtr camera = Camera::CreatePerspective(mResMng, eyePos, math::vec::Forward() * fabs(eyePos.z()),
		math::cam::DefClippingPlane(), math::cam::DefFov(), camMask);
	mCameras.push_back(camera);
	mCamerasDirty = true;
	return camera;
}
CameraPtr SceneManager::AddOthogonalCamera(const Eigen::Vector3f& eyePos, unsigned camMask)
{
	CameraPtr camera = Camera::CreateOthogonal(mResMng, eyePos, math::vec::Forward() * fabs(eyePos.z()), 
		math::cam::DefClippingPlane(), math::cam::DefOthoSize() * mPixelPerUnit, camMask);
	mCameras.push_back(camera);
	mCamerasDirty = true;
	return camera;
}

void SceneManager::ResortCameras() const 
{
	if (mCamerasDirty) {
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
	mLights.clear();
}
SpotLightPtr SceneManager::AddSpotLight(unsigned camMask)
{
	SpotLightPtr light = std::make_shared<SpotLight>();
	light->SetCameraMask(camMask);
	mLights.push_back(light);
	mLightsDirty = true;
	return light;
}
PointLightPtr SceneManager::AddPointLight(unsigned camMask)
{
	PointLightPtr light = std::make_shared<PointLight>();
	light->SetCameraMask(camMask);
	mLights.push_back(light);
	mLightsDirty = true;
	return light;
}
DirectLightPtr SceneManager::AddDirectLight(unsigned camMask)
{
	DirectLightPtr light = std::make_shared<DirectLight>();
	light->SetCameraMask(camMask);
	mLights.push_back(light);
	mLightsDirty = true;
	return light;
}

void SceneManager::ResortLights() const 
{
	if (mLightsDirty) {
		mLightsDirty = false;

		std::sort(mLights.begin(), mLights.end(), [](const ILightPtr& l, const ILightPtr& r)->bool {
			return l->GetType() < r->GetType();
		});
	}
}
const std::vector<ILightPtr>& SceneManager::GetLights() const 
{ 
	ResortLights();
	return mLights; 
}

}