#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d11/interface_type11.h"
#include "core/rendersys/d3d11/thread_pump.h"
#include "core/rendersys/material_factory.h"
#include "core/base/utility.h"
#include <boost/assert.hpp>

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

	D3D_SHADER_MACRO Shader_Macros[] = { "SHADER_MODEL", "40000", NULL, NULL };
	mShaderMacros.assign(Shader_Macros, Shader_Macros+ARRAYSIZE(Shader_Macros));
	return true;
}

HRESULT RenderSystem11::_CreateDeviceAndSwapChain(int width, int height)
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
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
void RenderSystem11::SetVertexLayout(IInputLayoutPtr layout) {
	mDeviceContext->IASetInputLayout(std::static_pointer_cast<InputLayout11>(layout)->GetLayout11());
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
	return CreateConstBuffer(*buffer->GetDecl(), nullptr);
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

void RenderSystem11::SetProgram(IProgramPtr program)
{
	mDeviceContext->VSSetShader(std::static_pointer_cast<VertexShader11>(program->GetVertex())->GetShader11(), NULL, 0);
	mDeviceContext->PSSetShader(std::static_pointer_cast<PixelShader11>(program->GetPixel())->GetShader11(), NULL, 0);
}

void RenderSystem11::SetConstBuffers(size_t slot, IContantBufferPtr buffers[], size_t count, IProgramPtr program)
{
	std::vector<ID3D11Buffer*> passConstBuffers(count);
	for (size_t i = 0; i < count; ++i)
		passConstBuffers[i] = buffers[i] ? std::static_pointer_cast<ContantBuffer11>(buffers[i])->GetBuffer11() : nullptr;
	mDeviceContext->VSSetConstantBuffers(slot, count, &passConstBuffers[0]);
	mDeviceContext->PSSetConstantBuffers(slot, count, &passConstBuffers[0]);
}

void RenderSystem11::DrawPrimitive(const RenderOperation& op, D3D11_PRIMITIVE_TOPOLOGY topo) {
	//if (_CanDraw())
	mDeviceContext->IASetPrimitiveTopology(topo);
	mDeviceContext->Draw(op.mVertexBuffer->GetBufferSize() / op.mVertexBuffer->GetStride(), 0);
}
void RenderSystem11::DrawIndexedPrimitive(const RenderOperation& op, D3D11_PRIMITIVE_TOPOLOGY topo) {
	//if (_CanDraw())
	mDeviceContext->IASetPrimitiveTopology(topo);
	int indexCount = op.mIndexCount != 0 ? op.mIndexCount : op.mIndexBuffer->GetBufferSize() / op.mIndexBuffer->GetWidth();
	mDeviceContext->DrawIndexed(indexCount, op.mIndexPos, op.mIndexBase);
}

void RenderSystem11::SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count)
{
	BOOST_ASSERT(count > 0);
	std::vector<ID3D11SamplerState*> passSamplers(count);
	for (size_t i = 0; i < count; ++i)
		passSamplers[i] = samplers[i] ? std::static_pointer_cast<SamplerState11>(samplers[i])->GetSampler11() : nullptr;
	mDeviceContext->PSSetSamplers(0, passSamplers.size(), &passSamplers[0]);
}

void RenderSystem11::SetTexture(size_t slot, ITexturePtr texture) {
	if (texture) {
		auto texture11 = std::static_pointer_cast<Texture11>(texture);
		ID3D11ShaderResourceView* srv11 = texture11->GetSRV11();
		mDeviceContext->PSSetShaderResources(slot, 1, &srv11);
	}
	else {
		ID3D11ShaderResourceView* srv11 = nullptr;
		mDeviceContext->PSSetShaderResources(slot, 1, &srv11);
	}
}

std::vector<ID3D11ShaderResourceView*> GetTextureViews11(ITexturePtr textures[], size_t count) {
	std::vector<ID3D11ShaderResourceView*> views(count);
	for (int i = 0; i < views.size(); ++i) {
		auto iTex = std::static_pointer_cast<Texture11>(textures[i]);
		if (iTex != nullptr) views[i] = iTex->GetSRV11();
	}
	return views;
}
void RenderSystem11::SetTextures(size_t slot, ITexturePtr textures[], size_t count) {
	std::vector<ID3D11ShaderResourceView*> texViews = GetTextureViews11(textures, count);
	mDeviceContext->PSSetShaderResources(slot, texViews.size(), &texViews[0]);
}

bool RenderSystem11::BeginScene()
{
	return true;
}

void RenderSystem11::EndScene()
{
	mSwapChain->Present(0, 0);
}

}