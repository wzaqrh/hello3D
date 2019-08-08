#include "TRenderSystem9.h"
#include "TMaterial.h"
#include "Utility.h"
#include "TSkyBox.h"
#include "TPostProcess.h"
#include "TInterfaceType9.h"

TRenderSystem9::TRenderSystem9()
{

}

TRenderSystem9::~TRenderSystem9()
{

}

bool TRenderSystem9::Initialize()
{
	RECT rc;
	GetClientRect(mHWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	if (!_CreateDeviceAndSwapChain()) return false;

	_SetRasterizerState();

	SetDepthState(TDepthState(TRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL));
	SetBlendFunc(TBlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA));

	mScreenWidth = width;
	mScreenHeight = height;
	mDefCamera = std::make_shared<TCamera>(mScreenWidth, mScreenHeight);

	mInput = new TD3DInput(mHInst, mHWnd, width, height);

	//mShadowPassRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R32_FLOAT);
	//SET_DEBUG_NAME(mShadowPassRT->mDepthStencilView, "mShadowPassRT");

	//mPostProcessRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R16G16B16A16_UNORM);// , DXGI_FORMAT_R8G8B8A8_UNORM);
	//SET_DEBUG_NAME(mPostProcessRT->mDepthStencilView, "mPostProcessRT");
	return true;
}

bool TRenderSystem9::_CreateDeviceAndSwapChain()
{
	if (NULL == (mD3D9 = Direct3DCreate9(D3D_SDK_VERSION))) {
		CheckHR(E_FAIL);
		return false;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	if (CheckHR(mD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mHWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &mDevice9))) {
		return false;
	}
	return true;
}

void TRenderSystem9::_SetRasterizerState()
{
	mDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	mDevice9->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	//g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
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
	mHInst = hInstance;
	mHWnd = hWnd;
}

static inline D3DCOLOR XMFLOAT2D3DCOLOR(XMFLOAT4 color) {
	return D3DCOLOR_ARGB(int(color.w * 255), int(color.x * 255), int(color.y * 255), int(color.z * 255));
}
void TRenderSystem9::ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil)
{
	mDevice9->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, XMFLOAT2D3DCOLOR(color), Depth, Stencil);
}

IRenderTexturePtr TRenderSystem9::CreateRenderTexture(int width, int height, DXGI_FORMAT format/*=DXGI_FORMAT_R32G32B32A32_FLOAT*/)
{
	IDirect3DTexture9 *pTextureColor = nullptr;
	D3DFORMAT Format = D3DEnumCT::d3d11To9(format);
	if (CheckHR(mDevice9->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &pTextureColor, NULL))) return nullptr;

	IDirect3DSurface9 *pSurfaceColor = nullptr;
	if (CheckHR(pTextureColor->GetSurfaceLevel(0, &pSurfaceColor))) return nullptr;

	IDirect3DSurface9 *pSurfaceDepthStencil = nullptr;
	if (CheckHR(mDevice9->CreateDepthStencilSurface(width, height, D3DFMT_D24FS8, D3DMULTISAMPLE_NONE, 0, TRUE, &pSurfaceDepthStencil, NULL))) return false;

	TRenderTexture9Ptr ret = std::make_shared<TRenderTexture9>(pSurfaceColor, pSurfaceDepthStencil);
	return ret;
}

void TRenderSystem9::ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color, FLOAT Depth/* = 1.0*/, UINT8 Stencil/* = 0*/)
{
	if (CheckHR(mDevice9->ColorFill(rendTarget->GetColorBuffer9(), NULL, XMFLOAT2D3DCOLOR(color)))) return;
}

void TRenderSystem9::SetRenderTarget(IRenderTexturePtr rendTarget)
{

}

TMaterialPtr TRenderSystem9::CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback)
{
	return nullptr;
}

IContantBufferPtr TRenderSystem9::CloneConstBuffer(IContantBufferPtr buffer)
{
	return nullptr;
}

IContantBufferPtr TRenderSystem9::CreateConstBuffer(int bufferSize, void* data /*= nullptr*/)
{
	return nullptr;
}

IIndexBufferPtr TRenderSystem9::CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer)
{
	TIndexBuffer9Ptr ret;
	IDirect3DIndexBuffer9* pIndexBuffer = nullptr;
	D3DFORMAT Format = D3DEnumCT::d3d11To9(format);
	if (! CheckHR(mDevice9->CreateIndexBuffer(bufferSize, 0, Format, D3DPOOL_DEFAULT, &pIndexBuffer, NULL))) {
		ret = std::make_shared<TIndexBuffer9>(pIndexBuffer, bufferSize, format);
	}
	if (buffer) UpdateBuffer(ret.get(), buffer, bufferSize);
	return ret;
}

void TRenderSystem9::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	mDevice9->SetIndices(indexBuffer->GetBuffer9());
}

IVertexBufferPtr TRenderSystem9::CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer/*=nullptr*/)
{
	TVertexBuffer9Ptr ret;
	IDirect3DVertexBuffer9* pVertexBuffer = nullptr;
	if (! CheckHR(mDevice9->CreateVertexBuffer(bufferSize, 0, 0/*non-FVF*/, D3DPOOL_DEFAULT, &pVertexBuffer, NULL))) {
		ret = std::make_shared<TVertexBuffer9>(pVertexBuffer, bufferSize, stride, offset);
	}
	if (buffer) UpdateBuffer(ret.get(), buffer, bufferSize);
	return ret;
}

void TRenderSystem9::SetVertexBuffer(IVertexBufferPtr vertexBuffer)
{
	UINT offset = vertexBuffer->GetOffset();
	UINT stride = vertexBuffer->GetStride();
	IDirect3DVertexBuffer9* buffer = vertexBuffer->GetBuffer9();
	mDevice9->SetStreamSource(0, buffer, offset, stride);
}

bool TRenderSystem9::UpdateBuffer(IHardwareBuffer* buffer, void* data, int dataSize)
{
	enHardwareBufferType bufferType = buffer->GetType();
	switch (bufferType)
	{
	case E_HWBUFFER_CONSTANT: {

	}break;
	case E_HWBUFFER_VERTEX: {
		IDirect3DVertexBuffer9* buffer9 = static_cast<IVertexBuffer*>(buffer)->GetBuffer9();
		void* pByteDest = nullptr;
		if (CheckHR(buffer9->Lock(0, dataSize, &pByteDest, 0))) return false;
		memcpy(pByteDest, data, dataSize);
		if (CheckHR(buffer9->Unlock())) return false;
	}break;
	case E_HWBUFFER_INDEX: {
		IDirect3DIndexBuffer9* buffer9 = static_cast<IIndexBuffer*>(buffer)->GetBuffer9();
		void* pByteDest = nullptr;
		if (CheckHR(buffer9->Lock(0, dataSize, &pByteDest, 0))) return false;
		memcpy(pByteDest, data, dataSize);
		if (CheckHR(buffer9->Unlock())) return false;
	}break;
	default:
		break;
	}
	return true;
}

void TRenderSystem9::UpdateConstBuffer(IContantBufferPtr buffer, void* data)
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

ISamplerStatePtr TRenderSystem9::CreateSampler(D3D11_FILTER filter /*= D3D11_FILTER_MIN_MAG_MIP_LINEAR*/, D3D11_COMPARISON_FUNC comp /*= D3D11_COMPARISON_NEVER*/)
{
	return nullptr;
}

IInputLayoutPtr TRenderSystem9::CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	return nullptr;
}

ITexturePtr TRenderSystem9::_CreateTexture(const char* pSrcFile, DXGI_FORMAT format /*= DXGI_FORMAT_UNKNOWN*/, bool async /*= false*/)
{
	std::string imgPath = GetModelPath() + pSrcFile;
#ifdef USE_ONLY_PNG
	if (!IsFileExist(imgPath)) {
		auto pos = imgPath.find_last_of(".");
		if (pos != std::string::npos) {
			imgPath = imgPath.substr(0, pos);
			imgPath += ".png";
		}
	}
#endif
	pSrcFile = imgPath.c_str();

	TTexture9Ptr pTextureRV = std::make_shared<TTexture9>(nullptr, imgPath);
	if (CheckHR(D3DXCreateTextureFromFileA(mDevice9, pSrcFile, &pTextureRV->GetSRV9()))) {
		pTextureRV = nullptr;
	}
	return pTextureRV;
}

void TRenderSystem9::SetBlendFunc(const TBlendFunc& blendFunc)
{
	mDevice9->SetRenderState(D3DRS_SRCBLEND, D3DEnumCT::d3d11To9(blendFunc.src));
	mDevice9->SetRenderState(D3DRS_DESTBLEND, D3DEnumCT::d3d11To9(blendFunc.dst));
}

void TRenderSystem9::SetDepthState(const TDepthState& depthState)
{
	mDevice9->SetRenderState(D3DRS_ZENABLE, depthState.depthEnable);
	mDevice9->SetRenderState(D3DRS_ZFUNC, D3DEnumCT::d3d11To9(depthState.depthFunc));
	mDevice9->SetRenderState(D3DRS_ZWRITEENABLE, depthState.depthWriteMask == D3D11_DEPTH_WRITE_MASK_ALL ? TRUE : FALSE);
}

bool TRenderSystem9::BeginScene()
{
	if (FAILED(mDevice9->BeginScene())) 
		return false;
	return false;
}

void TRenderSystem9::EndScene()
{
	mDevice9->EndScene();
	mDevice9->Present(NULL, NULL, NULL, NULL);
}

void TRenderSystem9::Draw(IRenderable* renderable)
{

}

void TRenderSystem9::RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode)
{

}