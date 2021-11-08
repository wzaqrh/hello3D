#include "render_system11.h"
#include "interface_type11.h"
#include "thread_pump.h"
#include "core/rendersys/scene_manager.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/post_process.h"
#include "core/renderable/skybox.h"
#include "core/base/utility.h"

namespace mir {

RenderSystem11::RenderSystem11()
{
	//mMaterialFac = std::make_shared<MaterialFactory>(*this);
	mThreadPump = std::make_shared<ThreadPump>();
	mFXCDir = "d3d11\\";
}

RenderSystem11::~RenderSystem11()
{
}

bool RenderSystem11::Initialize(HWND hWnd, RECT vp)
{
	mHWnd = hWnd;
	
	if (vp.right == 0 || vp.bottom == 0)
		GetClientRect(mHWnd, &vp);
	UINT vpWidth = vp.right - vp.left;
	UINT vpHeight = vp.bottom - vp.top;

	RECT rc;
	GetClientRect(mHWnd, &rc);
	UINT rcWidth = rc.right - rc.left;
	UINT rcHeight = rc.bottom - rc.top;

	if (CheckHR(_CreateDeviceAndSwapChain(rcWidth, rcHeight))) return false;

	if (CheckHR(_CreateBackRenderTargetView())) return false;
	if (CheckHR(_CreateBackDepthStencilView(rcWidth, rcHeight))) return false;
	mDeviceContext->OMSetRenderTargets(1, &mBackRenderTargetView, mBackDepthStencilView);

	_SetViewports(vpWidth, vpHeight, vp.left, vp.top);

	if (CheckHR(_SetRasterizerState())) return false;

	SetDepthState(DepthState(TRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL));
	SetBlendFunc(BlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA));

	mScreenWidth = vpWidth;
	mScreenHeight = vpHeight;

	mShadowPassRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R32_FLOAT);
	SET_DEBUG_NAME(mShadowPassRT->mDepthStencilView, "mShadowPassRT");

	mPostProcessRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R16G16B16A16_UNORM);// , DXGI_FORMAT_R8G8B8A8_UNORM);
	SET_DEBUG_NAME(mPostProcessRT->mDepthStencilView, "mPostProcessRT");

	//mSceneManager = MakePtr<SceneManager>(*this, *mMaterialFac, XMINT2(mScreenWidth, mScreenHeight), mPostProcessRT, Camera::CreatePerspective(mScreenWidth, mScreenHeight));

	D3D_SHADER_MACRO Shader_Macros[] = { "SHADER_MODEL", "40000", NULL, NULL };
	mShaderMacros.assign(Shader_Macros, Shader_Macros+ARRAYSIZE(Shader_Macros));
	return true;
}

HRESULT RenderSystem11::_CreateDeviceAndSwapChain(int width, int height)
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = mHWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		mDriverType = driverTypes[driverTypeIndex];
		mDriverType = D3D_DRIVER_TYPE_HARDWARE;
		hr = D3D11CreateDeviceAndSwapChain(NULL, mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &mSwapChain, &mDevice, &mFeatureLevel, &mDeviceContext);
		if (SUCCEEDED(hr))
			break;
	}
	return hr;
}

HRESULT RenderSystem11::_CreateBackRenderTargetView()
{
	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	HRESULT hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (CheckHR(hr))
		return hr;

	hr = mDevice->CreateRenderTargetView(pBackBuffer, NULL, &mBackRenderTargetView); mCurRenderTargetView = mBackRenderTargetView;
	pBackBuffer->Release();
	return hr;
}

HRESULT RenderSystem11::_CreateBackDepthStencilView(int width, int height)
{
	HRESULT hr = S_OK;
	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = mDevice->CreateTexture2D(&descDepth, NULL, &mDepthStencil);
	if (CheckHR(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = mDevice->CreateDepthStencilView(mDepthStencil, &descDSV, &mBackDepthStencilView); mCurDepthStencilView = mBackDepthStencilView;
	return hr;
}

void RenderSystem11::_SetViewports(int width, int height, int x, int y)
{
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = x;
	vp.TopLeftY = y;
	mDeviceContext->RSSetViewports(1, &vp);
}

HRESULT RenderSystem11::_SetRasterizerState()
{
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_SOLID;
	wfdesc.CullMode = D3D11_CULL_NONE;// D3D11_CULL_BACK;
	ID3D11RasterizerState* pRasterizerState = nullptr;
	HRESULT hr = mDevice->CreateRasterizerState(&wfdesc, &pRasterizerState);
	mDeviceContext->RSSetState(pRasterizerState);
	return hr;
}

void RenderSystem11::Update(float dt)
{
	mThreadPump->Update(dt);
}

void RenderSystem11::CleanUp()
{
}

void RenderSystem11::ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil)
{
	float colorArr[4] = { color.x, color.y, color.z, color.w }; // red, green, blue, alpha
	mDeviceContext->ClearRenderTargetView(mCurRenderTargetView, colorArr);
	mDeviceContext->ClearDepthStencilView(mCurDepthStencilView, D3D11_CLEAR_DEPTH, Depth, Stencil);
}

IRenderTexturePtr RenderSystem11::CreateRenderTexture(int width, int height, DXGI_FORMAT format)
{
	return MakePtr<RenderTexture11>(mDevice, width, height, format);
}

void RenderSystem11::_ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color, FLOAT Depth/* = 1.0*/, UINT8 Stencil/* = 0*/)
{
	auto target11 = std::static_pointer_cast<RenderTexture11>(rendTarget);
	mDeviceContext->ClearRenderTargetView(target11->GetColorBuffer11(), (const float*)&color);
	mDeviceContext->ClearDepthStencilView(target11->GetDepthStencilBuffer11(), D3D11_CLEAR_DEPTH, Depth, Stencil);
}

void RenderSystem11::SetViewPort(int x, int y, int w, int h)
{
	_SetViewports(w, h, x, y);
}

void RenderSystem11::SetRenderTarget(IRenderTexturePtr rendTarget)
{
	ID3D11ShaderResourceView* TextureNull = nullptr;
	mDeviceContext->PSSetShaderResources(0, 1, &TextureNull);

	auto target11 = std::static_pointer_cast<RenderTexture11>(rendTarget);
	mCurRenderTargetView = target11 != nullptr ? target11->GetColorBuffer11() : mBackRenderTargetView;
	mCurDepthStencilView = target11 != nullptr ? target11->GetDepthStencilBuffer11() : mBackDepthStencilView;
	mDeviceContext->OMSetRenderTargets(1, &mCurRenderTargetView, mCurDepthStencilView);
}

ID3D11InputLayout* RenderSystem11::_CreateInputLayout(Program11* pProgram, const std::vector<D3D11_INPUT_ELEMENT_DESC>& descArr)
{
	ID3D11InputLayout* pVertexLayout;
	auto blob = pProgram->mVertex->GetBlob();
	HRESULT hr = mDevice->CreateInputLayout(&descArr[0], descArr.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &pVertexLayout);
	if (CheckHR(hr)) {
		DXTrace(__FILE__, __LINE__, hr, DXGetErrorDescription(hr), FALSE);
		return pVertexLayout;
	}
	return pVertexLayout;
}
IInputLayoutPtr RenderSystem11::CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	InputLayout11Ptr ret = MakePtr<InputLayout11>();
	ret->mInputDescs.assign(descArray, descArray + descCount);

	auto resource = pProgram->AsRes();
	if (resource->IsLoaded()) {
		ret->mLayout = _CreateInputLayout(std::static_pointer_cast<Program11>(pProgram).get(), ret->mInputDescs);
		resource->SetLoaded();
	}
	else {
		resource->AddOnLoadedListener([=](IResource* res) {
			ret->mLayout = _CreateInputLayout(std::static_pointer_cast<Program11>(pProgram).get(), ret->mInputDescs);
			res->SetLoaded();
		});
	}
	return ret;
}

bool RenderSystem11::UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize)
{
	assert(buffer != nullptr);
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	switch (buffer->GetType())
	{
	case kHWBufferConstant: {
		IContantBufferPtr cbuffer = std::static_pointer_cast<IContantBuffer>(buffer);
		ContantBuffer11Ptr cbuffer11 = std::static_pointer_cast<ContantBuffer11>(cbuffer);
		hr = (mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		if (CheckHR(hr)) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	case kHWBufferVertex:{
		IVertexBufferPtr cbuffer = std::static_pointer_cast<IVertexBuffer>(buffer);
		VertexBuffer11Ptr cbuffer11 = std::static_pointer_cast<VertexBuffer11>(cbuffer);
		hr = (mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		if (CheckHR(hr)) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	case kHWBufferIndex: {
		IIndexBufferPtr cbuffer = std::static_pointer_cast<IIndexBuffer>(buffer);
		IndexBuffer11Ptr cbuffer11 = std::static_pointer_cast<IndexBuffer11>(cbuffer);
		hr = (mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		if (CheckHR(hr)) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	default:
		break;
	}
	return true;
}

ISamplerStatePtr RenderSystem11::CreateSampler(D3D11_FILTER filter, D3D11_COMPARISON_FUNC comp)
{
	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = filter;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;//D3D11_TEXTURE_ADDRESS_MIRROR
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
	sampDesc.ComparisonFunc = comp;
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* pSamplerLinear = nullptr;
	hr = mDevice->CreateSamplerState(&sampDesc, &pSamplerLinear);
	if (CheckHR(hr))
		return nullptr;

	SamplerState11Ptr ret = MakePtr<SamplerState11>(pSamplerLinear);
	return ret;
}

DWORD GetShaderFlag() {
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG) && defined(D3D11_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	return dwShaderFlags;
}
bool CheckCompileError(HRESULT hr, ID3DBlob* pErrorBlob) {
	bool ret = true;
	if (pErrorBlob != NULL) {
		OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		pErrorBlob->Release();
		pErrorBlob = nullptr;
	}
	if (FAILED(hr)) {
		CheckHR(hr);
		ret = false;
	}
	return ret;
}
PixelShader11Ptr RenderSystem11::_CreatePS(const char* filename, const char* szEntry, bool async)
{
	PixelShader11Ptr ret = MakePtr<PixelShader11>(MakePtr<BlobData11>(nullptr));
	szEntry = szEntry ? szEntry : "PS";
	const char* shaderModel = "ps_4_0";
	DWORD dwShaderFlags = GetShaderFlag();

	HRESULT hr;
	if (async) {
		hr = mThreadPump->AddWorkItem(ret->AsRes(), [&](ID3DX11ThreadPump* pump, ThreadPumpEntryPtr entry)->HRESULT {
			const D3D10_SHADER_MACRO* pDefines = nullptr;
			LPD3D10INCLUDE pInclude = new IncludeStdIo("shader\\");
			return D3DX11CompileFromFileA(filename, pDefines, pInclude, szEntry, shaderModel, dwShaderFlags, 0, pump,
				&static_cast<BlobData11*>(PtrRaw(ret->mBlob))->mBlob, &ret->mErrBlob, (HRESULT*)&entry->hr);
		}, [=](IResource* res, HRESULT hr) {
			if (!FAILED(hr)) {
				//TPixelShader11* ret = static_cast<TPixelShader11*>(res);
				assert(dynamic_cast<VertexShader11*>(res) && ret->mBlob);
				if (!CheckHR(mDevice->CreatePixelShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), NULL, &ret->mShader))) {
					res->SetLoaded();
				}
			}
			else {
				CheckCompileError(hr, ret->mErrBlob);
			}
		});
	}
	else {
		ret->mBlob = MakePtr<BlobData11>(nullptr);
		hr = D3DX11CompileFromFileA(filename, &mShaderMacros[0], NULL, szEntry, shaderModel, dwShaderFlags, 0, nullptr,
			&static_cast<BlobData11*>(PtrRaw(ret->mBlob))->mBlob, &ret->mErrBlob, NULL);
		if (CheckCompileError(hr, ret->mErrBlob)
			&& !CheckHR(mDevice->CreatePixelShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), NULL, &ret->mShader))) {
			ret->AsRes()->SetLoaded();
		}
		else {
			ret = nullptr;
		}
	}
	return ret;
}

PixelShader11Ptr RenderSystem11::_CreatePSByFXC(const char* filename)
{
	PixelShader11Ptr ret = MakePtr<PixelShader11>(nullptr);
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret->mBlob = MakePtr<BlobDataStandard>(buffer);
		HRESULT hr = mDevice->CreatePixelShader(&buffer[0], buffer.size(), NULL, &ret->mShader);
		if (!FAILED(hr)) {
			ret->AsRes()->SetLoaded();
		}
		else {
			D3DGetDebugInfo(&buffer[0], buffer.size(), &ret->mErrBlob);
			CheckCompileError(hr, ret->mErrBlob);
			ret = nullptr;
		}
	}
	else {
		ret = nullptr;
	}
	return ret;
}

VertexShader11Ptr RenderSystem11::_CreateVS(const char* filename, const char* szEntry, bool async)
{
	VertexShader11Ptr ret = MakePtr<VertexShader11>(MakePtr<BlobData11>(nullptr));
	szEntry = szEntry ? szEntry : "VS";
	const char* shaderModel = "vs_4_0";
	DWORD dwShaderFlags = GetShaderFlag();

	HRESULT hr;
	if (async) {
		ID3DX11DataProcessor* pProcessor = nullptr;
		ID3DX11DataLoader* pDataLoader = nullptr;
		const D3D10_SHADER_MACRO* pDefines = nullptr;
		LPD3D10INCLUDE pInclude = new IncludeStdIo("shader\\");
		if (!CheckHR(D3DX11CreateAsyncCompilerProcessor(filename, pDefines, pInclude, szEntry, shaderModel, dwShaderFlags, 0,
			&std::static_pointer_cast<BlobData11>(ret->mBlob)->mBlob, &ret->mErrBlob, &pProcessor))
			&& !CheckHR(D3DX11CreateAsyncFileLoaderA(filename, &pDataLoader))) {
			hr = mThreadPump->AddWorkItem(ret->AsRes(), pDataLoader, pProcessor, [=](IResource* res, HRESULT hr) {
				if (!FAILED(hr))  {
					//TVertexShader11* ret = static_cast<TVertexShader11*>(res);
					assert(dynamic_cast<VertexShader11*>(res) && ret->mBlob);
					if (!CheckHR(mDevice->CreateVertexShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), NULL, &ret->mShader))) {
						res->SetLoaded();
					}
				}
				else {
					CheckCompileError(hr, ret->mErrBlob);
				}
			});
			CheckCompileError(hr, ret->mErrBlob);
		}
	}
	else {
		hr = D3DX11CompileFromFileA(filename, &mShaderMacros[0], NULL, szEntry, shaderModel, dwShaderFlags, 0, nullptr,
			&std::static_pointer_cast<BlobData11>(ret->mBlob)->mBlob, &ret->mErrBlob, NULL);
		if (CheckCompileError(hr, ret->mErrBlob)
			&& !CheckHR(mDevice->CreateVertexShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), NULL, &ret->mShader))) {
			ret->AsRes()->SetLoaded();
		}
		else {
			ret = nullptr;
		}
	}
	return ret;
}

VertexShader11Ptr RenderSystem11::_CreateVSByFXC(const char* filename)
{
	VertexShader11Ptr ret = MakePtr<VertexShader11>(nullptr);
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret->mBlob = MakePtr<BlobDataStandard>(buffer);
		auto buffer_size = buffer.size();
		HRESULT hr = mDevice->CreateVertexShader(&buffer[0], buffer_size, NULL, &ret->mShader);
		if (!FAILED(hr)) {
			ret->AsRes()->SetLoaded();
		}
		else {
			D3DGetDebugInfo(&buffer[0], buffer.size(), &ret->mErrBlob);
			CheckCompileError(hr, ret->mErrBlob);
			ret = nullptr;
		}
	}
	else {
		ret = nullptr;
	}
	return ret;
}

IProgramPtr RenderSystem11::CreateProgramByCompile(const char* vsPath, const char* psPath, const char* vsEntry, const char* psEntry)
{
	TIME_PROFILE2(CreateProgramByCompile, std::string(vsPath));
	psPath = psPath ? psPath : vsPath;
	Program11Ptr program = MakePtr<Program11>();
	program->SetVertex(_CreateVS(vsPath, vsEntry, false));
	program->SetPixel(_CreatePS(psPath, psEntry, false));
	program->AsRes()->CheckAndSetLoaded();
	return program;
}

IProgramPtr RenderSystem11::CreateProgramByFXC(const std::string& name, const char* vsEntry, const char* psEntry)
{
	TIME_PROFILE2(CreateProgramByFXC, (name));
	Program11Ptr program = MakePtr<Program11>();

	vsEntry = vsEntry ? vsEntry : "VS";
	std::string vsName = (name)+"_" + vsEntry + FILE_EXT_CSO;
	program->SetVertex(_CreateVSByFXC(vsName.c_str()));

	psEntry = psEntry ? psEntry : "PS";
	std::string psName = (name)+"_" + psEntry + FILE_EXT_CSO;
	program->SetPixel(_CreatePSByFXC(psName.c_str()));

	program->AsRes()->CheckAndSetLoaded();
	return program;
}

ID3D11Buffer* RenderSystem11::_CreateVertexBuffer(int bufferSize, void* buffer)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = bufferSize;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = buffer;

	ID3D11Buffer* pVertexBuffer = nullptr;
	hr = mDevice->CreateBuffer(&bd, &InitData, &pVertexBuffer);
	if (CheckHR(hr))
		return nullptr;
	return pVertexBuffer;
}

ID3D11Buffer* RenderSystem11::_CreateVertexBuffer(int bufferSize)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = bufferSize;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Buffer* pVertexBuffer = nullptr;
	hr = mDevice->CreateBuffer(&bd, nullptr, &pVertexBuffer);
	if (CheckHR(hr))
		return nullptr;
	return pVertexBuffer;
}

IVertexBufferPtr RenderSystem11::CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer/*=nullptr*/)
{
	IVertexBufferPtr vertexBuffer;
	if (buffer) {
		vertexBuffer = MakePtr<VertexBuffer11>(_CreateVertexBuffer(bufferSize, buffer), bufferSize, stride, offset);
	}
	else {
		vertexBuffer = MakePtr<VertexBuffer11>(_CreateVertexBuffer(bufferSize), bufferSize, stride, offset);
	}
	return vertexBuffer;
}

void RenderSystem11::SetVertexBuffer(IVertexBufferPtr vertexBuffer)
{
	UINT stride = vertexBuffer->GetStride();
	UINT offset = vertexBuffer->GetOffset();
	mDeviceContext->IASetVertexBuffers(0, 1, &std::static_pointer_cast<VertexBuffer11>(vertexBuffer)->GetBuffer11(), &stride, &offset);
}

IIndexBufferPtr RenderSystem11::CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = bufferSize;// sizeof(WORD) * Indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = buffer;

	ID3D11Buffer* pIndexBuffer = nullptr;
	hr = mDevice->CreateBuffer(&bd, &InitData, &pIndexBuffer);
	if (CheckHR(hr))
		return nullptr;

	IIndexBufferPtr indexBuffer = MakePtr<IndexBuffer11>(pIndexBuffer, bufferSize, format);
	return indexBuffer;
}

void RenderSystem11::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	if (indexBuffer) {
		mDeviceContext->IASetIndexBuffer(std::static_pointer_cast<IndexBuffer11>(indexBuffer)->GetBuffer11(), indexBuffer->GetFormat(), 0);
	}
	else {
		mDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	}
}

IContantBufferPtr RenderSystem11::CreateConstBuffer(const ConstBufferDecl& cbDecl, void* data)
{
	HRESULT hr = S_OK;

	ID3D11Buffer* pConstantBuffer = nullptr;
	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = cbDecl.BufferSize % sizeof(XMFLOAT4) == 0 ? cbDecl.BufferSize : (cbDecl.BufferSize / sizeof(XMFLOAT4) + 1) * sizeof(XMFLOAT4);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = mDevice->CreateBuffer(&bd, NULL, &pConstantBuffer);
	if (CheckHR(hr))
		return nullptr;
	ConstBufferDeclPtr declPtr = std::make_shared<ConstBufferDecl>(cbDecl);
	IContantBufferPtr ret = MakePtr<ContantBuffer11>(pConstantBuffer, declPtr);

	if (data) UpdateConstBuffer(ret, data, ret->GetBufferSize());
	return ret;
}

IContantBufferPtr RenderSystem11::CloneConstBuffer(IContantBufferPtr buffer)
{
	return CreateConstBuffer(*buffer->GetDecl());
}

void RenderSystem11::UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize)
{
	mDeviceContext->UpdateSubresource(std::static_pointer_cast<ContantBuffer11>(buffer)->GetBuffer11(), 0, NULL, data, 0, 0);
}

ITexturePtr RenderSystem11::_CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube)
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

	ITexturePtr pTextureRV;
	if (IsFileExist(pSrcFile))
	{
		D3DX11_IMAGE_LOAD_INFO LoadInfo = {};
		LoadInfo.Format = format;
		D3DX11_IMAGE_LOAD_INFO* pLoadInfo = format != DXGI_FORMAT_UNKNOWN ? &LoadInfo : nullptr;

		pTextureRV = MakePtr<Texture11>(nullptr, imgPath);
		IResourcePtr resource = pTextureRV->AsRes();
		HRESULT hr;
		if (async) {
			hr = mThreadPump->AddWorkItem(resource, [&](ID3DX11ThreadPump* pump, ThreadPumpEntryPtr entry)->HRESULT {
				return D3DX11CreateShaderResourceViewFromFileA(mDevice, pSrcFile, pLoadInfo, pump, 
					&std::static_pointer_cast<Texture11>(pTextureRV)->GetSRV11(), NULL);
			}, ResourceSetLoaded);
		}
		else {
			hr = D3DX11CreateShaderResourceViewFromFileA(mDevice, pSrcFile, pLoadInfo, nullptr, 
				&std::static_pointer_cast<Texture11>(pTextureRV)->GetSRV11(), NULL);
			resource->SetLoaded();
		}
		if (CheckHR(hr)) {
			pTextureRV = nullptr;
		}
	}
	else {
		char szBuf[260]; sprintf(szBuf, "image file %s not exist\n", pSrcFile);
		OutputDebugStringA(szBuf);
		MessageBoxA(0, szBuf, "", MB_OK);
		pTextureRV = nullptr;
	}
	return pTextureRV;
}

ITexturePtr RenderSystem11::CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap)
{
	assert(mipmap > 0);
	return MakePtr<Texture11>(width, height, format, mipmap);
}
bool RenderSystem11::LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep)
{
	assert(dataStep * texture->GetHeight() <= dataSize);
	D3D11_SUBRESOURCE_DATA initData = { data, dataStep, 0 };

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = texture->GetWidth();
	desc.Height = texture->GetHeight();
	desc.MipLevels = desc.ArraySize = texture->GetMipmapCount();
	desc.Format = texture->GetFormat();
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> tex;
	HRESULT hr = mDevice->CreateTexture2D(&desc, &initData, tex.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = texture->GetFormat();
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = texture->GetMipmapCount();

		ID3D11ShaderResourceView* texSRV;
		hr = mDevice->CreateShaderResourceView(tex.Get(), &SRVDesc, &texSRV);
		if (SUCCEEDED(hr)) 
		{
			Texture11Ptr tex11 = std::static_pointer_cast<Texture11>(texture);
			tex11->SetSRV11(texSRV);

			tex11->AsRes()->CheckAndSetLoaded();
		}
	}

	return SUCCEEDED(hr);
}

void RenderSystem11::SetBlendFunc(const BlendFunc& blendFunc)
{
	mCurBlendFunc = blendFunc;

	D3D11_BLEND_DESC blendDesc = { 0 };
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = blendFunc.Src;
	blendDesc.RenderTarget[0].DestBlend = blendFunc.Dst;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (CheckHR(mDevice->CreateBlendState(&blendDesc, &mBlendState)))
		return;

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	mDeviceContext->OMSetBlendState(mBlendState, blendFactor, 0xffffffff);
}

void RenderSystem11::SetDepthState(const DepthState& depthState)
{
	mCurDepthState = depthState;

	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory(&DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	DSDesc.DepthEnable = depthState.DepthEnable;
	DSDesc.DepthWriteMask = depthState.DepthWriteMask;
	DSDesc.DepthFunc = depthState.DepthFunc;
	DSDesc.StencilEnable = FALSE;
	if (CheckHR(mDevice->CreateDepthStencilState(&DSDesc, &mDepthStencilState)))
		return;
	mDeviceContext->OMSetDepthStencilState(mDepthStencilState, 1);
}

static std::vector<ID3D11Buffer*> GetConstBuffer11List(const std::vector<CBufferEntry>& bufferInfos) {
	std::vector<ID3D11Buffer*> ret;
	for (auto& iter : bufferInfos)
		ret.push_back(std::static_pointer_cast<ContantBuffer11>(iter.Buffer)->GetBuffer11());
	return ret;
}
static std::vector<ID3D11SamplerState*> GetSampler11List(const std::vector<ISamplerStatePtr>& samplers) {
	std::vector<ID3D11SamplerState*> ret;
	for (auto& iter : samplers)
		ret.push_back(std::static_pointer_cast<SamplerState11>(iter)->GetSampler11());
	return ret;
}

void RenderSystem11::BindPass(const PassPtr& pass, const cbGlobalParam& globalParam)
{
	std::vector<ID3D11Buffer*> passConstBuffers = GetConstBuffer11List(pass->mConstantBuffers);
	mDeviceContext->UpdateSubresource(passConstBuffers[0], 0, NULL, &globalParam, 0, 0);

	mDeviceContext->VSSetShader(std::static_pointer_cast<VertexShader11>(pass->mProgram->GetVertex())->GetShader11(), NULL, 0);
	mDeviceContext->PSSetShader(std::static_pointer_cast<PixelShader11>(pass->mProgram->GetPixel())->GetShader11(), NULL, 0);

	mDeviceContext->VSSetConstantBuffers(0, passConstBuffers.size(), &passConstBuffers[0]);
	mDeviceContext->PSSetConstantBuffers(0, passConstBuffers.size(), &passConstBuffers[0]);
	mDeviceContext->IASetInputLayout(std::static_pointer_cast<InputLayout11>(pass->mInputLayout)->GetLayout11());

	mDeviceContext->IASetPrimitiveTopology(pass->mTopoLogy);

	if (!pass->mSamplers.empty()) {
		std::vector<ID3D11SamplerState*> passSamplers = GetSampler11List(pass->mSamplers);
		mDeviceContext->PSSetSamplers(0, passSamplers.size(), &passSamplers[0]);
	}
}

std::vector<ID3D11ShaderResourceView*> GetTextureViews11(std::vector<ITexturePtr>& textures) {
	std::vector<ID3D11ShaderResourceView*> views(textures.size());
	for (int i = 0; i < views.size(); ++i) {
		auto iTex = std::static_pointer_cast<Texture11>(textures[i]);
		if (iTex != nullptr) {
			views[i] = iTex->GetSRV11();
		}
	}
	return views;
}

void RenderSystem11::RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op, const cbGlobalParam& globalParam)
{
	if (iterCnt >= 0) {
		_PushRenderTarget(pass->mIterTargets[iterCnt]);
	}
	else {
		if (pass->mRenderTarget)
			_PushRenderTarget(pass->mRenderTarget);
	}

	if (iterCnt >= 0) {
		if (iterCnt + 1 < pass->mIterTargets.size())
			textures[0] = pass->mIterTargets[iterCnt + 1]->GetColorTexture();
	}
	else {
		if (!pass->mIterTargets.empty())
			textures[0] = pass->mIterTargets[0]->GetColorTexture();
	}

	{
		if (textures.Count() > 0) {
			std::vector<ID3D11ShaderResourceView*> texViews = GetTextureViews11(textures.Textures);
			mDeviceContext->PSSetShaderResources(0, texViews.size(), &texViews[0]);
		}

		if (pass->OnBind)
			pass->OnBind(*pass, *this, textures);

		BindPass(pass, globalParam);

		if (op.mIndexBuffer) {
			//if (_CanDraw())
			int indexCount = op.mIndexCount != 0 ? op.mIndexCount : op.mIndexBuffer->GetBufferSize() / op.mIndexBuffer->GetWidth();
			mDeviceContext->DrawIndexed(indexCount, op.mIndexPos, op.mIndexBase);
		}
		else {
			//if (_CanDraw())
			mDeviceContext->Draw(op.mVertexBuffer->GetBufferSize() / op.mVertexBuffer->GetStride(), 0);
		}

		if (pass->OnUnbind)
			pass->OnUnbind(*pass, *this, textures);
	}

	if (iterCnt >= 0) {
		_PopRenderTarget();
	}
	else {
		if (pass->mRenderTarget)
			_PopRenderTarget();
	}
}

void RenderSystem11::RenderOp(const RenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam)
{
	TechniquePtr tech = op.mMaterial->CurTech();
	std::vector<PassPtr> passes = tech->GetPassesByLightMode(lightMode);
	for (auto& pass : passes)
	{
		SetVertexBuffer(op.mVertexBuffer);
		SetIndexBuffer(op.mIndexBuffer);

		TextureBySlot textures = op.mTextures;
		textures.Merge(pass->mTextures);

		for (int i = pass->mIterTargets.size() - 1; i >= 0; --i) {
			auto iter = op.mVertBufferByPass.find(std::make_pair(pass, i));
			if (iter != op.mVertBufferByPass.end()) {
				SetVertexBuffer(iter->second);
			}
			else {
				SetVertexBuffer(op.mVertexBuffer);
			}
			ITexturePtr first = !textures.Empty() ? textures[0] : nullptr;
			RenderPass(pass, textures, i, op, globalParam);
			textures[0] = first;
		}
		auto iter = op.mVertBufferByPass.find(std::make_pair(pass, -1));
		if (iter != op.mVertBufferByPass.end()) {
			SetVertexBuffer(iter->second);
		}
		else {
			SetVertexBuffer(op.mVertexBuffer);
		}
		RenderPass(pass, textures, -1, op, globalParam);
	}
}

void RenderSystem11::RenderLight(cbDirectLight* light, LightType lightType, const RenderOperationQueue& opQueue, const std::string& lightMode)
{
	auto LightCam = light->GetLightCamera(*mSceneManager->mDefCamera);
	cbGlobalParam globalParam;
	MakeAutoParam(globalParam, LightCam, lightMode == E_PASS_SHADOWCASTER, light, lightType);
	for (int i = 0; i < opQueue.Count(); ++i)
		if (opQueue[i].mMaterial->IsLoaded()) {
			globalParam.World = opQueue[i].mWorldTransform;
			globalParam.WorldInv = XM::Inverse(globalParam.World);
			RenderOp(opQueue[i], lightMode, globalParam);
		}
}

void RenderSystem11::RenderQueue(const RenderOperationQueue& opQueue, const std::string& lightMode)
{
	mDrawCount = 0;
	DepthState orgState = mCurDepthState;
	BlendFunc orgBlend = mCurBlendFunc;

	if (lightMode == E_PASS_SHADOWCASTER) {
		_PushRenderTarget(mShadowPassRT);
		ClearColorDepthStencil(XMFLOAT4(1, 1, 1, 1), 1.0, 0);
		SetDepthState(DepthState(false));
		SetBlendFunc(BlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_ZERO));
		mCastShdowFlag = true;
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		ID3D11ShaderResourceView* depthMapView = std::static_pointer_cast<Texture11>(mShadowPassRT->GetColorTexture())->GetSRV11();
		mDeviceContext->PSSetShaderResources(E_TEXTURE_DEPTH_MAP, 1, &depthMapView);
		auto skybox = mSceneManager->mSkyBox;
		if (skybox && skybox->mCubeSRV) {
			auto texture = std::static_pointer_cast<Texture11>(skybox->mCubeSRV)->GetSRV11();
			mDeviceContext->PSSetShaderResources(E_TEXTURE_ENV, 1, &texture);
		}
	}
	else if (lightMode == E_PASS_POSTPROCESS) {
		ID3D11ShaderResourceView* pSRV = std::static_pointer_cast<Texture11>(mPostProcessRT->GetColorTexture())->GetSRV11();
		mDeviceContext->PSSetShaderResources(0, 1, &pSRV);
	}

	auto& lightsOrder = mSceneManager->mLightsByOrder;
	if (!lightsOrder.empty()) {
		BlendFunc orgBlend = mCurBlendFunc;
		SetBlendFunc(BlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA));
		RenderLight(lightsOrder[0].first, lightsOrder[0].second, opQueue, lightMode);

		for (int i = 1; i < lightsOrder.size(); ++i) {
			auto order = lightsOrder[i];
			SetBlendFunc(BlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE));
			auto __lightMode = (lightMode == E_PASS_FORWARDBASE) ? E_PASS_FORWARDADD : lightMode;
			RenderLight(order.first, order.second, opQueue, __lightMode);
		}
		SetBlendFunc(orgBlend);
	}

	if (lightMode == E_PASS_SHADOWCASTER) {
		_PopRenderTarget();
		SetDepthState(orgState);
		SetBlendFunc(orgBlend);
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		ID3D11ShaderResourceView* texViewNull = nullptr;
		mDeviceContext->PSSetShaderResources(E_TEXTURE_DEPTH_MAP, 1, &texViewNull);
	}
}

void RenderSystem11::_RenderSkyBox()
{
	if (mSceneManager->mSkyBox) mSceneManager->mSkyBox->Draw();
}

void RenderSystem11::_DoPostProcess()
{
	DepthState orgState = mCurDepthState;
	SetDepthState(DepthState(false));

	RenderOperationQueue opQue;
	for (size_t i = 0; i < mSceneManager->mPostProcs.size(); ++i)
		mSceneManager->mPostProcs[i]->GenRenderOperation(opQue);
	RenderQueue(opQue, E_PASS_POSTPROCESS);

	SetDepthState(orgState);
}

bool RenderSystem11::BeginScene()
{
	mCastShdowFlag = false;

	if (!mSceneManager->mPostProcs.empty()) {
		SetRenderTarget(mPostProcessRT);
		ClearColorDepthStencil(XMFLOAT4(0, 0, 0, 0), 1.0, 0);
	}
	_RenderSkyBox();
	return true;
}

void RenderSystem11::EndScene()
{
	if (!mSceneManager->mPostProcs.empty()) {
		SetRenderTarget(nullptr);
	}
	_DoPostProcess();
	mSwapChain->Present(0, 0);
}

}