#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <dxerr.h>
#include <xnamath.h>
#include <time.h>
#include "Utility.h"
#include "Model.h"
#include "AssimpModel.h"

ID3D11InputLayout*      g_pVertexLayout = NULL;
ID3D11VertexShader*     g_pVertexShader = NULL;
ID3D11PixelShader*      g_pPixelShader = NULL;

ID3D11Buffer*           g_pVertexBuffer = NULL;
ID3D11Buffer*           g_pIndexBuffer = NULL;

ID3D11ShaderResourceView*           g_pTextureRV = NULL;
ID3D11SamplerState*                 g_pSamplerLinear = NULL;

ID3D11Buffer*           g_pConstantBuffer = NULL;
ConstantBuffer			g_ConstBuf;

int g_ScreenWidth = 0;
int g_ScreenHeight = 0;

HRESULT InitVS()
{
	ID3DBlob* pVSBlob;
	g_pVertexShader = CreateVS(pVSBlob);

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 12 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);
	HRESULT hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr)) {
		DXTrace(__FILE__, __LINE__, hr, DXGetErrorDescription(hr), FALSE);
		return hr;
	}
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
	return hr;
}

HRESULT InitMVP(int width, int height)
{
	ConstantBuffer constBuf;
	g_pConstantBuffer = CreateUBO(width, height, constBuf);
	g_ConstBuf = constBuf;
	g_ConstBuf.mModel = XMMatrixIdentity();
	return S_OK;
}

void InitAll(int w, int h)
{
	g_ScreenWidth = w;
	g_ScreenHeight = h;

	InitVS();

	g_pPixelShader = CreatePS();
	
	/*{
		g_pVertexBuffer = CreateVBO();

		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	}*/
	
	/*{
		g_pIndexBuffer = CreateVIO();
		g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	}*/
	
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	InitMVP(w, h);
	
	g_pSamplerLinear = CreateSampler();
}

void OnPreInitDevice()
{
}

AssimpModel* gModel;
XMMATRIX changeWorld(float time);
void OnPostInitDevice()
{
	changeWorld(0);

	gModel = new AssimpModel;
	//gModel->LoadModel(g_pd3dDevice, MakeModelPath("Dragon.fbx"));
	//gModel->LoadModel(g_pd3dDevice, MakeModelPath("Dragon_Action.fbx"));
	gModel->LoadModel(g_pd3dDevice, MakeModelPath("Spaceship/Spaceship.fbx"));
	gModel->PlayAnim(2);
}

void DrawMesh(TMesh& mesh)
{
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	if (g_pSamplerLinear) g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &mesh.VertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetIndexBuffer(mesh.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_pImmediateContext->PSSetShaderResources(0, 1, &mesh.textures[0].texture);

	g_pImmediateContext->DrawIndexed(mesh.indices.size(), 0, 0);
}

XMMATRIX ToXM(const aiMatrix4x4& m);
XMMATRIX changeWorld(float time) {
	float rY = (90) / 180.0 * 3.14;
	//float rY = time;
	XMFLOAT3 t = XMFLOAT3(0, 0, 0);
	float scale = 1.0;

	/*XMMATRIX ml = XMMatrixTranslation(0, -50, 0) * XMMatrixScaling(scale, scale, scale);

	aiMatrix4x4 t1, t2;
	aiMatrix4x4 mr = aiMatrix4x4::Translation(aiVector3D(0, -50, 0), t1) * aiMatrix4x4::Scaling(aiVector3D(scale, scale, scale), t2);*/

	int mx, my;
	InputGetMouseLocation(&mx, &my);
	float angy = 3.14 * -mx / g_ScreenWidth, angx = 3.14 * -my / g_ScreenHeight;

	XMMATRIX euler = XMMatrixRotationZ(0) * XMMatrixRotationX(angx) * XMMatrixRotationY(angy);
	return euler
		* XMMatrixTranslation(t.x, t.y, t.z) 
		* XMMatrixScaling(scale, scale, scale);
}

int drawFlag = 0;

void DrawNode(aiNode* node)
{
	auto& meshes = gModel->mNodeInfos[node];
	if (meshes.size() > 0) {
		g_ConstBuf.mModel = ToXM(gModel->mNodeInfos[node].mGlobalTransform);
		g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &g_ConstBuf, 0, 0);

		for (int i = 0; i < meshes.size(); i++) {
			auto mesh = meshes[i];

			if (mesh->data->HasBones()) {
				const auto& boneMats = gModel->GetBoneMatrices(node, i);
				size_t boneSize = boneMats.size(); assert(boneSize <= MAX_MATRICES);
				for (int j = 0; j < min(MAX_MATRICES, boneSize); ++j)
					g_ConstBuf.Models[j] = ToXM(boneMats[j]);
				g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &g_ConstBuf, 0, 0);
			
				//DrawMesh(*mesh);
			}
			DrawMesh(*mesh);
		}
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		DrawNode(node->mChildren[i]);
	}
}

double gLastTime;
void OnDisplay()
{
#if 0
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	if (g_pSamplerLinear) g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->DrawIndexed(36, 0, 0);
#else
	double time = GetTickCount() / 1000.0;
	double dt = gLastTime == 0.0f ? 0.0 : time - gLastTime;
	gLastTime = time;
	gModel->Update(dt);

	DrawNode(gModel->mRootNode);

	drawFlag = 0;
#endif
}

void OnRender()
{
	// Update our time
	static float t = 0.0f;
	if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}

	// Rotate cube around the origin
	g_ConstBuf.mWorld = changeWorld(t);// XMMatrixRotationY(t);

	// Setup our lighting parameters
	XMFLOAT4 vLightDirs[2] =
	{
		XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
	};
	XMFLOAT4 vLightColors[2] =
	{
		XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
		XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f)
	};

	// Rotate the second light around the origin
	XMMATRIX mRotate = XMMatrixRotationY(-2.0f * t);
	XMVECTOR vLightDir = XMLoadFloat4(&vLightDirs[1]);
	vLightDir = XMVector3Transform(vLightDir, mRotate);
	XMStoreFloat4(&vLightDirs[1], vLightDir);

	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	ConstantBuffer& cb1 = g_ConstBuf;
	/*cb1.mWorld = (g_ConstBuf.mWorld);
	cb1.mView = (g_ConstBuf.mView);
	cb1.mProjection = (g_ConstBuf.mProjection);*/
	cb1.vLightDir[0] = vLightDirs[0];
	cb1.vLightDir[1] = vLightDirs[1];
	cb1.vLightColor[0] = vLightColors[0];
	cb1.vLightColor[1] = vLightColors[1];
	cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb1, 0, 0);

	OnDisplay();

	g_pSwapChain->Present(0, 0);
}

void OnCleanUp()
{
	if (g_pConstantBuffer) g_pConstantBuffer->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
}