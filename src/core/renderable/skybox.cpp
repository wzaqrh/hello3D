#include "core/renderable/skybox.h"
#include "core/resource/resource_manager.h"

namespace mir {

SkyBox::SkyBox(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName, const std::string& imgName)
	:Super(launchMode, resourceMng, matName)
{
	if (matName.VariantName == "Skybox/Deprecate") {
		SkyboxVertex Vertexs[4];
		float fHighW = -1.0f - (1.0f / (float)mResourceMng.WinSize().x());
		float fHighH = -1.0f - (1.0f / (float)mResourceMng.WinSize().y());
		float fLowW = 1.0f + (1.0f / (float)mResourceMng.WinSize().x());
		float fLowH = 1.0f + (1.0f / (float)mResourceMng.WinSize().y());
		Vertexs[0].Pos = Eigen::Vector3f(fLowW, fLowH, 1.0f);
		Vertexs[1].Pos = Eigen::Vector3f(fLowW, fHighH, 1.0f);
		Vertexs[2].Pos = Eigen::Vector3f(fHighW, fLowH, 1.0f);
		Vertexs[3].Pos = Eigen::Vector3f(fHighW, fHighH, 1.0f);
		mVertexBuffer = mResourceMng.CreateVertexBuffer(launchMode, sizeof(SkyboxVertex), 0, Data::Make(Vertexs));
	}
	else {
		SkyboxVertex Vertexs[36];
		float negY = -1, top = 1;
		float negX = -1, right = 1;
		float negZ = -1, far1 = 1;
		Vertexs[0].Pos = Eigen::Vector3f(negX, 1.0f, negZ);
		Vertexs[1].Pos = Eigen::Vector3f(negX, negY, negZ);
		Vertexs[2].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[3].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[4].Pos = Eigen::Vector3f(1.0f, 1.0f, negZ);
		Vertexs[5].Pos = Eigen::Vector3f(negX, 1.0f, negZ);

		Vertexs[6].Pos = Eigen::Vector3f(negX, negY, 1.0f);
		Vertexs[7].Pos = Eigen::Vector3f(negX, negY, negZ);
		Vertexs[8].Pos = Eigen::Vector3f(negX, 1.0f, negZ);
		Vertexs[9].Pos = Eigen::Vector3f(negX, 1.0f, negZ);
		Vertexs[10].Pos = Eigen::Vector3f(negX, 1.0f, 1.0f);
		Vertexs[11].Pos = Eigen::Vector3f(negX, negY, 1.0f);

		Vertexs[12].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[13].Pos = Eigen::Vector3f(1.0f, negY, 1.0f);
		Vertexs[14].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[15].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[16].Pos = Eigen::Vector3f(1.0f, 1.0f, negZ);
		Vertexs[17].Pos = Eigen::Vector3f(1.0f, negY, negZ);

		Vertexs[18].Pos = Eigen::Vector3f(negX, negY, 1.0f);
		Vertexs[19].Pos = Eigen::Vector3f(negX, 1.0f, 1.0f);
		Vertexs[20].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[21].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[22].Pos = Eigen::Vector3f(1.0f, negY, 1.0f);
		Vertexs[23].Pos = Eigen::Vector3f(negX, negY, 1.0f);

		Vertexs[24].Pos = Eigen::Vector3f(negX, 1.0f, negZ);
		Vertexs[25].Pos = Eigen::Vector3f(1.0f, 1.0f, negZ);
		Vertexs[26].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[27].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[28].Pos = Eigen::Vector3f(negX, 1.0f, 1.0f);
		Vertexs[29].Pos = Eigen::Vector3f(negX, 1.0f, negZ);

		Vertexs[30].Pos = Eigen::Vector3f(negX, negY, negZ);
		Vertexs[31].Pos = Eigen::Vector3f(negX, negY, 1.0f);
		Vertexs[32].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[33].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[34].Pos = Eigen::Vector3f(negX, negY, 1.0f);
		Vertexs[35].Pos = Eigen::Vector3f(1.0f, negY, 1.0f);

		//std::vector<SkyboxVertex> vec(Vertexs + 18, Vertexs + 24);
		std::vector<SkyboxVertex> vec(Vertexs + 0, Vertexs + 36);
		mVertexBuffer = mResourceMng.CreateVertexBuffer(launchMode, sizeof(SkyboxVertex), 0, Data::Make(vec));
	}

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