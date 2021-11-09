#include "core/rendersys/scene_manager.h"
#include "core/base/transform.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/base/utility.h"

namespace mir {

SceneManager::SceneManager(IRenderSystem& renderSys, MaterialFactory& matFac, XMINT2 screenSize, IRenderTexturePtr postRT, CameraPtr defCamera)
	:mRenderSys(renderSys)
	,mMaterialFac(matFac)
{
	mScreenWidth  = screenSize.x;
	mScreenHeight = screenSize.y;
	mPostProcessRT = postRT;
	mDefCamera = defCamera;
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

CameraPtr SceneManager::SetOthogonalCamera(double far1)
{
	mDefCamera = Camera::CreateOthogonal(mScreenWidth, mScreenHeight, far1);
	if (mSkyBox) mSkyBox->SetRefCamera(mDefCamera);
	return mDefCamera;
}

CameraPtr SceneManager::SetPerspectiveCamera(double fov, int eyeDistance, double far1)
{
	mDefCamera = Camera::CreatePerspective(mScreenWidth, mScreenHeight, fov, eyeDistance, far1);
	if (mSkyBox) mSkyBox->SetRefCamera(mDefCamera);
	return mDefCamera;
}

SkyBoxPtr SceneManager::SetSkyBox(const std::string& imgName)
{
	mSkyBox = std::make_shared<SkyBox>(mRenderSys, mMaterialFac, mDefCamera, imgName);
	return mSkyBox;
}

PostProcessPtr SceneManager::AddPostProcess(const std::string& name)
{
	PostProcessPtr process;
	if (name == E_PASS_POSTPROCESS) {
		process = std::make_shared<Bloom>(mRenderSys, mMaterialFac, mPostProcessRT);
	}

	if (process) mPostProcs.push_back(process);
	return process;
}

}