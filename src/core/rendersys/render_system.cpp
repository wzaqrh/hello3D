#include "core/rendersys/render_system.h"
#include "core/rendersys/scene_manager.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/post_process.h"
#include "core/renderable/skybox.h"
#include "core/base/utility.h"

namespace mir {

RenderSystem::RenderSystem()
{
	mDrawLimit = 4;
}

RenderSystem::~RenderSystem()
{
}

bool RenderSystem::_CanDraw()
{
	//return mDrawCount++ < mDrawLimit;
	//return mDrawCount++ == mDrawLimit-1;
	return true;
}

void RenderSystem::_PushRenderTarget(IRenderTexturePtr rendTarget)
{
	mRenderTargetStk.push_back(rendTarget);
	SetRenderTarget(rendTarget);
}

void RenderSystem::_PopRenderTarget()
{
	if (!mRenderTargetStk.empty())
		mRenderTargetStk.pop_back();

	SetRenderTarget(!mRenderTargetStk.empty() ? mRenderTargetStk.back() : nullptr);
}

IProgramPtr RenderSystem::CreateProgram(const std::string& name, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
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

ITexturePtr RenderSystem::LoadTexture(const std::string& __imgPath, DXGI_FORMAT format /*= DXGI_FORMAT_UNKNOWN*/, bool async/* = true*/, bool isCube/* = false*/)
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

void RenderSystem::Draw(IRenderable* renderable)
{
	RenderOperationQueue opQue;
	renderable->GenRenderOperation(opQue);
	RenderQueue(opQue, E_PASS_FORWARDBASE);
}

void RenderSystem::MakeAutoParam(cbGlobalParam& globalParam, CameraBase* pLightCam, bool castShadow, cbDirectLight* light, enLightType lightType)
{
	memset(&globalParam, 0, sizeof(globalParam));

	if (castShadow) {
		globalParam.View = COPY_TO_GPU(pLightCam->GetView());
		globalParam.Projection = COPY_TO_GPU(pLightCam->GetProjection());
	}
	else {
		globalParam.View = COPY_TO_GPU(mSceneManager->mDefCamera->GetView());
		globalParam.Projection = COPY_TO_GPU(mSceneManager->mDefCamera->GetProjection());

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
		static_cast<cbDirectLight&>(globalParam.Light) = *light;
		break;
	case E_LIGHT_POINT:
		static_cast<cbPointLight&>(globalParam.Light) = *(cbPointLight*)light;
		break;
	case E_LIGHT_SPOT:
		globalParam.Light = *(cbSpotLight*)light;
		break;
	default:
		break;
	}
}

ISceneManagerPtr RenderSystem::GetSceneManager()
{
	return mSceneManager;
}

}