#include "IRenderSystem.h"
#include "TMaterial.h"
#include "Utility.h"
#include "TSkyBox.h"
#include "TPostProcess.h"
#include "TThreadPump.h"
#include <d3dcompiler.h>

IRenderSystem::IRenderSystem()
{
}

IRenderSystem::~IRenderSystem()
{
}

void IRenderSystem::_PushRenderTarget(IRenderTexturePtr rendTarget)
{
	mRenderTargetStk.push_back(rendTarget);
	SetRenderTarget(rendTarget);
}

void IRenderSystem::_PopRenderTarget()
{
	if (!mRenderTargetStk.empty())
		mRenderTargetStk.pop_back();

	SetRenderTarget(!mRenderTargetStk.empty() ? mRenderTargetStk.back() : nullptr);
}

TSpotLightPtr IRenderSystem::AddSpotLight()
{
	TSpotLightPtr light = std::make_shared<TSpotLight>();
	mSpotLights.push_back(light);
	mLightsOrder.push_back(std::pair<TDirectLight*, enLightType>(light.get(), E_LIGHT_SPOT));
	return light;
}

TPointLightPtr IRenderSystem::AddPointLight()
{
	TPointLightPtr light = std::make_shared<TPointLight>();
	mPointLights.push_back(light);
	mLightsOrder.push_back(std::pair<TDirectLight*, enLightType>(light.get(), E_LIGHT_POINT));
	return light;
}

TDirectLightPtr IRenderSystem::AddDirectLight()
{
	TDirectLightPtr light = std::make_shared<TDirectLight>();
	mDirectLights.push_back(light);
	mLightsOrder.push_back(std::pair<TDirectLight*, enLightType>(light.get(), E_LIGHT_DIRECT));
	return light;
}

TCameraPtr IRenderSystem::SetOthogonalCamera(double far1)
{
	mDefCamera = TCamera::CreateOthogonal(mScreenWidth, mScreenHeight, far1);
	if (mSkyBox) mSkyBox->SetRefCamera(mDefCamera);
	return mDefCamera;
}

TCameraPtr IRenderSystem::SetPerspectiveCamera(double fov, int eyeDistance, double far1)
{
	mDefCamera = TCamera::CreatePerspective(mScreenWidth, mScreenHeight, fov, eyeDistance, far1);
	if (mSkyBox) mSkyBox->SetRefCamera(mDefCamera);
	return mDefCamera;
}

TSkyBoxPtr IRenderSystem::SetSkyBox(const std::string& imgName)
{
	mSkyBox = std::make_shared<TSkyBox>(this, mDefCamera, imgName);
	return mSkyBox;
}

TPostProcessPtr IRenderSystem::AddPostProcess(const std::string& name)
{
	TPostProcessPtr process;
	if (name == E_PASS_POSTPROCESS) {
		TBloom* bloom = new TBloom(this, mPostProcessRT);
		process = std::shared_ptr<TPostProcess>(bloom);
	}

	if (process) mPostProcs.push_back(process);
	return process;
}


TProgramPtr IRenderSystem::CreateProgram(const std::string& name, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	std::string ext = GetFileExt(name);
	if (ext.empty()) {
		return CreateProgramByFXC(name, vsEntry, psEntry);
	}
	else {
		return CreateProgramByCompile(name.c_str(), name.c_str(), vsEntry, psEntry);
	}
}

ITexturePtr IRenderSystem::GetTexByPath(const std::string& __imgPath, DXGI_FORMAT format /*= DXGI_FORMAT_UNKNOWN*/)
{
	const char* pSrc = __imgPath.c_str();
	std::string imgPath = __imgPath;
	auto pos = __imgPath.find_last_of("\\");
	if (pos != std::string::npos) {
		imgPath = __imgPath.substr(pos + 1, std::string::npos);
	}

	ITexturePtr texView = nullptr;
	if (mTexByPath.find(imgPath) == mTexByPath.end()) {
		texView = _CreateTexture(imgPath.c_str(), format, true);
		mTexByPath.insert(std::make_pair(imgPath, texView));
	}
	else {
		texView = mTexByPath[imgPath];
	}
	return texView;
}

void IRenderSystem::Draw(IRenderable* renderable)
{
	TRenderOperationQueue opQue;
	renderable->GenRenderOperation(opQue);
	RenderQueue(opQue, E_PASS_FORWARDBASE);
}

cbGlobalParam IRenderSystem::MakeAutoParam(TCameraBase* pLightCam, bool castShadow, TDirectLight* light, enLightType lightType)
{
	cbGlobalParam globalParam = {};
	//globalParam.mWorld = mWorldTransform;

	if (castShadow) {
		globalParam.mView = COPY_TO_GPU(pLightCam->mView);
		globalParam.mProjection = COPY_TO_GPU(pLightCam->mProjection);
	}
	else {
		globalParam.mView = COPY_TO_GPU(mDefCamera->mView);
		globalParam.mProjection = COPY_TO_GPU(mDefCamera->mProjection);

		globalParam.mLightView = COPY_TO_GPU(pLightCam->mView);
		globalParam.mLightProjection = COPY_TO_GPU(pLightCam->mProjection);
	}
	globalParam.HasDepthMap = mCastShdowFlag ? TRUE : FALSE;

	{
		XMVECTOR det = XMMatrixDeterminant(COPY_TO_GPU(globalParam.mWorld));
		globalParam.mWorldInv = COPY_TO_GPU(XMMatrixInverse(&det, globalParam.mWorld));

		det = XMMatrixDeterminant(COPY_TO_GPU(globalParam.mView));
		globalParam.mViewInv = COPY_TO_GPU(XMMatrixInverse(&det, globalParam.mView));

		det = XMMatrixDeterminant(COPY_TO_GPU(globalParam.mProjection));
		globalParam.mProjectionInv = COPY_TO_GPU(XMMatrixInverse(&det, globalParam.mProjection));
	}

	switch (lightType)
	{
	case E_LIGHT_DIRECT:
		globalParam.mLightNum.x = 1;
		globalParam.mDirectLights[0] = *light;
		break;
	case E_LIGHT_POINT:
		globalParam.mLightNum.y = 1;
		globalParam.mPointLights[0] = *(TPointLight*)light;
		break;
	case E_LIGHT_SPOT:
		globalParam.mLightNum.z = 1;
		globalParam.mSpotLights[0] = *(TSpotLight*)light;
		break;
	default:
		break;
	}
	return globalParam;
}