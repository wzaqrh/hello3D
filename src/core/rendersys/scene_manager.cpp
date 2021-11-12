#include "core/rendersys/scene_manager.h"
#include "core/base/transform.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/base/utility.h"

namespace mir {

SceneManager::SceneManager(RenderSystem& renderSys, MaterialFactory& matFac, XMINT2 screenSize, CameraPtr defCamera)
	:mRenderSys(renderSys)
	,mMaterialFac(matFac)
{
	mScreenWidth  = screenSize.x;
	mScreenHeight = screenSize.y;
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
	CameraPtr camera = Camera::CreateOthogonal(mRenderSys, mScreenWidth, mScreenHeight, eyePos, far1);
	mCameras.push_back(camera);
	return camera;
}
CameraPtr SceneManager::AddPerspectiveCamera(const Eigen::Vector3f& eyePos, double far1, double fov)
{
	CameraPtr camera = Camera::CreatePerspective(mRenderSys, mScreenWidth, mScreenHeight, eyePos, far1, fov);
	mCameras.push_back(camera);
	return camera;
}

cbSpotLightPtr SceneManager::AddSpotLight()
{
	cbSpotLightPtr light = std::make_shared<cbSpotLight>();
	mSpotLights.push_back(light);
	mLightsByOrder.push_back(std::pair<cbDirectLight*, LightType>(light.get(), kLightSpot));
	return light;
}
cbPointLightPtr SceneManager::AddPointLight()
{
	cbPointLightPtr light = std::make_shared<cbPointLight>();
	mPointLights.push_back(light);
	mLightsByOrder.push_back(std::pair<cbDirectLight*, LightType>(light.get(), kLightPoint));
	return light;
}
cbDirectLightPtr SceneManager::AddDirectLight()
{
	cbDirectLightPtr light = std::make_shared<cbDirectLight>();
	mDirectLights.push_back(light);
	mLightsByOrder.push_back(std::pair<cbDirectLight*, LightType>(light.get(), kLightDirectional));
	return light;
}

}