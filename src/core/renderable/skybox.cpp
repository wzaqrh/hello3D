#include "core/renderable/skybox.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/material_factory.h"

namespace mir {

SkyBox::SkyBox(IRenderSystem& renderSys, MaterialFactory& matFac, const std::string& imgName)
	:mRenderSys(renderSys)
{
	mMaterial = matFac.GetMaterial(E_MAT_SKYBOX);
	mIndexBuffer = nullptr;

	SkyboxVertex Vertexs[4];
	float fHighW = -1.0f - (1.0f / (float)mRenderSys.WinSize().x());
	float fHighH = -1.0f - (1.0f / (float)mRenderSys.WinSize().y());
	float fLowW = 1.0f + (1.0f / (float)mRenderSys.WinSize().x());
	float fLowH = 1.0f + (1.0f / (float)mRenderSys.WinSize().y());
	Vertexs[0].pos = Eigen::Vector4f(fLowW, fLowH, 1.0f, 1.0f);
	Vertexs[1].pos = Eigen::Vector4f(fLowW, fHighH, 1.0f, 1.0f);
	Vertexs[2].pos = Eigen::Vector4f(fHighW, fLowH, 1.0f, 1.0f);
	Vertexs[3].pos = Eigen::Vector4f(fHighW, fHighH, 1.0f, 1.0f);
	mVertexBuffer = mRenderSys.CreateVertexBuffer(sizeof(SkyboxVertex) * 4, sizeof(SkyboxVertex), 0, Vertexs);

	DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	mCubeSRV = mRenderSys.LoadTexture(imgName, format, true, true);
#if 0
	auto pCam1 = mContext->GetSceneMng()->GetDefCamera();
	Eigen::Vector3f pos0 = pCam->ProjectPoint(Eigen::Vector3f(fLowW, fLowH, 1.0f));
	Eigen::Vector3f pos1 = pCam->ProjectPoint(Eigen::Vector3f(fLowW, fHighH, 1.0f));
	Eigen::Vector3f pos2 = pCam->ProjectPoint(Eigen::Vector3f(fHighW, fLowH, 1.0f));
	Eigen::Vector3f pos3 = pCam->ProjectPoint(Eigen::Vector3f(fHighW, fHighH, 1.0f));
#endif
}

SkyBox::~SkyBox()
{
}

int SkyBox::GenRenderOperation(RenderOperationQueue& opList)
{
	RenderOperation op = {};
	op.mMaterial = mMaterial;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	op.mTextures.Add(mCubeSRV);
	op.mWorldTransform = Eigen::Matrix4f::Identity();
	opList.AddOP(op);
	return 1;
}

}