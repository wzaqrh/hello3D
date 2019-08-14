#include "TRenderSystem11.h"
#include "TMaterial.h"
#include "Utility.h"
#include "TSkyBox.h"
#include "TPostProcess.h"
#include "TThreadPump.h"
#include "TInterfaceType11.h"

TRenderSystem11::TRenderSystem11()
{
	mMaterialFac = std::make_shared<TMaterialFactory>(this);
	mThreadPump = std::make_shared<TThreadPump>();
}

TRenderSystem11::~TRenderSystem11()
{
}

bool TRenderSystem11::Initialize()
{
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
	mDefCamera = std::make_shared<TCamera>(mScreenWidth, mScreenHeight);

	mInput = new TD3DInput(mHInst, mHWnd, width, height);

	mShadowPassRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R32_FLOAT);
	SET_DEBUG_NAME(mShadowPassRT->mDepthStencilView, "mShadowPassRT");

	mPostProcessRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R16G16B16A16_UNORM);// , DXGI_FORMAT_R8G8B8A8_UNORM);
	SET_DEBUG_NAME(mPostProcessRT->mDepthStencilView, "mPostProcessRT");
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

	hr = mDevice->CreateRenderTargetView(pBackBuffer, NULL, &mBackRenderTargetView);
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
	hr = mDevice->CreateDepthStencilView(mDepthStencil, &descDSV, &mBackDepthStencilView);
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

void TRenderSystem11::SetHandle(HINSTANCE hInstance, HWND hWnd)
{
	mHInst = hInstance;
	mHWnd = hWnd;
}

void TRenderSystem11::ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil)
{
	float colorArr[4] = { color.x, color.y, color.z, color.w }; // red, green, blue, alpha
	mDeviceContext->ClearRenderTargetView(mBackRenderTargetView, colorArr);
	mDeviceContext->ClearDepthStencilView(mBackDepthStencilView, D3D11_CLEAR_DEPTH, Depth, Stencil);
}

IRenderTexturePtr TRenderSystem11::CreateRenderTexture(int width, int height, DXGI_FORMAT format)
{
	return std::make_shared<TRenderTexture11>(mDevice, width, height, format);
}

void TRenderSystem11::_ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color, FLOAT Depth/* = 1.0*/, UINT8 Stencil/* = 0*/)
{
	mDeviceContext->ClearRenderTargetView(rendTarget->GetColorBuffer11(), (const float*)&color);
	mDeviceContext->ClearDepthStencilView(rendTarget->GetDepthStencilBuffer11(), D3D11_CLEAR_DEPTH, Depth, Stencil);
}

void TRenderSystem11::SetRenderTarget(IRenderTexturePtr rendTarget)
{
	ID3D11ShaderResourceView* TextureNull = nullptr;
	mDeviceContext->PSSetShaderResources(0, 1, &TextureNull);

	//ID3D11RenderTargetView* renderTargetView = mRenderTargetView;
	mCurRenderTargetView = rendTarget != nullptr ? rendTarget->GetColorBuffer11() : mBackRenderTargetView;
	mCurDepthStencilView = rendTarget != nullptr ? rendTarget->GetDepthStencilBuffer11() : mBackDepthStencilView;
	mDeviceContext->OMSetRenderTargets(1, &mCurRenderTargetView, mCurDepthStencilView);
}

void TRenderSystem11::_PushRenderTarget(IRenderTexturePtr rendTarget)
{
	mRenderTargetStk.push_back(rendTarget);
	SetRenderTarget(rendTarget);
}

void TRenderSystem11::_PopRenderTarget()
{
	if (!mRenderTargetStk.empty())
		mRenderTargetStk.pop_back();

	SetRenderTarget(!mRenderTargetStk.empty() ? mRenderTargetStk.back() : nullptr);
}

TMaterialPtr TRenderSystem11::CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback)
{
	return mMaterialFac->GetMaterial(name, callback);
}

ID3D11InputLayout* TRenderSystem11::_CreateInputLayout(TProgram* pProgram, const std::vector<D3D11_INPUT_ELEMENT_DESC>& descArr)
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
IInputLayoutPtr TRenderSystem11::CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	TInputLayout11Ptr ret = std::make_shared<TInputLayout11>();
	ret->mInputDescs.assign(descArray, descArray + descCount);
	if (pProgram->IsLoaded()) {
		ret->mLayout = _CreateInputLayout(pProgram.get(), ret->mInputDescs);
		ret->SetLoaded();
	}
	else {
		pProgram->AddOnLoadedListener([=](IResource* program) {
			ret->mLayout = _CreateInputLayout(static_cast<TProgram*>(program), ret->mInputDescs);
			ret->SetLoaded();
		});
	}
	return ret;
}

bool TRenderSystem11::UpdateBuffer(IHardwareBuffer* buffer, void* data, int dataSize)
{
	assert(buffer);
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	hr = (mDeviceContext->Map(buffer->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	if (CheckHR(hr))
		return false;
	memcpy(MappedResource.pData, data, dataSize);
	mDeviceContext->Unmap(buffer->GetBuffer11(), 0);
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

	TSamplerState11Ptr ret = std::make_shared<TSamplerState11>(pSamplerLinear);
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
IPixelShaderPtr TRenderSystem11::_CreatePS(const char* filename, const char* szEntry, bool async)
{
	TPixelShader11Ptr ret = std::make_shared<TPixelShader11>(std::shared_ptr<IBlobData>(new TBlobDataD3d11(nullptr)));
	szEntry = szEntry ? szEntry : "PS";
	const char* shaderModel = "ps_4_0";
	DWORD dwShaderFlags = GetShaderFlag();

	HRESULT hr;
	if (async) {
		hr = mThreadPump->AddWorkItem(ret, [&](ID3DX11ThreadPump* pump, TThreadPumpEntryPtr entry)->HRESULT {
			const D3D10_SHADER_MACRO* pDefines = nullptr;
			LPD3D10INCLUDE pInclude = new TIncludeStdio("shader\\");
			return D3DX11CompileFromFileA(filename, pDefines, pInclude, szEntry, shaderModel, dwShaderFlags, 0, pump,
				&static_cast<TBlobDataD3d11*>(ret->mBlob.get())->mBlob, &ret->mErrBlob, (HRESULT*)&entry->hr);
		}, [=](IResource* res, HRESULT hr) {
			if (!FAILED(hr)) {
				TPixelShader11* ret = static_cast<TPixelShader11*>(res);
				assert(dynamic_cast<TVertexShader11*>(res) && ret->mBlob);
				if (!CheckHR(mDevice->CreatePixelShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), NULL, &ret->mShader))) {
					ret->SetLoaded();
				}
			}
			else {
				CheckCompileError(hr, ret->mErrBlob);
			}
		});
	}
	else {
		ret->mBlob = std::shared_ptr<IBlobData>(new TBlobDataD3d11(nullptr));
		hr = D3DX11CompileFromFileA(filename, NULL, NULL, szEntry, shaderModel, dwShaderFlags, 0, nullptr,
			&static_cast<TBlobDataD3d11*>(ret->mBlob.get())->mBlob, &ret->mErrBlob, NULL);
		if (CheckCompileError(hr, ret->mErrBlob)
			&& !CheckHR(mDevice->CreatePixelShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), NULL, &ret->mShader))) {
			ret->SetLoaded();
		}
		else {
			ret = nullptr;
		}
	}
	return ret;
}

IPixelShaderPtr TRenderSystem11::_CreatePSByFXC(const char* filename)
{
	TPixelShader11Ptr ret = std::make_shared<TPixelShader11>(nullptr);
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret->mBlob = std::shared_ptr<IBlobData>(new TBlobDataStd(buffer));
		HRESULT hr = mDevice->CreatePixelShader(&buffer[0], buffer.size(), NULL, &ret->mShader);
		if (!FAILED(hr)) {
			ret->SetLoaded();
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

IVertexShaderPtr TRenderSystem11::_CreateVS(const char* filename, const char* szEntry, bool async)
{
	TVertexShader11Ptr ret = std::make_shared<TVertexShader11>(std::shared_ptr<IBlobData>(new TBlobDataD3d11(nullptr)));
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
			&static_cast<TBlobDataD3d11*>(ret->mBlob.get())->mBlob, &ret->mErrBlob, &pProcessor))
			&& !CheckHR(D3DX11CreateAsyncFileLoaderA(filename, &pDataLoader))) {
			hr = mThreadPump->AddWorkItem(ret, pDataLoader, pProcessor, [=](IResource* res, HRESULT hr) {
				if (!FAILED(hr)) {
					TVertexShader11* ret = static_cast<TVertexShader11*>(res);
					assert(dynamic_cast<TVertexShader11*>(res) && ret->mBlob);
					if (!CheckHR(mDevice->CreateVertexShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), NULL, &ret->mShader))) {
						ret->SetLoaded();
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
		hr = D3DX11CompileFromFileA(filename, NULL, NULL, szEntry, shaderModel, dwShaderFlags, 0, nullptr,
			&static_cast<TBlobDataD3d11*>(ret->mBlob.get())->mBlob, &ret->mErrBlob, NULL);
		if (CheckCompileError(hr, ret->mErrBlob)
			&& !CheckHR(mDevice->CreateVertexShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), NULL, &ret->mShader))) {
			ret->SetLoaded();
		}
		else {
			ret = nullptr;
		}
	}
	return ret;
}

IVertexShaderPtr TRenderSystem11::_CreateVSByFXC(const char* filename)
{
	TVertexShader11Ptr ret = std::make_shared<TVertexShader11>(nullptr);
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret->mBlob = std::shared_ptr<IBlobData>(new TBlobDataStd(buffer));
		auto buffer_size = buffer.size();
		HRESULT hr = mDevice->CreateVertexShader(&buffer[0], buffer_size, NULL, &ret->mShader);
		if (!FAILED(hr)) {
			ret->SetLoaded();
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

TProgramPtr TRenderSystem11::CreateProgramByCompile(const char* vsPath, const char* psPath, const char* vsEntry, const char* psEntry)
{
	TIME_PROFILE2(CreateProgramByCompile, std::string(vsPath));
	psPath = psPath ? psPath : vsPath;
	TProgramPtr program = std::make_shared<TProgram>();
	program->SetVertex(_CreateVS(vsPath, vsEntry, false));
	program->SetPixel(_CreatePS(psPath, psEntry, false));
	program->CheckAndSetLoaded();
	return program;
}

TProgramPtr TRenderSystem11::CreateProgramByFXC(const std::string& name, const char* vsEntry, const char* psEntry)
{
	TIME_PROFILE2(CreateProgramByFXC, (name));
	TProgramPtr program = std::make_shared<TProgram>();

	vsEntry = vsEntry ? vsEntry : "VS";
	std::string vsName = (name)+"_" + vsEntry + FILE_EXT_CSO;
	program->SetVertex(_CreateVSByFXC(vsName.c_str()));

	psEntry = psEntry ? psEntry : "PS";
	std::string psName = (name)+"_" + psEntry + FILE_EXT_CSO;
	program->SetPixel(_CreatePSByFXC(psName.c_str()));

	program->CheckAndSetLoaded();
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
		vertexBuffer = std::make_shared<TVertexBuffer11>(_CreateVertexBuffer(bufferSize, buffer), bufferSize, stride, offset);
	}
	else {
		vertexBuffer = std::make_shared<TVertexBuffer11>(_CreateVertexBuffer(bufferSize), bufferSize, stride, offset);
	}
	return vertexBuffer;
}

void TRenderSystem11::SetVertexBuffer(IVertexBufferPtr vertexBuffer)
{
	UINT stride = vertexBuffer->GetStride();
	UINT offset = vertexBuffer->GetOffset();
	mDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer->GetBuffer11(), &stride, &offset);
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

	IIndexBufferPtr indexBuffer = std::make_shared<TIndexBuffer11>(pIndexBuffer, bufferSize, format);
	return indexBuffer;
}

void TRenderSystem11::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	if (indexBuffer) {
		mDeviceContext->IASetIndexBuffer(indexBuffer->GetBuffer11(), indexBuffer->GetFormat(), 0);
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
	IContantBufferPtr ret = std::make_shared<TContantBuffer11>(pConstantBuffer, declPtr);

	if (data) UpdateConstBuffer(ret, data, ret->GetBufferSize());
	return ret;
}

IContantBufferPtr TRenderSystem11::CloneConstBuffer(IContantBufferPtr buffer)
{
	return CreateConstBuffer(*buffer->GetDecl());
}

void TRenderSystem11::UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize)
{
	mDeviceContext->UpdateSubresource(buffer->GetBuffer11(), 0, NULL, data, 0, 0);
}

ITexturePtr TRenderSystem11::_CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async)
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

		pTextureRV = std::make_shared<TTexture11>(nullptr, imgPath);
		HRESULT hr;
		if (async) {
			hr = mThreadPump->AddWorkItem(pTextureRV, [&](ID3DX11ThreadPump* pump, TThreadPumpEntryPtr entry)->HRESULT {
				return D3DX11CreateShaderResourceViewFromFileA(mDevice, pSrcFile, pLoadInfo, pump, &pTextureRV->GetSRV11(), NULL);
			}, ResourceSetLoaded);
		}
		else {
			hr = D3DX11CreateShaderResourceViewFromFileA(mDevice, pSrcFile, pLoadInfo, nullptr, &pTextureRV->GetSRV11(), NULL);
			pTextureRV->SetLoaded();
		}
		if (CheckHR(hr)) {
			pTextureRV = nullptr;
		}
	}
	else {
		char szBuf[260]; sprintf(szBuf, "image file %s not exist\n", pSrcFile);
		OutputDebugStringA(szBuf);
		//MessageBoxA(0, szBuf, "", MB_OK);
		pTextureRV = nullptr;
	}
	return pTextureRV;
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

cbGlobalParam TRenderSystem11::MakeAutoParam(TCameraBase* pLightCam, bool castShadow, TDirectLight* light, enLightType lightType)
{
	cbGlobalParam globalParam = {};
	//globalParam.mWorld = mWorldTransform;

	if (castShadow) {
		globalParam.mView = COPY_TO_GPU(pLightCam->mView);
		globalParam.mProjection = COPY_TO_GPU(pLightCam->mProjection);
	}
	else {
		globalParam.mView = COPY_TO_GPU(mDefCamera->mView);
		globalParam.mProjection = COPY_TO_GPU(mDefCamera->mProjection);

		globalParam.mLightView = COPY_TO_GPU(pLightCam->mView);
		globalParam.mLightProjection = COPY_TO_GPU(pLightCam->mProjection);
	}
	globalParam.HasDepthMap = mCastShdowFlag ? TRUE : FALSE;

	{
		XMVECTOR det = XMMatrixDeterminant(COPY_TO_GPU(globalParam.mWorld));
		globalParam.mWorldInv = COPY_TO_GPU(XMMatrixInverse(&det, globalParam.mWorld));

		det = XMMatrixDeterminant(COPY_TO_GPU(globalParam.mView));
		globalParam.mViewInv = COPY_TO_GPU(XMMatrixInverse(&det, globalParam.mView));

		det = XMMatrixDeterminant(COPY_TO_GPU(globalParam.mProjection));
		globalParam.mProjectionInv = COPY_TO_GPU(XMMatrixInverse(&det, globalParam.mProjection));
	}

#if 1
	switch (lightType)
	{
	case E_LIGHT_DIRECT:
		globalParam.mLightNum.x = 1;
		globalParam.mDirectLights[0] = *light;
		break;
	case E_LIGHT_POINT:
		globalParam.mLightNum.y = 1;
		globalParam.mPointLights[0] = *(TPointLight*)light;
		break;
	case E_LIGHT_SPOT:
		globalParam.mLightNum.z = 1;
		globalParam.mSpotLights[0] = *(TSpotLight*)light;
		break;
	default:
		break;
	}
#else
	globalParam.mLightNum.x = min(MAX_LIGHTS, mDirectLights.size());
	for (int i = 0; i < globalParam.mLightNum.x; ++i)
		globalParam.mDirectLights[i] = *mDirectLights[i];

	globalParam.mLightNum.y = min(MAX_LIGHTS, mPointLights.size());
	for (int i = 0; i < globalParam.mLightNum.y; ++i)
		globalParam.mPointLights[i] = *mPointLights[i];

	globalParam.mLightNum.z = min(MAX_LIGHTS, mSpotLights.size());
	for (int i = 0; i < globalParam.mLightNum.z; ++i)
		globalParam.mSpotLights[i] = *mSpotLights[i];
#endif
	return globalParam;
}

static std::vector<ID3D11Buffer*> GetConstBuffer11List(const std::vector<TContantBufferInfo>& bufferInfos) {
	std::vector<ID3D11Buffer*> ret;
	for (auto& iter : bufferInfos)
		ret.push_back(iter.buffer->GetBuffer11());
	return ret;
}
static std::vector<ID3D11SamplerState*> GetSampler11List(const std::vector<ISamplerStatePtr>& samplers) {
	std::vector<ID3D11SamplerState*> ret;
	for (auto& iter : samplers)
		ret.push_back(iter->GetSampler11());
	return ret;
}

void TRenderSystem11::BindPass(TPassPtr pass, const cbGlobalParam& globalParam)
{
	std::vector<ID3D11Buffer*> passConstBuffers = GetConstBuffer11List(pass->mConstantBuffers);
	mDeviceContext->UpdateSubresource(passConstBuffers[0], 0, NULL, &globalParam, 0, 0);

	mDeviceContext->VSSetShader(pass->mProgram->mVertex->GetShader11(), NULL, 0);
	mDeviceContext->PSSetShader(pass->mProgram->mPixel->GetShader11(), NULL, 0);

	mDeviceContext->VSSetConstantBuffers(0, passConstBuffers.size(), &passConstBuffers[0]);
	mDeviceContext->PSSetConstantBuffers(0, passConstBuffers.size(), &passConstBuffers[0]);
	mDeviceContext->IASetInputLayout(pass->mInputLayout->GetLayout11());

	mDeviceContext->IASetPrimitiveTopology(pass->mTopoLogy);

	if (!pass->mSamplers.empty()) {
		std::vector<ID3D11SamplerState*> passSamplers = GetSampler11List(pass->mSamplers);
		mDeviceContext->PSSetSamplers(0, passSamplers.size(), &passSamplers[0]);
	}
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
			std::vector<ID3D11ShaderResourceView*> texViews = textures.GetTextureViews();
			mDeviceContext->PSSetShaderResources(0, texViews.size(), &texViews[0]);
		}

		if (pass->OnBind)
			pass->OnBind(pass.get(), this, textures);

		BindPass(pass, globalParam);

		if (indexBuffer) {
			mDeviceContext->DrawIndexed(indexBuffer->GetBufferSize() / indexBuffer->GetWidth(), 0, 0);
		}
		else {
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
	cbGlobalParam globalParam = MakeAutoParam(&LightCam, lightMode == E_PASS_SHADOWCASTER, light, lightType);
	for (int i = 0; i < opQueue.size(); ++i)
		if (opQueue[i].mMaterial->IsLoaded())
		{
			globalParam.mWorld = opQueue[i].mWorldTransform;
			RenderOperation(opQueue[i], lightMode, globalParam);
		}
}

void TRenderSystem11::RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode)
{
	if (lightMode == E_PASS_SHADOWCASTER) {
		_PushRenderTarget(mShadowPassRT);
		ClearColorDepthStencil(XMFLOAT4(1, 1, 1, 1.0f), 1.0, 0);
		mCastShdowFlag = true;
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		ID3D11ShaderResourceView* depthMapView = mShadowPassRT->GetColorTexture()->GetSRV11();
		mDeviceContext->PSSetShaderResources(E_TEXTURE_DEPTH_MAP, 1, &depthMapView);

		if (mSkyBox && mSkyBox->mCubeSRV) {
			auto texture = mSkyBox->mCubeSRV->GetSRV11();
			mDeviceContext->PSSetShaderResources(E_TEXTURE_ENV, 1, &texture);
		}
	}
	else if (lightMode == E_PASS_POSTPROCESS) {
		ID3D11ShaderResourceView* pSRV = mPostProcessRT->GetColorTexture()->GetSRV11();
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
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		ID3D11ShaderResourceView* texViewNull = nullptr;
		mDeviceContext->PSSetShaderResources(E_TEXTURE_DEPTH_MAP, 1, &texViewNull);
	}
}

void TRenderSystem11::RenderSkyBox()
{
	if (mSkyBox)
		mSkyBox->Draw();
}

void TRenderSystem11::DoPostProcess()
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
	RenderSkyBox();
	return true;
}

void TRenderSystem11::EndScene()
{
	if (!mPostProcs.empty()) {
		SetRenderTarget(nullptr);
	}
	DoPostProcess();
	mSwapChain->Present(0, 0);
}
