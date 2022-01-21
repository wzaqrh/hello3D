#include <boost/filesystem.hpp>
#include "core/renderable/skybox.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace renderable {

SkyBox::SkyBox(Launch launchMode, ResourceManager& resourceMng, const ShaderLoadParam& matName, const std::string& imgName)
	:Super(launchMode, resourceMng, matName)
{
	if (matName.VariantName == "Deprecate") {
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
		//-Z
		int face = 0;
		Vertexs[0].Pos = Eigen::Vector3f(negX, 1.0f, negZ);
		Vertexs[1].Pos = Eigen::Vector3f(negX, negY, negZ);
		Vertexs[2].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[3].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[4].Pos = Eigen::Vector3f(1.0f, 1.0f, negZ);
		Vertexs[5].Pos = Eigen::Vector3f(negX, 1.0f, negZ);

		face = 1;
		Vertexs[face * 6 + 0].Pos = Eigen::Vector3f(negX, negY, 1.0f);
		Vertexs[face * 6 + 1].Pos = Eigen::Vector3f(negX, negY, negZ);
		Vertexs[face * 6 + 2].Pos = Eigen::Vector3f(negX, 1.0f, negZ);
		Vertexs[face * 6 + 3].Pos = Eigen::Vector3f(negX, 1.0f, negZ);
		Vertexs[face * 6 + 4].Pos = Eigen::Vector3f(negX, 1.0f, 1.0f);
		Vertexs[face * 6 + 5].Pos = Eigen::Vector3f(negX, negY, 1.0f);

		face = 2;
		Vertexs[face * 6 + 0].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[face * 6 + 1].Pos = Eigen::Vector3f(1.0f, negY, 1.0f);
		Vertexs[face * 6 + 2].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[face * 6 + 3].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[face * 6 + 4].Pos = Eigen::Vector3f(1.0f, 1.0f, negZ);
		Vertexs[face * 6 + 5].Pos = Eigen::Vector3f(1.0f, negY, negZ);

		//+Z
		face = 3;
		Vertexs[face * 6 + 0].Pos = Eigen::Vector3f(negX, negY, 1.0f);
		Vertexs[face * 6 + 1].Pos = Eigen::Vector3f(negX, 1.0f, 1.0f);
		Vertexs[face * 6 + 2].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[face * 6 + 3].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[face * 6 + 4].Pos = Eigen::Vector3f(1.0f, negY, 1.0f);
		Vertexs[face * 6 + 5].Pos = Eigen::Vector3f(negX, negY, 1.0f);

		face = 4;
		Vertexs[face * 6 + 0].Pos = Eigen::Vector3f(negX, 1.0f, negZ);
		Vertexs[face * 6 + 1].Pos = Eigen::Vector3f(1.0f, 1.0f, negZ);
		Vertexs[face * 6 + 2].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[face * 6 + 3].Pos = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
		Vertexs[face * 6 + 4].Pos = Eigen::Vector3f(negX, 1.0f, 1.0f);
		Vertexs[face * 6 + 5].Pos = Eigen::Vector3f(negX, 1.0f, negZ);

		face = 5;
		Vertexs[face * 6 + 0].Pos = Eigen::Vector3f(negX, negY, negZ);
		Vertexs[face * 6 + 1].Pos = Eigen::Vector3f(negX, negY, 1.0f);
		Vertexs[face * 6 + 2].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[face * 6 + 3].Pos = Eigen::Vector3f(1.0f, negY, negZ);
		Vertexs[face * 6 + 4].Pos = Eigen::Vector3f(negX, negY, 1.0f);
		Vertexs[face * 6 + 5].Pos = Eigen::Vector3f(1.0f, negY, 1.0f);

		std::vector<SkyboxVertex> vec(Vertexs + 0, Vertexs + 36);
		//face = 3; vec.assign(Vertexs + face * 6, Vertexs + (face + 1) * 6);
		mVertexBuffer = mResourceMng.CreateVertexBuffer(launchMode, sizeof(SkyboxVertex), 0, Data::Make(vec));
	}

	if (!boost::filesystem::is_regular_file(imgName)) {
		boost::filesystem::path dir(imgName);
		dir.remove_filename();
		boost::filesystem::path specularEnvPath = dir / "specular_env.dds";
		if (boost::filesystem::exists(specularEnvPath)) {
			boost::filesystem::path lutPath = dir / "lut.png";
			mLutMap = mResourceMng.CreateTextureByFile(launchMode, lutPath.string());

			boost::filesystem::path diffuseEnvPath = dir / "diffuse_env.dds";
			mDiffuseEnvMap = mResourceMng.CreateTextureByFile(launchMode, diffuseEnvPath.string());

		#if USE_MATERIAL_INSTANCE
			SetTexture(mResourceMng.CreateTextureByFile(launchMode, specularEnvPath.string()));
		#else
			mTexture = mResourceMng.CreateTextureByFile(launchMode, specularEnvPath.string());
		#endif
		}
	}
	else {
	#if USE_MATERIAL_INSTANCE
		SetTexture(mResourceMng.CreateTextureByFile(launchMode, imgName));
	#else
		mTexture = mResourceMng.CreateTextureByFile(launchMode, imgName);
	#endif
	}
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
}