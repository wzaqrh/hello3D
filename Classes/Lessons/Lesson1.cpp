#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <dxerr.h>
#include <xnamath.h>
#include <time.h>
#include "Utility.h"
#include "Lesson1.h"
#include "AssimpModel.h"
#include "TRenderSystem.h"

ConstantBuffer::ConstantBuffer()
{
	auto Ident = XMMatrixIdentity();
	mWorld = Ident;
	mView = Ident;
	mProjection = Ident;
	mModel = Ident;
	for (int i = 0; i < MAX_MATRICES; ++i)
		Models[i] = Ident;
}

void TAppLesson1::OnPostInitDevice()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 12 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	mMaterial = mRenderSys->CreateMaterial("shader\\Model.fx", "shader\\Model.fx", layout, ARRAYSIZE(layout));
	mMaterial->mConstantBuffer = mRenderSys->CreateConstBuffer(sizeof(ConstantBuffer));

	mModel = new AssimpModel(mRenderSys);
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx"));
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx"));
	mModel->PlayAnim(0);
}

void TAppLesson1::DrawMesh(TMesh& mesh)
{
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	mRenderSys->mDeviceContext->IASetVertexBuffers(0, 1, &mesh.mVertexBuffer, &stride, &offset);
	mRenderSys->mDeviceContext->IASetIndexBuffer(mesh.mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	if (mesh.textures.size() > 0)
		mRenderSys->mDeviceContext->PSSetShaderResources(0, 1, &mesh.textures[0].texture);

	mRenderSys->mDeviceContext->DrawIndexed(mesh.indices.size(), 0, 0);
}

void TAppLesson1::DrawNode(aiNode* node)
{
	auto& meshes = mModel->mNodeInfos[node];
	if (meshes.size() > 0) {
		mConstBuf.mModel = ToXM(mModel->mNodeInfos[node].mGlobalTransform);
		mRenderSys->mDeviceContext->UpdateSubresource(mMaterial->mConstantBuffer, 0, NULL, &mConstBuf, 0, 0);

		for (int i = 0; i < meshes.size(); i++) {
			auto mesh = meshes[i];

			if (mesh->data->HasBones()) {
				const auto& boneMats = mModel->GetBoneMatrices(node, i);
				size_t boneSize = boneMats.size(); assert(boneSize <= MAX_MATRICES);
				for (int j = 0; j < min(MAX_MATRICES, boneSize); ++j)
					mConstBuf.Models[j] = ToXM(boneMats[j]);
				mRenderSys->mDeviceContext->UpdateSubresource(mMaterial->mConstantBuffer, 0, NULL, &mConstBuf, 0, 0);
			}
			else {
				mConstBuf.Models[0] = XMMatrixIdentity();
			}
			DrawMesh(*mesh);
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		DrawNode(node->mChildren[i]);
}

void TAppLesson1::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);

	{
		mConstBuf.mWorld = GetWorldTransform();
		mConstBuf.mView = mRenderSys->mDefCamera->mView;
		mConstBuf.mProjection = mRenderSys->mDefCamera->mProjection;

		mRenderSys->mDeviceContext->VSSetShader(mMaterial->mVertexShader, NULL, 0);
		mRenderSys->mDeviceContext->VSSetConstantBuffers(0, 1, &mMaterial->mConstantBuffer);

		mRenderSys->mDeviceContext->PSSetShader(mMaterial->mPixelShader, NULL, 0);
		mRenderSys->mDeviceContext->PSSetConstantBuffers(0, 1, &mMaterial->mConstantBuffer);
		mRenderSys->mDeviceContext->IASetInputLayout(mMaterial->mInputLayout);

		mRenderSys->mDeviceContext->IASetPrimitiveTopology(mMaterial->mTopoLogy);

		if (mMaterial->mSampler) mRenderSys->mDeviceContext->PSSetSamplers(0, 1, &mMaterial->mSampler);
	}

	DrawNode(mModel->mRootNode);
}

void TAppLesson1::OnPreInitDevice()
{

}

auto reg = AppRegister<TAppLesson1>("TAppLesson1");
