#include "TRenderSystem.h"
#include "Utility.h"
#include "TSkyBox.h"
#include "TPostProcess.h"

TRenderSystem* gRenderSys;

TRenderSystem::TRenderSystem()
{
	mMaterialFac = std::make_shared<TMaterialFactory>(this);
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


	SetDepthState(TDepthState(TRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL));

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
	wfdesc.CullMode = D3D11_CULL_NONE;// D3D11_CULL_BACK;
	ID3D11RasterizerState* pRasterizerState = nullptr;
	hr = mDevice->CreateRasterizerState(&wfdesc, &pRasterizerState);
	mDeviceContext->RSSetState(pRasterizerState);

	mScreenWidth = width;
	mScreenHeight = height;
	mDefCamera = std::make_shared<TCamera>(mScreenWidth, mScreenHeight);

	mInput = new TD3DInput(mHInst, mHWnd, width, height);

	mShadowPassRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R32_FLOAT);
	mPostProcessRT = CreateRenderTexture(mScreenWidth, mScreenHeight);// , DXGI_FORMAT_R8G8B8A8_UNORM);

	return S_OK;
}

void TRenderSystem::CleanUp()
{
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

TCameraPtr TRenderSystem::SetCamera(double fov, int eyeDistance, double far1)
{
	mDefCamera = std::make_shared<TCamera>(mScreenWidth, mScreenHeight, fov, eyeDistance, far1);
	if (mSkyBox)
		mSkyBox->SetRefCamera(mDefCamera);
	return mDefCamera;
}

TSkyBoxPtr TRenderSystem::SetSkyBox(const std::string& imgName)
{
	mSkyBox = std::make_shared<TSkyBox>(this, mDefCamera, imgName);
	return mSkyBox;
}

TPostProcessPtr TRenderSystem::AddPostProcess(const std::string& name)
{
	TPostProcessPtr process;
	if (name == E_PASS_POSTPROCESS) {
		TBloom* bloom = new TBloom(this, mPostProcessRT);
		process = std::shared_ptr<TPostProcess>(bloom);
	}

	if (process) mPostProcs.push_back(process);
	return process;
}

TRenderTexturePtr TRenderSystem::CreateRenderTexture(int width, int height, DXGI_FORMAT format)
{
	return std::make_shared<TRenderTexture>(mDevice, width, height, format);
}

void TRenderSystem::ClearRenderTexture(TRenderTexturePtr rendTarget, XMFLOAT4 color)
{
	mDeviceContext->ClearRenderTargetView(rendTarget->mRenderTargetView, (const float*)&color);
	mDeviceContext->ClearDepthStencilView(rendTarget->mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void TRenderSystem::SetRenderTarget(TRenderTexturePtr rendTarget)
{
	ID3D11ShaderResourceView* TextureNull = nullptr;
	mDeviceContext->PSSetShaderResources(0, 1, &TextureNull);

	//ID3D11RenderTargetView* renderTargetView = mRenderTargetView;
	ID3D11RenderTargetView* renderTargetView = rendTarget != nullptr ? rendTarget->mRenderTargetView : mRenderTargetView;
	ID3D11DepthStencilView* depthStencilView = rendTarget != nullptr ? rendTarget->mDepthStencilView : mDepthStencilView;
	mDeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

TMaterialPtr TRenderSystem::CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback)
{
	return mMaterialFac->GetMaterial(name, callback);
}

ID3D11InputLayout* TRenderSystem::CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	ID3D11InputLayout* pVertexLayout = nullptr;
	HRESULT hr = mDevice->CreateInputLayout(descArray, descCount, pProgram->mVSBlob->GetBufferPointer(), pProgram->mVSBlob->GetBufferSize(), &pVertexLayout);
	if (CheckHR(hr)) {
		DXTrace(__FILE__, __LINE__, hr, DXGetErrorDescription(hr), FALSE);
		return pVertexLayout;
	}
	return pVertexLayout;
}

bool TRenderSystem::UpdateBuffer(THardwareBuffer* buffer, void* data, int dataSize)
{
	assert(buffer);
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	hr = (mDeviceContext->Map(buffer->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	if (CheckHR(hr))
		return false;
	memcpy(MappedResource.pData, data, dataSize);
	mDeviceContext->Unmap(buffer->buffer, 0);
	return true;
}

void TRenderSystem::UpdateConstBuffer(TContantBufferPtr buffer, void* data)
{
	mDeviceContext->UpdateSubresource(buffer->buffer, 0, NULL, data, 0, 0);
}

ID3D11SamplerState* TRenderSystem::CreateSampler(D3D11_FILTER filter, D3D11_COMPARISON_FUNC comp)
{
	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = filter;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
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
	return pSamplerLinear;
}

ID3D11PixelShader* TRenderSystem::_CreatePS(const char* filename, const char* entry)
{
	HRESULT hr = S_OK;

	ID3DBlob* pBlob = NULL;
	entry = entry ? entry : "PS";
	hr = CompileShaderFromFile(filename, entry, "ps_4_0", &pBlob);
	if (FAILED(hr)) {
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

std::pair<ID3D11VertexShader*, ID3DBlob*> TRenderSystem::_CreateVS(const char* filename, const char* entry)
{
	std::pair<ID3D11VertexShader*, ID3DBlob*> ret = std::pair<ID3D11VertexShader*, ID3DBlob*>(nullptr, nullptr);
	HRESULT hr = S_OK;

	ID3DBlob* pVSBlob = NULL;
	entry = entry ? entry : "VS";
	hr = CompileShaderFromFile(filename, entry, "vs_4_0", &pVSBlob);
	if (CheckHR(hr))
	{
		MessageBox(NULL, L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return ret;
	}

	ID3D11VertexShader* pVertexShader = nullptr;
	hr = mDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &pVertexShader);
	if (CheckHR(hr))
	{
		pVSBlob->Release();
		return ret;
	}
	ret.first = pVertexShader;
	ret.second = pVSBlob;
	return ret;
}

TProgramPtr TRenderSystem::CreateProgram(const char* vsPath, const char* psPath, const char* vsEntry, const char* psEntry)
{
	psPath = psPath ? psPath : vsPath;
	TProgramPtr program = std::make_shared<TProgram>();
	auto ret = _CreateVS(vsPath, vsEntry);
	program->mVertexShader = ret.first;
	program->mVSBlob = ret.second;
	program->mPixelShader = _CreatePS(psPath, psEntry);
	return program;
}

TProgramPtr TRenderSystem::CreateProgram(const char* vsPath)
{
	return CreateProgram(vsPath, nullptr, nullptr, nullptr);
}

ID3D11Buffer* TRenderSystem::_CreateVertexBuffer(int bufferSize, void* buffer)
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

ID3D11Buffer* TRenderSystem::_CreateVertexBuffer(int bufferSize)
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

TVertexBufferPtr TRenderSystem::CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer/*=nullptr*/)
{
	TVertexBufferPtr vertexBuffer;
	if (buffer) {
		vertexBuffer = std::make_shared<TVertexBuffer>(_CreateVertexBuffer(bufferSize, buffer), bufferSize, stride, offset);
	}
	else {
		vertexBuffer = std::make_shared<TVertexBuffer>(_CreateVertexBuffer(bufferSize), bufferSize, stride, offset);
	}
	return vertexBuffer;
}

void TRenderSystem::SetVertexBuffer(TVertexBufferPtr vertexBuffer)
{
	UINT stride = vertexBuffer->stride;
	UINT offset = vertexBuffer->offset;
	mDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer->buffer, &stride, &offset);
}

TIndexBufferPtr TRenderSystem::CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer)
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

	TIndexBufferPtr indexBuffer = std::make_shared<TIndexBuffer>(pIndexBuffer, bufferSize, format);
	return indexBuffer;
}

void TRenderSystem::SetIndexBuffer(TIndexBufferPtr indexBuffer)
{
	if (indexBuffer) {
		mDeviceContext->IASetIndexBuffer(indexBuffer->buffer, indexBuffer->format, 0);
	}
	else {
		mDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	}
}

void TRenderSystem::DrawIndexed(TIndexBufferPtr indexBuffer)
{
	mDeviceContext->DrawIndexed(indexBuffer->bufferSize / indexBuffer->GetWidth(), 0, 0);
}

TContantBufferPtr TRenderSystem::CreateConstBuffer(int bufferSize)
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
	return std::make_shared<TContantBuffer>(pConstantBuffer, bufferSize);
}

ID3D11ShaderResourceView* TRenderSystem::_CreateTexture(const char* pSrcFile, DXGI_FORMAT format)
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

	ID3D11ShaderResourceView* pTextureRV = nullptr;
	if (IsFileExist(pSrcFile))
	{
		HRESULT hr = S_OK;

		D3DX11_IMAGE_LOAD_INFO LoadInfo = {};
		LoadInfo.Format = format;

		hr = D3DX11CreateShaderResourceViewFromFileA(mDevice, pSrcFile, format != DXGI_FORMAT_UNKNOWN ? &LoadInfo : nullptr, NULL, &pTextureRV, NULL);
		if (CheckHR(hr)) 
			return nullptr;
	}
	else {
		char szBuf[260]; sprintf(szBuf, "image file %s not exist", pSrcFile);
		MessageBoxA(0, szBuf, "", MB_OK);
	}
	//mDeviceContext->GenerateMips(pTextureRV);
	return pTextureRV;
}

TTexture TRenderSystem::GetTexByPath(const std::string& __imgPath, DXGI_FORMAT format/* = DXGI_FORMAT_UNKNOWN*/) {
	const char* pSrc = __imgPath.c_str();
	std::string imgPath = __imgPath;
	auto pos = __imgPath.find_last_of("\\");
	if (pos != std::string::npos) {
		imgPath = __imgPath.substr(pos + 1, std::string::npos);
	}

	ID3D11ShaderResourceView* texView = nullptr;
	if (mTexByPath.find(imgPath) == mTexByPath.end()) {
		texView = _CreateTexture(imgPath.c_str(), format);
		mTexByPath.insert(std::make_pair(imgPath, texView));
	}
	else {
		texView = mTexByPath[imgPath];
	}
	return TTexture(__imgPath, texView);
}

void TRenderSystem::SetBlendFunc(const TBlendFunc& blendFunc)
{
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

void TRenderSystem::SetDepthState(const TDepthState& depthState)
{
	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory(&DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	DSDesc.DepthEnable = depthState.depthEnable;
	DSDesc.DepthWriteMask = depthState.depthWriteMask;
	DSDesc.DepthFunc = depthState.depthFunc;
	DSDesc.StencilEnable = FALSE;
	if (CheckHR(mDevice->CreateDepthStencilState(&DSDesc, &mDepthStencilState)))
		return;
	mDeviceContext->OMSetDepthStencilState(mDepthStencilState,1);
}

cbGlobalParam TRenderSystem::MakeAutoParam(TCameraBase* pLightCam, bool castShadow)
{
	cbGlobalParam globalParam = {};
	//globalParam.mWorld = mWorldTransform;
	
	if (castShadow) {
		globalParam.mView = pLightCam->mView;
		globalParam.mProjection = pLightCam->mProjection;
	}
	else {
		globalParam.mView = mDefCamera->mView;
		globalParam.mProjection = mDefCamera->mProjection;

		globalParam.mLightView = pLightCam->mView;
		globalParam.mLightProjection = pLightCam->mProjection;
	}
	globalParam.HasDepthMap = TRUE;

	{
		XMVECTOR det = XMMatrixDeterminant(globalParam.mWorld);
		globalParam.mWorldInv = XMMatrixInverse(&det, globalParam.mWorld);

		det = XMMatrixDeterminant(globalParam.mView);
		globalParam.mViewInv = XMMatrixInverse(&det, globalParam.mView);

		det = XMMatrixDeterminant(globalParam.mProjection);
		globalParam.mProjectionInv = XMMatrixInverse(&det, globalParam.mProjection);
	}

	globalParam.mLightNum.x = min(MAX_LIGHTS, mDirectLights.size());
	for (int i = 0; i < globalParam.mLightNum.x; ++i)
		globalParam.mDirectLights[i] = *mDirectLights[i];

	globalParam.mLightNum.y = min(MAX_LIGHTS, mPointLights.size());
	for (int i = 0; i < globalParam.mLightNum.y; ++i)
		globalParam.mPointLights[i] = *mPointLights[i];

	globalParam.mLightNum.z = min(MAX_LIGHTS, mSpotLights.size());
	for (int i = 0; i < globalParam.mLightNum.z; ++i)
		globalParam.mSpotLights[i] = *mSpotLights[i];
	return globalParam;
}

void TRenderSystem::BindPass(TPassPtr pass, const cbGlobalParam& globalParam)
{
	mDeviceContext->UpdateSubresource(pass->mConstBuffers[0], 0, NULL, &globalParam, 0, 0);

	mDeviceContext->VSSetShader(pass->mProgram->mVertexShader, NULL, 0);
	mDeviceContext->PSSetShader(pass->mProgram->mPixelShader, NULL, 0);

	mDeviceContext->VSSetConstantBuffers(0, pass->mConstBuffers.size(), &pass->mConstBuffers[0]);
	mDeviceContext->PSSetConstantBuffers(0, pass->mConstBuffers.size(), &pass->mConstBuffers[0]);
	mDeviceContext->IASetInputLayout(pass->mInputLayout);

	mDeviceContext->IASetPrimitiveTopology(pass->mTopoLogy);

	if (!pass->mSamplers.empty()) {
		mDeviceContext->PSSetSamplers(0, pass->mSamplers.size(), &pass->mSamplers[0]);
	}
}

void TRenderSystem::RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam)
{
	TTechniquePtr tech = op.mMaterial->CurTech();
	auto passes = tech->GetPassesByName(lightMode);
	for (auto& pass : passes)
	{
		SetVertexBuffer(op.mVertexBuffer);
		SetIndexBuffer(op.mIndexBuffer);

		auto textures = op.mTextures;
		textures.Merge(pass->mTextures);
		if (textures.size() > 0) {
			std::vector<ID3D11ShaderResourceView*> texViews = textures.GetTextureViews();
			mDeviceContext->PSSetShaderResources(0, texViews.size(), &texViews[0]);
		}

		if (pass->OnBind)
			pass->OnBind(pass.get(), this, textures);
		BindPass(pass, globalParam);

		if (op.mIndexBuffer) {
			DrawIndexed(op.mIndexBuffer);
		}
		else {
			mDeviceContext->Draw(op.mVertexBuffer->GetCount(), 0);
		}

		if (pass->OnUnbind)
			pass->OnUnbind(pass.get(), this, textures);
	}
}

void TRenderSystem::RenderLight(TPointLightPtr light, const TRenderOperationQueue& opQueue, const std::string& lightMode)
{
	auto LightCam = light->GetLightCamera(*mDefCamera);
	cbGlobalParam globalParam = MakeAutoParam(&LightCam, lightMode == E_PASS_SHADOWCASTER);
	for (int i = 0; i < opQueue.size(); ++i) {
		globalParam.mWorld = opQueue[i].mWorldTransform;
		RenderOperation(opQueue[i], lightMode, globalParam);
	}
}

void TRenderSystem::RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode)
{
	if (lightMode == E_PASS_SHADOWCASTER) {
		ClearRenderTexture(mShadowPassRT, XMFLOAT4(1, 1, 1, 1.0f));
		SetRenderTarget(mShadowPassRT);
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		ID3D11ShaderResourceView* depthMapView = mShadowPassRT->mRenderTargetSRV;
		mDeviceContext->PSSetShaderResources(E_TEXTURE_DEPTH_MAP, 1, &depthMapView);
	}
	else if (lightMode == E_PASS_POSTPROCESS) {
		ID3D11ShaderResourceView* pSRV = mPostProcessRT->mRenderTargetSRV;
		mDeviceContext->PSSetShaderResources(0, 1, &pSRV);
	}

	for (int i = 0; i < mPointLights.size(); ++i) {
		RenderLight(mPointLights[i], opQueue, lightMode);
	}

	if (lightMode == E_PASS_SHADOWCASTER) {
		SetRenderTarget(nullptr);
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		ID3D11ShaderResourceView* texViewNull = nullptr;
		mDeviceContext->PSSetShaderResources(E_TEXTURE_DEPTH_MAP, 1, &texViewNull);
	}
}

void TRenderSystem::Draw(IRenderable* renderable)
{
	TRenderOperationQueue opQue;
	renderable->GenRenderOperation(opQue);
	RenderQueue(opQue, E_PASS_FORWARDBASE);
}

void TRenderSystem::RenderSkyBox()
{
	if (mSkyBox)
		mSkyBox->Draw();
}

void TRenderSystem::DoPostProcess()
{
	TRenderOperationQueue opQue;
	for (size_t i = 0; i < mPostProcs.size(); ++i)
		mPostProcs[i]->GenRenderOperation(opQue);
	RenderQueue(opQue, E_PASS_POSTPROCESS);
}

bool TRenderSystem::BeginScene()
{
	if (!mPostProcs.empty()) {
		ClearRenderTexture(mPostProcessRT, XMFLOAT4(0,0,0,0));
		SetRenderTarget(mPostProcessRT);
	}
	RenderSkyBox();
	return true;
}

void TRenderSystem::EndScene()
{
	if (!mPostProcs.empty()) {
		SetRenderTarget(nullptr);
	}
	DoPostProcess();
}

