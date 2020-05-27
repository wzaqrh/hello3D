#include "IRenderSystem.h"
#include "ISceneManager.h"
#include "TMaterialCB.h"
#include "TMaterial.h"
#include "TPostProcess.h"
#include "TSkyBox.h"
#include "Utility.h"

TRenderSystem::TRenderSystem()
{
	mDrawLimit = 4;
}

TRenderSystem::~TRenderSystem()
{
}

bool TRenderSystem::_CanDraw()
{
	//return mDrawCount++ < mDrawLimit;
	//return mDrawCount++ == mDrawLimit-1;
	return true;
}

void TRenderSystem::_PushRenderTarget(IRenderTexturePtr rendTarget)
{
	mRenderTargetStk.push_back(rendTarget);
	SetRenderTarget(rendTarget);
}

void TRenderSystem::_PopRenderTarget()
{
	if (!mRenderTargetStk.empty())
		mRenderTargetStk.pop_back();

	SetRenderTarget(!mRenderTargetStk.empty() ? mRenderTargetStk.back() : nullptr);
}

TSpotLightPtr TRenderSystem::AddSpotLight()
{
	TSpotLightPtr light = std::make_shared<TSpotLight>();
	mSpotLights.push_back(light);
	mLightsOrder.push_back(std::pair<TDirectLight*, enLightType>(light.get(), E_LIGHT_SPOT));
	return light;
}

TPointLightPtr TRenderSystem::AddPointLight()
{
	TPointLightPtr light = std::make_shared<TPointLight>();
	mPointLights.push_back(light);
	mLightsOrder.push_back(std::pair<TDirectLight*, enLightType>(light.get(), E_LIGHT_POINT));
	return light;
}

TDirectLightPtr TRenderSystem::AddDirectLight()
{
	TDirectLightPtr light = std::make_shared<TDirectLight>();
	mDirectLights.push_back(light);
	mLightsOrder.push_back(std::pair<TDirectLight*, enLightType>(light.get(), E_LIGHT_DIRECT));
	return light;
}

TCameraPtr TRenderSystem::SetOthogonalCamera(double far1)
{
	mDefCamera = TCamera::CreateOthogonal(mScreenWidth, mScreenHeight, far1);
	if (mSkyBox) mSkyBox->SetRefCamera(mDefCamera);
	return mDefCamera;
}

TCameraPtr TRenderSystem::SetPerspectiveCamera(double fov, int eyeDistance, double far1)
{
	mDefCamera = TCamera::CreatePerspective(mScreenWidth, mScreenHeight, fov, eyeDistance, far1);
	if (mSkyBox) mSkyBox->SetRefCamera(mDefCamera);
	return mDefCamera;
}

TSkyBoxPtr TRenderSystem::SetSkyBox(const std::string& imgName)
{
	mSkyBox = std::make_shared<TSkyBox>(this, mDefCamera, imgName);
	return mSkyBox;
}

TPostProcessPtr TRenderSystem::AddPostProcess(const std::string& name)
{
	TPostProcessPtr process;
	if (name == E_PASS_POSTPROCESS) {
		TBloom* bloom = new TBloom(this, mPostProcessRT);
		process = std::shared_ptr<TPostProcess>(bloom);
	}

	if (process) mPostProcs.push_back(process);
	return process;
}


IProgramPtr TRenderSystem::CreateProgram(const std::string& name, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	std::string ext = GetFileExt(name);
	if (ext.empty()) {
		std::string fullname = "shader\\" + mFXCDir + name;
		return CreateProgramByFXC(fullname, vsEntry, psEntry);
	}
	else {
		std::string fullname = "shader\\" + name;
		return CreateProgramByCompile(fullname.c_str(), fullname.c_str(), vsEntry, psEntry);
	}
}

ITexturePtr TRenderSystem::LoadTexture(const std::string& __imgPath, DXGI_FORMAT format /*= DXGI_FORMAT_UNKNOWN*/, bool async/* = true*/, bool isCube/* = false*/)
{
	const char* pSrc = __imgPath.c_str();
	std::string imgPath = __imgPath;
	auto pos = __imgPath.find_last_of("\\");
	if (pos != std::string::npos) {
		imgPath = __imgPath.substr(pos + 1, std::string::npos);
	}

	ITexturePtr texView = nullptr;
	if (mTexByPath.find(imgPath) == mTexByPath.end()) {
		texView = _CreateTexture(imgPath.c_str(), format, async, isCube);
		mTexByPath.insert(std::make_pair(imgPath, texView));
	}
	else {
		texView = mTexByPath[imgPath];
	}
	return texView;
}

void TRenderSystem::Draw(IRenderable* renderable)
{
	TRenderOperationQueue opQue;
	renderable->GenRenderOperation(opQue);
	RenderQueue(opQue, E_PASS_FORWARDBASE);
}

void TRenderSystem::MakeAutoParam(cbGlobalParam& globalParam, TCameraBase* pLightCam, bool castShadow, TDirectLight* light, enLightType lightType)
{
	memset(&globalParam, 0, sizeof(globalParam));

	if (castShadow) {
		globalParam.View = COPY_TO_GPU(pLightCam->GetView());
		globalParam.Projection = COPY_TO_GPU(pLightCam->GetProjection());
	}
	else {
		globalParam.View = COPY_TO_GPU(mDefCamera->GetView());
		globalParam.Projection = COPY_TO_GPU(mDefCamera->GetProjection());

		globalParam.LightView = COPY_TO_GPU(pLightCam->GetView());
		globalParam.LightProjection = COPY_TO_GPU(pLightCam->GetProjection());
	}
	globalParam.HasDepthMap = mCastShdowFlag ? TRUE : FALSE;

	globalParam.WorldInv = XM::Inverse(globalParam.World);
	globalParam.ViewInv = XM::Inverse(globalParam.View);
	globalParam.ProjectionInv = XM::Inverse(globalParam.Projection);

	globalParam.LightType = lightType + 1;
	switch (lightType)
	{
	case E_LIGHT_DIRECT:
		static_cast<TDirectLight&>(globalParam.Light) = *light;
		break;
	case E_LIGHT_POINT:
		static_cast<TPointLight&>(globalParam.Light) = *(TPointLight*)light;
		break;
	case E_LIGHT_SPOT:
		globalParam.Light = *(TSpotLight*)light;
		break;
	default:
		break;
	}
}