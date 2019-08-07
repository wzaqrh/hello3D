#include "TRenderSystem9.h"
#include "TMaterial.h"
#include "Utility.h"
#include "TSkyBox.h"
#include "TPostProcess.h"
#include "TThreadPump.h"
#include <d3dcompiler.h>

TRenderSystem9::TRenderSystem9()
{

}

TRenderSystem9::~TRenderSystem9()
{

}

HRESULT TRenderSystem9::Initialize()
{

	return S_OK;
}

void TRenderSystem9::Update(float dt)
{

}

void TRenderSystem9::CleanUp()
{

}

TSpotLightPtr TRenderSystem9::AddSpotLight()
{
	return nullptr;
}

TPointLightPtr TRenderSystem9::AddPointLight()
{
	return nullptr;
}

TDirectLightPtr TRenderSystem9::AddDirectLight()
{
	return nullptr;
}

TCameraPtr TRenderSystem9::SetCamera(double fov, int eyeDistance, double far1)
{
	return nullptr;
}

TSkyBoxPtr TRenderSystem9::SetSkyBox(const std::string& imgName)
{
	return nullptr;
}

TPostProcessPtr TRenderSystem9::AddPostProcess(const std::string& name)
{
	return nullptr;
}

void TRenderSystem9::SetHandle(HINSTANCE hInstance, HWND hWnd)
{

}

void TRenderSystem9::ClearColor(const XMFLOAT4& color)
{

}

void TRenderSystem9::ClearDepthStencil(FLOAT Depth, UINT8 Stencil)
{

}

TRenderTexturePtr TRenderSystem9::CreateRenderTexture(int width, int height, DXGI_FORMAT format/*=DXGI_FORMAT_R32G32B32A32_FLOAT*/)
{
	return nullptr;
}

void TRenderSystem9::ClearRenderTexture(TRenderTexturePtr rendTarget, XMFLOAT4 color)
{

}

void TRenderSystem9::SetRenderTarget(TRenderTexturePtr rendTarget)
{

}

TMaterialPtr TRenderSystem9::CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback)
{
	return nullptr;
}

TContantBufferPtr TRenderSystem9::CloneConstBuffer(TContantBufferPtr buffer)
{
	return nullptr;
}

TContantBufferPtr TRenderSystem9::CreateConstBuffer(int bufferSize, void* data /*= nullptr*/)
{
	return nullptr;
}

TIndexBufferPtr TRenderSystem9::CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer)
{
	return nullptr;
}

void TRenderSystem9::SetIndexBuffer(TIndexBufferPtr indexBuffer)
{

}

void TRenderSystem9::DrawIndexed(TIndexBufferPtr indexBuffer)
{

}

TVertexBufferPtr TRenderSystem9::CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer/*=nullptr*/)
{
	return nullptr;
}

void TRenderSystem9::SetVertexBuffer(TVertexBufferPtr vertexBuffer)
{

}

bool TRenderSystem9::UpdateBuffer(THardwareBuffer* buffer, void* data, int dataSize)
{
	return false;
}

void TRenderSystem9::UpdateConstBuffer(TContantBufferPtr buffer, void* data)
{

}

TProgramPtr TRenderSystem9::CreateProgramByCompile(const char* vsPath, const char* psPath /*= nullptr*/, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	return nullptr;
}

TProgramPtr TRenderSystem9::CreateProgramByFXC(const std::string& name, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	return nullptr;
}

TProgramPtr TRenderSystem9::CreateProgram(const std::string& name, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	return nullptr;
}

ID3D11SamplerState* TRenderSystem9::CreateSampler(D3D11_FILTER filter /*= D3D11_FILTER_MIN_MAG_MIP_LINEAR*/, D3D11_COMPARISON_FUNC comp /*= D3D11_COMPARISON_NEVER*/)
{
	return nullptr;
}

TInputLayoutPtr TRenderSystem9::CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	return nullptr;
}

TTexturePtr TRenderSystem9::GetTexByPath(const std::string& __imgPath, DXGI_FORMAT format /*= DXGI_FORMAT_UNKNOWN*/)
{
	return nullptr;
}

void TRenderSystem9::SetBlendFunc(const TBlendFunc& blendFunc)
{

}

void TRenderSystem9::SetDepthState(const TDepthState& depthState)
{

}

bool TRenderSystem9::BeginScene()
{
	return false;
}

void TRenderSystem9::EndScene()
{

}

void TRenderSystem9::Draw(IRenderable* renderable)
{

}

void TRenderSystem9::RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode)
{

}
