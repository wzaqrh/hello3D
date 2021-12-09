#include "core/renderable/skybox.h"
#include "core/resource/resource_manager.h"

namespace mir {

SkyBox::SkyBox(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName, const std::string& imgName)
	:Super(launchMode, resourceMng, matName)
{
	SkyboxVertex Vertexs[4];
	float fHighW = -1.0f - (1.0f / (float)mResourceMng.WinSize().x());
	float fHighH = -1.0f - (1.0f / (float)mResourceMng.WinSize().y());
	float fLowW = 1.0f + (1.0f / (float)mResourceMng.WinSize().x());
	float fLowH = 1.0f + (1.0f / (float)mResourceMng.WinSize().y());
	Vertexs[0].pos = Eigen::Vector4f(fLowW, fLowH, 1.0f, 1.0f);
	Vertexs[1].pos = Eigen::Vector4f(fLowW, fHighH, 1.0f, 1.0f);
	Vertexs[2].pos = Eigen::Vector4f(fHighW, fLowH, 1.0f, 1.0f);
	Vertexs[3].pos = Eigen::Vector4f(fHighW, fHighH, 1.0f, 1.0f);
	mVertexBuffer = mResourceMng.CreateVertexBuffer(launchMode, sizeof(SkyboxVertex), 0, Data::Make(Vertexs));

	mTexture = mResourceMng.CreateTextureByFile(launchMode, imgName);
#if 0
	auto pCam1 = mContext->GetSceneMng()->GetDefCamera();
	Eigen::Vector3f pos0 = pCam->ProjectPoint(Eigen::Vector3f(fLowW, fLowH, 1.0f));
	Eigen::Vector3f pos1 = pCam->ProjectPoint(Eigen::Vector3f(fLowW, fHighH, 1.0f));
	Eigen::Vector3f pos2 = pCam->ProjectPoint(Eigen::Vector3f(fHighW, fLowH, 1.0f));
	Eigen::Vector3f pos3 = pCam->ProjectPoint(Eigen::Vector3f(fHighW, fHighH, 1.0f));
#endif
}

void SkyBox::GenRenderOperation(RenderOperationQueue& opList)
{
	RenderOperation op = {};
	if (!MakeRenderOperation(op)) return;

	opList.AddOP(op);
}

}