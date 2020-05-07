#include "TRenderSystem11.h"
#include "TMaterial.h"
#include "Utility.h"
#include "TSkyBox.h"
#include "TPostProcess.h"
#include "TThreadPump.h"
#include "TInterfaceType11.h"
#include "TMaterialCB.h"

TRenderSystem11::TRenderSystem11()
{
	mMaterialFac = std::make_shared<TMaterialFactory>(this);
	mThreadPump = std::make_shared<TThreadPump>();
	mFXCDir = "d3d11\\";
}

TRenderSystem11::~TRenderSystem11()
{
}

bool TRenderSystem11::Initialize(HWND hWnd)
{
	mHWnd = hWnd;
	RECT rc;
	GetClientRect(mHWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	if (CheckHR(_CreateDeviceAndSwapChain(width, height))) return false;

	if (CheckHR(_CreateBackRenderTargetView())) return false;
	if (CheckHR(_CreateBackDepthStencilView(width, height))) return false;
	mDeviceContext->OMSetRenderTargets(1, &mBackRenderTargetView, mBackDepthStencilView);

	_SetViewports(width, height);

	if (CheckHR(_SetRasterizerState())) return false;

	SetDepthState(TDepthState(TRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL));
	SetBlendFunc(TBlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA));

	mScreenWidth = width;
	mScreenHeight = height;
	mDefCamera = TCamera::CreatePerspective(mScreenWidth, mScreenHeight);

	mShadowPassRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R32_FLOAT);
	SET_DEBUG_NAME(mShadowPassRT->mDepthStencilView, "mShadowPassRT");

	mPostProcessRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R16G16B16A16_UNORM);// , DXGI_FORMAT_R8G8B8A8_UNORM);
	SET_DEBUG_NAME(mPostProcessRT->mDepthStencilView, "mPostProcessRT");

	D3D_SHADER_MACRO Shader_Macros[] = { "SHADER_MODEL", "40000", NULL, NULL };
	mShaderMacros.assign(Shader_Macros, Shader_Macros+ARRAYSIZE(Shader_Macros));
	return true;
}

HRESULT TRenderSystem11::_CreateDeviceAndSwapChain(int width, int height)
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

HRESULT TRenderSystem11::_CreateBackRenderTargetView()
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

HRESULT TRenderSystem11::_CreateBackDepthStencilView(int width, int height)
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

void TRenderSystem11::_SetViewports(int width, int height)
{
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	mDeviceContext->RSSetViewports(1, &vp);
}

HRESULT TRenderSystem11::_SetRasterizerState()
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

void TRenderSystem11::Update(float dt)
{
	mThreadPump->Update(dt);
}

void TRenderSystem11::CleanUp()
{
}

void TRenderSystem11::ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil)
{
	float colorArr[4] = { color.x, color.y, color.z, color.w }; // red, green, blue, alpha
	mDeviceContext->ClearRenderTargetView(mCurRenderTargetView, colorArr);
	mDeviceContext->ClearDepthStencilView(mCurDepthStencilView, D3D11_CLEAR_DEPTH, Depth, Stencil);
}

IRenderTexturePtr TRenderSystem11::CreateRenderTexture(int width, int height, DXGI_FORMAT format)
{
	return ComPtr<TRenderTexture11>(new TRenderTexture11(mDevice, width, height, format));
}

void TRenderSystem11::_ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color, FLOAT Depth/* = 1.0*/, UINT8 Stencil/* = 0*/)
{
	auto target11 = PtrCast(rendTarget).As<TRenderTexture11>();
	mDeviceContext->ClearRenderTargetView(target11->GetColorBuffer11(), (const float*)&color);
	mDeviceContext->ClearDepthStencilView(target11->GetDepthStencilBuffer11(), D3D11_CLEAR_DEPTH, Depth, Stencil);
}

void TRenderSystem11::SetRenderTarget(IRenderTexturePtr rendTarget)
{
	ID3D11ShaderResourceView* TextureNull = nullptr;
	mDeviceContext->PSSetShaderResources(0, 1, &TextureNull);

	auto target11 = PtrCast(rendTarget).As<TRenderTexture11>();
	mCurRenderTargetView = target11 != nullptr ? target11->GetColorBuffer11() : mBackRenderTargetView;
	mCurDepthStencilView = target11 != nullptr ? target11->GetDepthStencilBuffer11() : mBackDepthStencilView;
	mDeviceContext->OMSetRenderTargets(1, &mCurRenderTargetView, mCurDepthStencilView);
}

TMaterialPtr TRenderSystem11::CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback)
{
	return mMaterialFac->GetMaterial(name, callback);
}

ID3D11InputLayout* TRenderSystem11::_CreateInputLayout(TProgram11* pProgram, const std::vector<D3D11_INPUT_ELEMENT_DESC>& descArr)
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
IInputLayoutPtr TRenderSystem11::CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	TInputLayout11Ptr ret = new TInputLayout11;
	ret->mInputDescs.assign(descArray, descArray + descCount);

	auto resource = pProgram->AsRes();
	if (resource->IsLoaded()) {
		ret->mLayout = _CreateInputLayout(PtrCast(pProgram).As<TProgram11>(), ret->mInputDescs);
		resource->SetLoaded();
	}
	else {
		resource->AddOnLoadedListener([=](IResource* res) {
			ret->mLayout = _CreateInputLayout(PtrCast(pProgram).As<TProgram11>(), ret->mInputDescs);
			res->SetLoaded();
		});
	}
	return ret;
}

bool TRenderSystem11::UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize)
{
	assert(buffer != nullptr);
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	switch (buffer->GetType())
	{
	case E_HWBUFFER_CONSTANT: {
		IContantBufferPtr cbuffer = PtrCast(buffer).Cast<IContantBuffer>();
		TContantBuffer11Ptr cbuffer11 = PtrCast(cbuffer).As<TContantBuffer11>();
		hr = (mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		if (CheckHR(hr)) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	case E_HWBUFFER_VERTEX:{
		IVertexBufferPtr cbuffer = PtrCast(buffer).Cast<IVertexBuffer>();
		TVertexBuffer11Ptr cbuffer11 = PtrCast(cbuffer).As<TVertexBuffer11>();
		hr = (mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		if (CheckHR(hr)) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	case E_HWBUFFER_INDEX: {
		IIndexBufferPtr cbuffer = PtrCast(buffer).Cast<IIndexBuffer>();
		TIndexBuffer11Ptr cbuffer11 = PtrCast(cbuffer).As<TIndexBuffer11>();
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

ISamplerStatePtr TRenderSystem11::CreateSampler(D3D11_FILTER filter, D3D11_COMPARISON_FUNC comp)
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

	TSamplerState11Ptr ret = new TSamplerState11(pSamplerLinear);
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
TPixelShader11Ptr TRenderSystem11::_CreatePS(const char* filename, const char* szEntry, bool async)
{
	TPixelShader11Ptr ret = new TPixelShader11(ComPtr<TBlobDataD3d11>(new TBlobDataD3d11(nullptr)));
	szEntry = szEntry ? szEntry : "PS";
	const char* shaderModel = "ps_4_0";
	DWORD dwShaderFlags = GetShaderFlag();

	HRESULT hr;
	if (async) {
		hr = mThreadPump->AddWorkItem(ret->AsRes(), [&](ID3DX11ThreadPump* pump, TThreadPumpEntryPtr entry)->HRESULT {
			const D3D10_SHADER_MACRO* pDefines = nullptr;
			LPD3D10INCLUDE pInclude = new TIncludeStdio("shader\\");
			return D3DX11CompileFromFileA(filename, pDefines, pInclude, szEntry, shaderModel, dwShaderFlags, 0, pump,
				&static_cast<TBlobDataD3d11*>(ret->mBlob.Get())->mBlob, &ret->mErrBlob, (HRESULT*)&entry->hr);
		}, [=](IResource* res, HRESULT hr) {
			if (!FAILED(hr)) {
				//TPixelShader11* ret = static_cast<TPixelShader11*>(res);
				assert(dynamic_cast<TVertexShader11*>(res) && ret->mBlob);
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
		ret->mBlob = ComPtr<IBlobData>(new TBlobDataD3d11(nullptr));
		hr = D3DX11CompileFromFileA(filename, &mShaderMacros[0], NULL, szEntry, shaderModel, dwShaderFlags, 0, nullptr,
			&static_cast<TBlobDataD3d11*>(ret->mBlob.Get())->mBlob, &ret->mErrBlob, NULL);
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

TPixelShader11Ptr TRenderSystem11::_CreatePSByFXC(const char* filename)
{
	TPixelShader11Ptr ret = MakePtr<TPixelShader11>(nullptr);
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret->mBlob = ComPtr<IBlobData>(new TBlobDataStd(buffer));
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

TVertexShader11Ptr TRenderSystem11::_CreateVS(const char* filename, const char* szEntry, bool async)
{
	TVertexShader11Ptr ret = MakePtr<TVertexShader11>(MakePtr<TBlobDataD3d11>(nullptr));
	szEntry = szEntry ? szEntry : "VS";
	const char* shaderModel = "vs_4_0";
	DWORD dwShaderFlags = GetShaderFlag();

	HRESULT hr;
	if (async) {
		ID3DX11DataProcessor* pProcessor = nullptr;
		ID3DX11DataLoader* pDataLoader = nullptr;
		const D3D10_SHADER_MACRO* pDefines = nullptr;
		LPD3D10INCLUDE pInclude = new TIncludeStdio("shader\\");
		if (!CheckHR(D3DX11CreateAsyncCompilerProcessor(filename, pDefines, pInclude, szEntry, shaderModel, dwShaderFlags, 0,
			&PtrCast(ret->mBlob).As<TBlobDataD3d11>()->mBlob, &ret->mErrBlob, &pProcessor))
			&& !CheckHR(D3DX11CreateAsyncFileLoaderA(filename, &pDataLoader))) {
			hr = mThreadPump->AddWorkItem(ret->AsRes(), pDataLoader, pProcessor, [=](IResource* res, HRESULT hr) {
				if (!FAILED(hr))  {
					//TVertexShader11* ret = static_cast<TVertexShader11*>(res);
					assert(dynamic_cast<TVertexShader11*>(res) && ret->mBlob);
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
			&PtrCast(ret->mBlob).As<TBlobDataD3d11>()->mBlob, &ret->mErrBlob, NULL);
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

TVertexShader11Ptr TRenderSystem11::_CreateVSByFXC(const char* filename)
{
	TVertexShader11Ptr ret = MakePtr<TVertexShader11>(nullptr);
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret->mBlob = ComPtr<IBlobData>(new TBlobDataStd(buffer));
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

IProgramPtr TRenderSystem11::CreateProgramByCompile(const char* vsPath, const char* psPath, const char* vsEntry, const char* psEntry)
{
	TIME_PROFILE2(CreateProgramByCompile, std::string(vsPath));
	psPath = psPath ? psPath : vsPath;
	TProgram11Ptr program = MakePtr<TProgram11>();
	program->SetVertex(_CreateVS(vsPath, vsEntry, false));
	program->SetPixel(_CreatePS(psPath, psEntry, false));
	program->AsRes()->CheckAndSetLoaded();
	return program;
}

IProgramPtr TRenderSystem11::CreateProgramByFXC(const std::string& name, const char* vsEntry, const char* psEntry)
{
	TIME_PROFILE2(CreateProgramByFXC, (name));
	TProgram11Ptr program = MakePtr<TProgram11>();

	vsEntry = vsEntry ? vsEntry : "VS";
	std::string vsName = (name)+"_" + vsEntry + FILE_EXT_CSO;
	program->SetVertex(_CreateVSByFXC(vsName.c_str()));

	psEntry = psEntry ? psEntry : "PS";
	std::string psName = (name)+"_" + psEntry + FILE_EXT_CSO;
	program->SetPixel(_CreatePSByFXC(psName.c_str()));

	program->AsRes()->CheckAndSetLoaded();
	return program;
}

ID3D11Buffer* TRenderSystem11::_CreateVertexBuffer(int bufferSize, void* buffer)
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

ID3D11Buffer* TRenderSystem11::_CreateVertexBuffer(int bufferSize)
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

IVertexBufferPtr TRenderSystem11::CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer/*=nullptr*/)
{
	IVertexBufferPtr vertexBuffer;
	if (buffer) {
		vertexBuffer = MakePtr<TVertexBuffer11>(_CreateVertexBuffer(bufferSize, buffer), bufferSize, stride, offset);
	}
	else {
		vertexBuffer = MakePtr<TVertexBuffer11>(_CreateVertexBuffer(bufferSize), bufferSize, stride, offset);
	}
	return vertexBuffer;
}

void TRenderSystem11::SetVertexBuffer(IVertexBufferPtr vertexBuffer)
{
	UINT stride = vertexBuffer->GetStride();
	UINT offset = vertexBuffer->GetOffset();
	mDeviceContext->IASetVertexBuffers(0, 1, &PtrCast(vertexBuffer).As<TVertexBuffer11>()->GetBuffer11(), &stride, &offset);
}

IIndexBufferPtr TRenderSystem11::CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = bufferSize;// sizeof(WORD) * Indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = buffer;

	ID3D11Buffer* pIndexBuffer = nullptr;
	hr = mDevice->CreateBuffer(&bd, &InitData, &pIndexBuffer);
	if (CheckHR(hr))
		return nullptr;

	IIndexBufferPtr indexBuffer = MakePtr<TIndexBuffer11>(pIndexBuffer, bufferSize, format);
	return indexBuffer;
}

void TRenderSystem11::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	if (indexBuffer) {
		mDeviceContext->IASetIndexBuffer(PtrCast(indexBuffer).As<TIndexBuffer11>()->GetBuffer11(), indexBuffer->GetFormat(), 0);
	}
	else {
		mDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	}
}

IContantBufferPtr TRenderSystem11::CreateConstBuffer(const TConstBufferDecl& cbDecl, void* data)
{
	HRESULT hr = S_OK;

	ID3D11Buffer* pConstantBuffer = nullptr;
	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = cbDecl.bufferSize % sizeof(XMFLOAT4) == 0 ? cbDecl.bufferSize : (cbDecl.bufferSize / sizeof(XMFLOAT4) + 1) * sizeof(XMFLOAT4);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = mDevice->CreateBuffer(&bd, NULL, &pConstantBuffer);
	if (CheckHR(hr))
		return nullptr;
	TConstBufferDeclPtr declPtr = std::make_shared<TConstBufferDecl>(cbDecl);
	IContantBufferPtr ret = MakePtr<TContantBuffer11>(pConstantBuffer, declPtr);

	if (data) UpdateConstBuffer(ret, data, ret->GetBufferSize());
	return ret;
}

IContantBufferPtr TRenderSystem11::CloneConstBuffer(IContantBufferPtr buffer)
{
	return CreateConstBuffer(*buffer->GetDecl());
}

void TRenderSystem11::UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize)
{
	mDeviceContext->UpdateSubresource(PtrCast(buffer).As<TContantBuffer11>()->GetBuffer11(), 0, NULL, data, 0, 0);
}

ITexturePtr TRenderSystem11::_CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube)
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

		pTextureRV = MakePtr<TTexture11>(nullptr, imgPath);
		IResourcePtr resource = pTextureRV->AsRes();
		HRESULT hr;
		if (async) {
			hr = mThreadPump->AddWorkItem(resource, [&](ID3DX11ThreadPump* pump, TThreadPumpEntryPtr entry)->HRESULT {
				return D3DX11CreateShaderResourceViewFromFileA(mDevice, pSrcFile, pLoadInfo, pump, &PtrCast(pTextureRV).As<TTexture11>()->GetSRV11(), NULL);
			}, ResourceSetLoaded);
		}
		else {
			hr = D3DX11CreateShaderResourceViewFromFileA(mDevice, pSrcFile, pLoadInfo, nullptr, &PtrCast(pTextureRV).As<TTexture11>()->GetSRV11(), NULL);
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

ITexturePtr TRenderSystem11::CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap)
{
	assert(mipmap > 0);
	return MakePtr<TTexture11>(width, height, format, mipmap);
}
bool TRenderSystem11::LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep)
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
			TTexture11Ptr tex11 = PtrCast(texture).As<TTexture11>();
			tex11->SetSRV11(texSRV);

			tex11->AsRes()->CheckAndSetLoaded();
		}
	}

	return SUCCEEDED(hr);
}

void TRenderSystem11::SetBlendFunc(const TBlendFunc& blendFunc)
{
	mCurBlendFunc = blendFunc;

	D3D11_BLEND_DESC blendDesc = { 0 };
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = blendFunc.src;
	blendDesc.RenderTarget[0].DestBlend = blendFunc.dst;
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

void TRenderSystem11::SetDepthState(const TDepthState& depthState)
{
	mCurDepthState = depthState;

	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory(&DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	DSDesc.DepthEnable = depthState.depthEnable;
	DSDesc.DepthWriteMask = depthState.depthWriteMask;
	DSDesc.DepthFunc = depthState.depthFunc;
	DSDesc.StencilEnable = FALSE;
	if (CheckHR(mDevice->CreateDepthStencilState(&DSDesc, &mDepthStencilState)))
		return;
	mDeviceContext->OMSetDepthStencilState(mDepthStencilState, 1);
}

static std::vector<ID3D11Buffer*> GetConstBuffer11List(const std::vector<TContantBufferInfo>& bufferInfos) {
	std::vector<ID3D11Buffer*> ret;
	for (auto& iter : bufferInfos)
		ret.push_back(PtrCast(iter.buffer).As<TContantBuffer11>()->GetBuffer11());
	return ret;
}
static std::vector<ID3D11SamplerState*> GetSampler11List(const std::vector<ISamplerStatePtr>& samplers) {
	std::vector<ID3D11SamplerState*> ret;
	for (auto& iter : samplers)
		ret.push_back(PtrCast(iter).As<TSamplerState11>()->GetSampler11());
	return ret;
}

void TRenderSystem11::BindPass(TPassPtr pass, const cbGlobalParam& globalParam)
{
	std::vector<ID3D11Buffer*> passConstBuffers = GetConstBuffer11List(pass->mConstantBuffers);
	mDeviceContext->UpdateSubresource(passConstBuffers[0], 0, NULL, &globalParam, 0, 0);

	mDeviceContext->VSSetShader(PtrCast(pass->mProgram->GetVertex()).As<TVertexShader11>()->GetShader11(), NULL, 0);
	mDeviceContext->PSSetShader(PtrCast(pass->mProgram->GetPixel()).As<TPixelShader11>()->GetShader11(), NULL, 0);

	mDeviceContext->VSSetConstantBuffers(0, passConstBuffers.size(), &passConstBuffers[0]);
	mDeviceContext->PSSetConstantBuffers(0, passConstBuffers.size(), &passConstBuffers[0]);
	mDeviceContext->IASetInputLayout(PtrCast(pass->mInputLayout).As<TInputLayout11>()->GetLayout11());

	mDeviceContext->IASetPrimitiveTopology(pass->mTopoLogy);

	if (!pass->mSamplers.empty()) {
		std::vector<ID3D11SamplerState*> passSamplers = GetSampler11List(pass->mSamplers);
		mDeviceContext->PSSetSamplers(0, passSamplers.size(), &passSamplers[0]);
	}
}

std::vector<ID3D11ShaderResourceView*> GetTextureViews11(std::vector<ITexturePtr>& textures) {
	std::vector<ID3D11ShaderResourceView*> views(textures.size());
	for (int i = 0; i < views.size(); ++i) {
		auto iTex = PtrCast(textures[i]).As<TTexture11>();
		if (iTex != nullptr) {
			views[i] = iTex->GetSRV11();
		}
	}
	return views;
}

void TRenderSystem11::RenderPass(TPassPtr pass, TTextureBySlot& textures, int iterCnt, IIndexBufferPtr indexBuffer, IVertexBufferPtr vertexBuffer, const cbGlobalParam& globalParam)
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
		if (textures.size() > 0) {
			std::vector<ID3D11ShaderResourceView*> texViews = GetTextureViews11(textures.textures);
			mDeviceContext->PSSetShaderResources(0, texViews.size(), &texViews[0]);
		}

		if (pass->OnBind)
			pass->OnBind(pass.get(), this, textures);

		BindPass(pass, globalParam);

		if (indexBuffer) {
			if (_CanDraw())
			mDeviceContext->DrawIndexed(indexBuffer->GetBufferSize() / indexBuffer->GetWidth(), 0, 0);
		}
		else {
			if (_CanDraw())
			mDeviceContext->Draw(vertexBuffer->GetBufferSize() / vertexBuffer->GetStride(), 0);
		}

		if (pass->OnUnbind)
			pass->OnUnbind(pass.get(), this, textures);
	}

	if (iterCnt >= 0) {
		_PopRenderTarget();
	}
	else {
		if (pass->mRenderTarget)
			_PopRenderTarget();
	}
}

void TRenderSystem11::RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam)
{
	TTechniquePtr tech = op.mMaterial->CurTech();
	std::vector<TPassPtr> passes = tech->GetPassesByName(lightMode);
	for (auto& pass : passes)
	{
		SetVertexBuffer(op.mVertexBuffer);
		SetIndexBuffer(op.mIndexBuffer);

		TTextureBySlot textures = op.mTextures;
		textures.Merge(pass->mTextures);

		for (int i = pass->mIterTargets.size() - 1; i >= 0; --i) {
			auto iter = op.mVertBufferByPass.find(std::make_pair(pass, i));
			if (iter != op.mVertBufferByPass.end()) {
				SetVertexBuffer(iter->second);
			}
			else {
				SetVertexBuffer(op.mVertexBuffer);
			}
			ITexturePtr first = !textures.empty() ? textures[0] : nullptr;
			RenderPass(pass, textures, i, op.mIndexBuffer, op.mVertexBuffer, globalParam);
			textures[0] = first;
		}
		auto iter = op.mVertBufferByPass.find(std::make_pair(pass, -1));
		if (iter != op.mVertBufferByPass.end()) {
			SetVertexBuffer(iter->second);
		}
		else {
			SetVertexBuffer(op.mVertexBuffer);
		}
		RenderPass(pass, textures, -1, op.mIndexBuffer, op.mVertexBuffer, globalParam);
	}
}

void TRenderSystem11::RenderLight(TDirectLight* light, enLightType lightType, const TRenderOperationQueue& opQueue, const std::string& lightMode)
{
	auto LightCam = light->GetLightCamera(*mDefCamera);
	cbGlobalParam globalParam;
	MakeAutoParam(globalParam, &LightCam, lightMode == E_PASS_SHADOWCASTER, light, lightType);
	for (int i = 0; i < opQueue.Count(); ++i)
		if (opQueue[i].mMaterial->IsLoaded()) {
			globalParam.World = opQueue[i].mWorldTransform;
			globalParam.WorldInv = XM::Inverse(globalParam.World);
			RenderOperation(opQueue[i], lightMode, globalParam);
		}
}

void TRenderSystem11::RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode)
{
	mDrawCount = 0;
	TDepthState orgState = mCurDepthState;
	TBlendFunc orgBlend = mCurBlendFunc;

	if (lightMode == E_PASS_SHADOWCASTER) {
		_PushRenderTarget(mShadowPassRT);
		ClearColorDepthStencil(XMFLOAT4(1, 1, 1, 1), 1.0, 0);
		SetDepthState(TDepthState(false));
		SetBlendFunc(TBlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_ZERO));
		mCastShdowFlag = true;
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		ID3D11ShaderResourceView* depthMapView = PtrCast(mShadowPassRT->GetColorTexture()).As<TTexture11>()->GetSRV11();
		mDeviceContext->PSSetShaderResources(E_TEXTURE_DEPTH_MAP, 1, &depthMapView);

		if (mSkyBox && mSkyBox->mCubeSRV) {
			auto texture = PtrCast(mSkyBox->mCubeSRV).As<TTexture11>()->GetSRV11();
			mDeviceContext->PSSetShaderResources(E_TEXTURE_ENV, 1, &texture);
		}
	}
	else if (lightMode == E_PASS_POSTPROCESS) {
		ID3D11ShaderResourceView* pSRV = PtrCast(mPostProcessRT->GetColorTexture()).As<TTexture11>()->GetSRV11();
		mDeviceContext->PSSetShaderResources(0, 1, &pSRV);
	}

	if (!mLightsOrder.empty()) {
		TBlendFunc orgBlend = mCurBlendFunc;
		SetBlendFunc(TBlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA));
		RenderLight(mLightsOrder[0].first, mLightsOrder[0].second, opQueue, lightMode);

		for (int i = 1; i < mLightsOrder.size(); ++i) {
			auto order = mLightsOrder[i];
			SetBlendFunc(TBlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE));
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

void TRenderSystem11::_RenderSkyBox()
{
	if (mSkyBox) mSkyBox->Draw();
}

void TRenderSystem11::_DoPostProcess()
{
	TDepthState orgState = mCurDepthState;
	SetDepthState(TDepthState(false));

	TRenderOperationQueue opQue;
	for (size_t i = 0; i < mPostProcs.size(); ++i)
		mPostProcs[i]->GenRenderOperation(opQue);
	RenderQueue(opQue, E_PASS_POSTPROCESS);

	SetDepthState(orgState);
}

bool TRenderSystem11::BeginScene()
{
	mCastShdowFlag = false;

	if (!mPostProcs.empty()) {
		SetRenderTarget(mPostProcessRT);
		ClearColorDepthStencil(XMFLOAT4(0, 0, 0, 0), 1.0, 0);
	}
	_RenderSkyBox();
	return true;
}

void TRenderSystem11::EndScene()
{
	if (!mPostProcs.empty()) {
		SetRenderTarget(nullptr);
	}
	_DoPostProcess();
	mSwapChain->Present(0, 0);
}
