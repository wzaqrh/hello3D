#include "TRenderSystem.h"
#include "Utility.h"

TRenderSystem* gRenderSys;

TRenderSystem::TRenderSystem()
{
	mPointLights.push_back(std::make_shared<TPointLight>());
}

TRenderSystem::~TRenderSystem()
{
}

HRESULT TRenderSystem::Initialize()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(mHWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

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
		hr = D3D11CreateDeviceAndSwapChain(NULL, mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &mSwapChain, &mDevice, &mFeatureLevel, &mDeviceContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (CheckHR(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (CheckHR(hr))
		return hr;

	hr = mDevice->CreateRenderTargetView(pBackBuffer, NULL, &mRenderTargetView);
	pBackBuffer->Release();
	if (CheckHR(hr))
		return hr;

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
	hr = mDevice->CreateDepthStencilView(mDepthStencil, &descDSV, &mDepthStencilView);
	if (CheckHR(hr))
		return hr;
	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);


	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory(&DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	DSDesc.DepthEnable = TRUE;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DSDesc.StencilEnable = FALSE;
	hr = (mDevice->CreateDepthStencilState(&DSDesc, &mDepthStencilState));

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	mDeviceContext->RSSetViewports(1, &vp);

	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_SOLID;
	wfdesc.CullMode = D3D11_CULL_BACK;
	ID3D11RasterizerState* pRasterizerState = nullptr;
	hr = mDevice->CreateRasterizerState(&wfdesc, &pRasterizerState);
	mDeviceContext->RSSetState(pRasterizerState);

	mScreenWidth = width;
	mScreenHeight = height;
	mDefCamera = std::make_shared<TCamera>(width, height);

	InputInit(mHInst, mHWnd, width, height);

	return S_OK;
}

void TRenderSystem::CleanUp()
{
}

void TRenderSystem::ApplyMaterial(TMaterialPtr material, const XMMATRIX& worldTransform)
{
	cbGlobalParam globalParam;
	globalParam.mWorld = worldTransform;
	globalParam.mView = mDefCamera->mView;
	globalParam.mProjection = mDefCamera->mProjection;
	XMVECTOR det = XMMatrixDeterminant(globalParam.mView);
	globalParam.mViewInv = XMMatrixInverse(&det, globalParam.mView);

	globalParam.mLightNum.x = min(MAX_LIGHTS, mDirectLights.size());
	for (int i = 0; i < globalParam.mLightNum.x; ++i)
		globalParam.mDirectLights[i] = *mDirectLights[i];

	globalParam.mLightNum.y = min(MAX_LIGHTS, mPointLights.size());
	for (int i = 0; i < globalParam.mLightNum.y; ++i)
		globalParam.mPointLights[i] = *mPointLights[i];

	globalParam.mLightNum.z = min(MAX_LIGHTS, mSpotLights.size());
	for (int i = 0; i < globalParam.mLightNum.z; ++i)
		globalParam.mSpotLights[i] = *mSpotLights[i];
	
	mDeviceContext->UpdateSubresource(material->mConstantBuffers[0], 0, NULL, &globalParam, 0, 0);

	mDeviceContext->VSSetShader(material->mVertexShader, NULL, 0);
	mDeviceContext->VSSetConstantBuffers(0, material->mConstantBuffers.size(), &material->mConstantBuffers[0]);

	mDeviceContext->PSSetShader(material->mPixelShader, NULL, 0);
	mDeviceContext->PSSetConstantBuffers(0, material->mConstantBuffers.size(), &material->mConstantBuffers[0]);
	mDeviceContext->IASetInputLayout(material->mInputLayout);

	mDeviceContext->IASetPrimitiveTopology(material->mTopoLogy);

	if (material->mSampler) mDeviceContext->PSSetSamplers(0, 1, &material->mSampler);
}

TSpotLightPtr TRenderSystem::AddSpotLight()
{
	TSpotLightPtr light = std::make_shared<TSpotLight>();
	mSpotLights.push_back(light);
	return light;
}

TPointLightPtr TRenderSystem::AddPointLight()
{
	TPointLightPtr light = std::make_shared<TPointLight>();
	mPointLights.push_back(light);
	return light;
}

TDirectLightPtr TRenderSystem::AddDirectLight()
{
	TDirectLightPtr light = std::make_shared<TDirectLight>();
	mDirectLights.push_back(light);
	return light;
}

TMaterialPtr TRenderSystem::CreateMaterial(const char* vsPath, const char* psPath, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	TMaterialPtr material = std::make_shared<TMaterial>();
	ID3DBlob* pVSBlob = nullptr;
	material->mVertexShader = CreateVS(vsPath, pVSBlob);
	material->mPixelShader = CreatePS(psPath);
	material->mInputLayout = CreateLayout(pVSBlob, descArray, descCount);
	
	material->mTopoLogy = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	material->mSampler = CreateSampler();

	material->mConstantBuffers.push_back(CreateConstBuffer(sizeof(cbGlobalParam)));
	return material;
}

ID3D11InputLayout* TRenderSystem::CreateLayout(ID3DBlob* pVSBlob, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	ID3D11InputLayout* pVertexLayout = nullptr;
	HRESULT hr = mDevice->CreateInputLayout(descArray, descCount, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pVertexLayout);
	if (CheckHR(hr)) {
		DXTrace(__FILE__, __LINE__, hr, DXGetErrorDescription(hr), FALSE);
		return pVertexLayout;
	}
	return pVertexLayout;
}

bool TRenderSystem::UpdateBuffer(ID3D11Buffer* buffer, void* data, int dataSize)
{
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	hr = (mDeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	if (CheckHR(hr))
		return false;
	memcpy(MappedResource.pData, data, dataSize);
	mDeviceContext->Unmap(buffer, 0);
	return true;
}

ID3D11SamplerState* TRenderSystem::CreateSampler()
{
	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* pSamplerLinear = nullptr;
	hr = mDevice->CreateSamplerState(&sampDesc, &pSamplerLinear);
	if (CheckHR(hr))
		return nullptr;
	return pSamplerLinear;
}

ID3D11PixelShader* TRenderSystem::CreatePS(const char* filename)
{
	HRESULT hr = S_OK;

	ID3DBlob* pBlob = NULL;
	hr = CompileShaderFromFile(filename, "PS", "ps_4_0", &pBlob);
	if (CheckHR(hr)) {
		MessageBox(NULL, L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return nullptr;
	}

	ID3D11PixelShader* PixelShader = nullptr;
	hr = mDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &PixelShader);
	pBlob->Release();
	if (CheckHR(hr))
		return nullptr;
	return PixelShader;
}

ID3D11VertexShader* TRenderSystem::CreateVS(const char* filename, ID3DBlob*& pVSBlob)
{
	HRESULT hr = S_OK;

	pVSBlob = NULL;
	hr = CompileShaderFromFile(filename, "VS", "vs_4_0", &pVSBlob);
	if (CheckHR(hr))
	{
		MessageBox(NULL, L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return nullptr;
	}

	ID3D11VertexShader* pVertexShader = nullptr;
	hr = mDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &pVertexShader);
	if (CheckHR(hr))
	{
		pVSBlob->Release();
		return nullptr;
	}
	return pVertexShader;
}

ID3D11Buffer* TRenderSystem::CreateVertexBuffer(int bufferSize, void* buffer)
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

ID3D11Buffer* TRenderSystem::CreateIndexBuffer(int bufferSize, void* buffer)
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
	return pIndexBuffer;
}

ID3D11Buffer* TRenderSystem::CreateConstBuffer(int bufferSize)
{
	HRESULT hr = S_OK;

	ID3D11Buffer* pConstantBuffer = nullptr;
	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = bufferSize % sizeof(XMFLOAT4) == 0 ? bufferSize : (bufferSize / sizeof(XMFLOAT4) + 1) * sizeof(XMFLOAT4);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = mDevice->CreateBuffer(&bd, NULL, &pConstantBuffer);
	if (CheckHR(hr))
		return nullptr;
	return pConstantBuffer;
}

ID3D11ShaderResourceView* TRenderSystem::CreateTexture(const char* pSrcFile)
{
	std::string imgPath = GetModelPath() + pSrcFile;
	pSrcFile = imgPath.c_str();

	ID3D11ShaderResourceView* pTextureRV = nullptr;
	WIN32_FIND_DATAA wfd;
	HANDLE hFind = FindFirstFileA(pSrcFile, &wfd);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		HRESULT hr = S_OK;
		hr = D3DX11CreateShaderResourceViewFromFileA(mDevice, pSrcFile, NULL, NULL, &pTextureRV, NULL);
		if (CheckHR(hr))
			return nullptr;
	}
	return pTextureRV;
}

std::map<std::string, ID3D11ShaderResourceView*> TexByPath;
ID3D11ShaderResourceView* TRenderSystem::GetTexByPath(const std::string& __imgPath) {
	std::string imgPath = __imgPath;
	auto pos = __imgPath.find_last_of("\\");
	if (pos != std::string::npos) {
		imgPath = __imgPath.substr(pos + 1, std::string::npos);
	}

	ID3D11ShaderResourceView* texView = nullptr;
	if (TexByPath.find(imgPath) == TexByPath.end()) {
		texView = CreateTexture(imgPath.c_str());
		TexByPath.insert(std::make_pair(imgPath, texView));
	}
	else {
		texView = TexByPath[imgPath];
	}
	return texView;
}