#include "TRenderSystem9.h"
#include "ISceneManager.h"
#include "TMaterialCB.h"
#include "TMaterial.h"
#include "TInterfaceType9.h"
#include "TPostProcess.h"
#include "TSkyBox.h"
#include "Utility.h"

TRenderSystem9::TRenderSystem9()
{
	mMaterialFac = std::make_shared<TMaterialFactory>(this);
	mFXCDir = "d3d9\\";
}

TRenderSystem9::~TRenderSystem9()
{

}

bool TRenderSystem9::Initialize(HWND hWnd, RECT vp)
{
	mHWnd = hWnd;
	
	RECT rc;
	GetClientRect(mHWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	if (NULL == (mD3D9 = Direct3DCreate9(D3D_SDK_VERSION))) {
		CheckHR(E_FAIL);
		return false;
	}
	if (!_GetDeviceCaps()) return false;
	if (!_CreateDeviceAndSwapChain()) return false;
	_SetRasterizerState();

	SetDepthState(TDepthState(TRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL));
	SetBlendFunc(TBlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA));

	mDevice9->GetRenderTarget(0, &mBackColorBuffer); mCurColorBuffer = mBackColorBuffer;
	mDevice9->GetDepthStencilSurface(&mBackDepthStencilBuffer); mCurDepthStencilBuffer = mBackDepthStencilBuffer;

	mScreenWidth = width;
	mScreenHeight = height;

	mShadowPassRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R32_FLOAT);
	SET_DEBUG_NAME(mShadowPassRT->mDepthStencilView, "mShadowPassRT");

	mPostProcessRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R16G16B16A16_UNORM);// , DXGI_FORMAT_R8G8B8A8_UNORM);
	SET_DEBUG_NAME(mPostProcessRT->mDepthStencilView, "mPostProcessRT");

	mSceneManager = MakePtr<TSceneManager>(this, XMINT2(mScreenWidth, mScreenHeight), mPostProcessRT, TCamera::CreatePerspective(mScreenWidth, mScreenHeight));

	D3DXMACRO Shader_Macros[] = { "SHADER_MODEL", "30000", NULL, NULL };
	mShaderMacros.assign(Shader_Macros, Shader_Macros + ARRAYSIZE(Shader_Macros));
	return true;
}

void TRenderSystem9::SetViewPort(int x, int y, int w, int h)
{

}

bool TRenderSystem9::_CreateDeviceAndSwapChain()
{	
	// Get the desktop display mode.
	D3DDISPLAYMODE displayMode;
	if (CheckHR(mD3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode)))
		return false;

	int sampleType = D3DMULTISAMPLE_16_SAMPLES;
	DWORD sampleQuality = 0;
	for (; sampleType >= D3DMULTISAMPLE_NONE; --sampleType) {
		if (SUCCEEDED(mD3D9->CheckDeviceMultiSampleType(mD3DCaps.AdapterOrdinal, mD3DCaps.DeviceType, 
			displayMode.Format, FALSE, (D3DMULTISAMPLE_TYPE)sampleType, &sampleQuality))) {
			break;
		}
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = displayMode.Format;// D3DFMT_A8B8G8R8;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)sampleType;
	//d3dpp.MultiSampleQuality = sampleQuality;

	bool success = false;
	int BehaviorFlags[] = { D3DCREATE_HARDWARE_VERTEXPROCESSING, D3DCREATE_HARDWARE_VERTEXPROCESSING, D3DCREATE_MIXED_VERTEXPROCESSING, D3DCREATE_SOFTWARE_VERTEXPROCESSING };
	for (size_t i = 0; i < ARRAYSIZE(BehaviorFlags); ++i) {
		if (! FAILED(mD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mHWnd, BehaviorFlags[i], &d3dpp, &mDevice9))) {
			success = true;
			break;
		}
	}
	assert(success);
	return success;
}

bool TRenderSystem9::_GetDeviceCaps()
{
	if (CheckHR(mD3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &mD3DCaps))) return false;
	__log(mD3DCaps);
	return true;
}

void TRenderSystem9::_SetRasterizerState()
{
	mDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	mDevice9->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}

void TRenderSystem9::Update(float dt)
{
}

void TRenderSystem9::CleanUp()
{
}

static inline D3DCOLOR XMFLOAT2D3DCOLOR(XMFLOAT4 color) {
	D3DCOLOR dc = D3DCOLOR_RGBA(int(color.x * 255), int(color.y * 255), int(color.z * 255), int(color.w * 255));
	return dc;
}
void TRenderSystem9::ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil)
{
	if (mCurColorBuffer != mBackColorBuffer) {
		//mDevice9->ColorFill(mCurColorBuffer, NULL, XMFLOAT2D3DCOLOR(color));
	}
	mDevice9->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, XMFLOAT2D3DCOLOR(color), Depth, Stencil);
}

IRenderTexturePtr TRenderSystem9::CreateRenderTexture(int width, int height, DXGI_FORMAT format/*=DXGI_FORMAT_R32G32B32A32_FLOAT*/)
{
	TTexture9Ptr pTextureRV = MakePtr<TTexture9>(nullptr, "");
	D3DFORMAT Format = D3DEnumCT::d3d11To9(format);
	if (CheckHR(mDevice9->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &pTextureRV->GetSRV9(), NULL))) return nullptr;

	IDirect3DSurface9 *pSurfaceDepthStencil = nullptr;
	if (CheckHR(mDevice9->CreateDepthStencilSurface(width, height, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE, &pSurfaceDepthStencil, NULL))) return false;

	TRenderTexture9Ptr ret = MakePtr<TRenderTexture9>(pTextureRV, pSurfaceDepthStencil);
	return ret;
}

void TRenderSystem9::SetRenderTarget(IRenderTexturePtr rendTarget)
{
	if (rendTarget) {
		mCurColorBuffer = PtrCast(rendTarget).As<TRenderTexture9>()->GetColorBuffer9();
		mCurDepthStencilBuffer = PtrCast(rendTarget).As<TRenderTexture9>()->GetDepthStencilBuffer9();
	}
	else {
		mCurColorBuffer = mBackColorBuffer;
		mCurDepthStencilBuffer = mBackDepthStencilBuffer;
	}
	if (CheckHR(mDevice9->SetRenderTarget(0, mCurColorBuffer))) return;
	if (CheckHR(mDevice9->SetDepthStencilSurface(mCurDepthStencilBuffer))) return;
}

TMaterialPtr TRenderSystem9::CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback)
{
	TMaterialPtr material = mMaterialFac->GetMaterial(name, callback);
	material->SetCurTechByName("d3d9");
	return material;
}

IContantBufferPtr TRenderSystem9::CloneConstBuffer(IContantBufferPtr buffer)
{
	TContantBuffer9Ptr ret = MakePtr<TContantBuffer9>(buffer->GetDecl());
	return ret;
}

IContantBufferPtr TRenderSystem9::CreateConstBuffer(const TConstBufferDecl& cbDecl, void* data /*= nullptr*/)
{
	TContantBuffer9Ptr ret = MakePtr<TContantBuffer9>(std::make_shared<TConstBufferDecl>(cbDecl));
	if (data) UpdateBuffer((ret), data, ret->GetBufferSize());
	return ret;
}

IIndexBufferPtr TRenderSystem9::CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer)
{
	TIndexBuffer9Ptr ret;
	IDirect3DIndexBuffer9* pIndexBuffer = nullptr;
	D3DFORMAT Format = D3DEnumCT::d3d11To9(format);
	if (! CheckHR(mDevice9->CreateIndexBuffer(bufferSize, D3DUSAGE_WRITEONLY, Format, D3DPOOL_MANAGED, &pIndexBuffer, NULL))) {
		ret = MakePtr<TIndexBuffer9>(pIndexBuffer, bufferSize, format);
	}
	if (buffer) UpdateBuffer((ret), buffer, bufferSize);
	return ret;
}

void TRenderSystem9::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	mDevice9->SetIndices(indexBuffer ? PtrCast(indexBuffer).As<TIndexBuffer9>()->GetBuffer9() : nullptr);
}

IVertexBufferPtr TRenderSystem9::CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer/*=nullptr*/)
{
	TVertexBuffer9Ptr ret;
	IDirect3DVertexBuffer9* pVertexBuffer = nullptr;
	if (! CheckHR(mDevice9->CreateVertexBuffer(bufferSize, D3DUSAGE_WRITEONLY, 0/*non-FVF*/, D3DPOOL_MANAGED, &pVertexBuffer, NULL))) {
		ret = MakePtr<TVertexBuffer9>(pVertexBuffer, bufferSize, stride, offset);
	}
	if (buffer) UpdateBuffer(ret, buffer, bufferSize);
	return ret;
}

void TRenderSystem9::SetVertexBuffer(IVertexBufferPtr vertexBuffer)
{
	UINT offset = vertexBuffer->GetOffset();
	UINT stride = vertexBuffer->GetStride();
	IDirect3DVertexBuffer9* buffer = PtrCast(vertexBuffer).As<TVertexBuffer9>()->GetBuffer9();
	mDevice9->SetStreamSource(0, buffer, offset, stride);
}

bool TRenderSystem9::UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize)
{
	assert(buffer != nullptr);
	enHardwareBufferType bufferType = buffer->GetType();
	switch (bufferType)
	{
	case E_HWBUFFER_CONSTANT: {
		IContantBufferPtr cbuffer = PtrCast(buffer).Cast<IContantBuffer>();
		PtrCast(cbuffer).As<TContantBuffer9>()->SetBuffer9((char*)data, dataSize);
	}break;
	case E_HWBUFFER_VERTEX: {
		IVertexBufferPtr vbuffer = PtrCast(buffer).Cast<IVertexBuffer>();
		IDirect3DVertexBuffer9* buffer9 = PtrCast(vbuffer).As<TVertexBuffer9>()->GetBuffer9();
		void* pByteDest = nullptr;
		if (CheckHR(buffer9->Lock(0, dataSize, &pByteDest, 0))) return false;
		memcpy(pByteDest, data, dataSize);
		if (CheckHR(buffer9->Unlock())) return false;
	}break;
	case E_HWBUFFER_INDEX: {
		IIndexBufferPtr ibuffer = PtrCast(buffer).Cast<IIndexBuffer>();
		IDirect3DIndexBuffer9* buffer9 = PtrCast(ibuffer).As<TIndexBuffer9>()->GetBuffer9();
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

void TRenderSystem9::UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize)
{
	UpdateBuffer((buffer), data, dataSize);
}

static TVertexShader9Ptr _CreateVSByBlob(IDirect3DDevice9* pDevice9, IBlobDataPtr pBlob) {
	TVertexShader9Ptr ret = MakePtr< TVertexShader9>();

	if (CheckHR(pDevice9->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), &ret->mShader))) return nullptr;

	ID3DXConstantTable* constTable = nullptr;
	if (CheckHR(D3DXGetShaderConstantTableEx((DWORD*)pBlob->GetBufferPointer(), D3DXCONSTTABLE_LARGEADDRESSAWARE, &constTable))) return nullptr;
	ret->SetConstTable(constTable);

	ret->mBlob = pBlob;
	ret->AsRes()->SetLoaded();
	return ret;
}
TVertexShader9Ptr TRenderSystem9::_CreateVS(const char* filename, const char* entry /*= nullptr*/)
{
	DWORD Flag = 0;
#ifdef _DEBUG
	Flag = D3DXSHADER_DEBUG;
#endif
	entry = entry ? entry : "VS";
	TBlobDataD3d9Ptr blob = MakePtr<TBlobDataD3d9>(nullptr);
	TBlobDataD3d9Ptr errBlob = MakePtr<TBlobDataD3d9>(nullptr);
	if (FAILED(D3DXCompileShaderFromFileA(filename, &mShaderMacros[0], nullptr, entry, "vs_3_0", Flag, &blob->mBlob, &errBlob->mBlob, nullptr))) {
		OutputDebugStringA((LPCSTR)errBlob->mBlob->GetBufferPointer());
		assert(FALSE);
		return nullptr;
	}

	TVertexShader9Ptr ret = _CreateVSByBlob(mDevice9, blob);
	ret->mErrBlob = errBlob;
	return ret;
}
TVertexShader9Ptr TRenderSystem9::_CreateVSByFXC(const char* filename)
{
	TVertexShader9Ptr ret;
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret = _CreateVSByBlob(mDevice9, MakePtr<TBlobDataStd>(buffer));
	}
	return ret;
}

static TPixelShader9Ptr _CreatePSByBlob(IDirect3DDevice9* pDevice9, IBlobDataPtr pBlob) {
	TPixelShader9Ptr ret = MakePtr<TPixelShader9>();
	ret->mBlob = pBlob;
	
	if (CheckHR(pDevice9->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), &ret->mShader))) return nullptr;

	ID3DXConstantTable* constTable = nullptr;
	if (CheckHR(D3DXGetShaderConstantTableEx((DWORD*)pBlob->GetBufferPointer(), D3DXCONSTTABLE_LARGEADDRESSAWARE, &constTable))) return nullptr;
	ret->SetConstTable(constTable);

	ret->AsRes()->SetLoaded();
	return ret;
}
TPixelShader9Ptr TRenderSystem9::_CreatePS(const char* filename, const char* entry /*= nullptr*/)
{
	DWORD Flag = 0;
#ifdef _DEBUG
	Flag = D3DXSHADER_DEBUG;
#endif
	entry = entry ? entry : "PS";
	TBlobDataD3d9Ptr blob = MakePtr<TBlobDataD3d9>(nullptr);
	TBlobDataD3d9Ptr errBlob = MakePtr<TBlobDataD3d9>(nullptr);
	if (FAILED(D3DXCompileShaderFromFileA(filename, &mShaderMacros[0], nullptr, entry, "ps_3_0", Flag, &blob->mBlob, &errBlob->mBlob, nullptr))) {
		OutputDebugStringA((LPCSTR)errBlob->mBlob->GetBufferPointer());
		assert(FALSE);
		return nullptr;
	}

	TPixelShader9Ptr ret = _CreatePSByBlob(mDevice9, blob);
	ret->mErrBlob = errBlob;
	return ret;
}
TPixelShader9Ptr TRenderSystem9::_CreatePSByFXC(const char* filename)
{
	TPixelShader9Ptr ret;
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret = _CreatePSByBlob(mDevice9, MakePtr<TBlobDataStd>(buffer));
	}
	return ret;
}

IProgramPtr TRenderSystem9::CreateProgramByCompile(const char* vsPath, const char* psPath /*= nullptr*/, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	TIME_PROFILE2(CreateProgramByCompile, std::string(vsPath));
	psPath = psPath ? psPath : vsPath;
	TProgram9Ptr program = MakePtr<TProgram9>();
	program->SetVertex(_CreateVS(vsPath, vsEntry));
	program->SetPixel(_CreatePS(psPath, psEntry));
	program->AsRes()->CheckAndSetLoaded();
	return program;
}

IProgramPtr TRenderSystem9::CreateProgramByFXC(const std::string& name, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	TIME_PROFILE2(CreateProgramByFXC, (name));
	TProgram9Ptr program = MakePtr<TProgram9>();

	vsEntry = vsEntry ? vsEntry : "VS";
	std::string vsName = (name)+"_" + vsEntry + FILE_EXT_CSO;
	program->SetVertex(_CreateVSByFXC(vsName.c_str()));

	psEntry = psEntry ? psEntry : "PS";
	std::string psName = (name)+"_" + psEntry + FILE_EXT_CSO;
	program->SetPixel(_CreatePSByFXC(psName.c_str()));

	program->AsRes()->CheckAndSetLoaded();
	return program;
}

ISamplerStatePtr TRenderSystem9::CreateSampler(D3D11_FILTER filter /*= D3D11_FILTER_MIN_MAG_MIP_LINEAR*/, D3D11_COMPARISON_FUNC comp /*= D3D11_COMPARISON_NEVER*/)
{
	TSamplerState9Ptr ret = MakePtr<TSamplerState9>();
	
	ret->mStates[D3DSAMP_ADDRESSU] = D3DEnumCT::d3d11To9(D3D11_TEXTURE_ADDRESS_WRAP);
	ret->mStates[D3DSAMP_ADDRESSV] = D3DEnumCT::d3d11To9(D3D11_TEXTURE_ADDRESS_WRAP);
	ret->mStates[D3DSAMP_ADDRESSW] = D3DEnumCT::d3d11To9(D3D11_TEXTURE_ADDRESS_WRAP);
	ret->mStates[D3DSAMP_BORDERCOLOR] = D3DCOLOR_ARGB(0,0,0,0);

	std::map<D3DSAMPLERSTATETYPE, D3DTEXTUREFILTERTYPE> fts = D3DEnumCT::d3d11To9(filter);
	for (auto& iter : fts)
		ret->mStates[iter.first] = iter.second;

	ret->mStates[D3DSAMP_MIPMAPLODBIAS] = 0.0f;
	ret->mStates[D3DSAMP_MAXANISOTROPY] = (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
	ret->mStates[D3DSAMP_MAXMIPLEVEL] = D3D11_FLOAT32_MAX;

	return ret;
}

IDirect3DVertexDeclaration9* TRenderSystem9::_CreateInputLayout(TProgram9* _, const std::vector<D3DVERTEXELEMENT9>& descArr)
{
	IDirect3DVertexDeclaration9* ret = nullptr;
	if (CheckHR(mDevice9->CreateVertexDeclaration(&descArr[0], &ret))) return nullptr;
	return ret;
}

IInputLayoutPtr TRenderSystem9::CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	TInputLayout9Ptr ret = MakePtr<TInputLayout9>();
	for (size_t i = 0; i < descCount; ++i)
		ret->mInputDescs.push_back(D3DEnumCT::d3d11To9(descArray[i]));
	ret->mInputDescs.push_back(D3DDECL_END());

	auto resource = pProgram->AsRes();
	if (resource->IsLoaded()) {
		ret->mLayout = _CreateInputLayout(PtrCast(pProgram).As<TProgram9>(), ret->mInputDescs);
		resource->SetLoaded();
	}
	else {
		resource->AddOnLoadedListener([=](IResource* res) {
			ret->mLayout = _CreateInputLayout(PtrCast(pProgram).As<TProgram9>(), ret->mInputDescs);
			res->SetLoaded();
		});
	}
	return ret;
}

ITexturePtr TRenderSystem9::_CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube)
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

	TTexture9Ptr pTextureRV;
	if (IsFileExist(pSrcFile)) {
		pTextureRV = MakePtr<TTexture9>(nullptr, imgPath);
		if (isCube) {
			if (CheckHR(D3DXCreateCubeTextureFromFileExA(mDevice9, pSrcFile, 
				D3DX_DEFAULT, 1, 0/*D3DUSAGE_RENDERTARGET|D3DUSAGE_DYNAMIC*/, D3DEnumCT::d3d11To9(format)/*D3DFMT_A16B16G16R16F*/,
				D3DPOOL_MANAGED, D3DX_FILTER_NONE/*D3DX_DEFAULT */, D3DX_FILTER_NONE, 0, NULL,
				NULL, &pTextureRV->GetSRVCube9()))) {
				pTextureRV = nullptr;
			}
		}
		else {
			if (CheckHR(D3DXCreateTextureFromFileA(mDevice9, pSrcFile, &pTextureRV->GetSRV9()))) {
				pTextureRV = nullptr;
			}
		}
	}
	else {
		char szBuf[260]; sprintf(szBuf, "image file %s not exist\n", pSrcFile);
		OutputDebugStringA(szBuf);
		//MessageBoxA(0, szBuf, "", MB_OK);
	}
	return pTextureRV;
}

ITexturePtr TRenderSystem9::CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap)
{
	assert(mipmap == 1);
	TTexture9Ptr texture = MakePtr<TTexture9>(width, height, format, mipmap);
	IDirect3DTexture9* pTexture = nullptr;
	if (FAILED(mDevice9->CreateTexture(width, height, 0, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, NULL)))
		return nullptr;
	texture->SetSRV9(pTexture);
	return texture;
}

bool TRenderSystem9::LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep)
{
	assert(dataStep * texture->GetHeight() <= dataSize);
	TTexture9Ptr tex9 = PtrCast(texture).As1<TTexture9>();

	int width = texture->GetWidth(), height = texture->GetHeight();
	IDirect3DTexture9* pTexture = tex9->GetSRV9();
	if (pTexture == nullptr) return false;

	D3DLOCKED_RECT outRect;
	if (FAILED(pTexture->LockRect(0, &outRect, nullptr, D3DLOCK_DISCARD)))
		return false;
	
	char *src = data, *dst = (char*)outRect.pBits;
	int pitch = min(outRect.Pitch, dataStep);
	for (int y = 0; y < height; ++y)
	{
		memcpy(dst, src, pitch);
		src += dataStep;
		dst += outRect.Pitch;
	}
	if (FAILED(pTexture->UnlockRect(0)))
		return false;

	return true;
}

void TRenderSystem9::SetBlendFunc(const TBlendFunc& blendFunc)
{
	mCurBlendFunc = blendFunc;
	mDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, (blendFunc.src == D3D11_BLEND_ONE && blendFunc.dst == D3D11_BLEND_ZERO) ? FALSE : TRUE);
	mDevice9->SetRenderState(D3DRS_SRCBLEND, D3DEnumCT::d3d11To9(blendFunc.src));
	mDevice9->SetRenderState(D3DRS_DESTBLEND, D3DEnumCT::d3d11To9(blendFunc.dst));
}

void TRenderSystem9::SetDepthState(const TDepthState& depthState)
{
	mCurDepthState = depthState;
	mDevice9->SetRenderState(D3DRS_ZENABLE, depthState.depthEnable);
	mDevice9->SetRenderState(D3DRS_ZFUNC, D3DEnumCT::d3d11To9(depthState.depthFunc));
	mDevice9->SetRenderState(D3DRS_ZWRITEENABLE, depthState.depthWriteMask == D3D11_DEPTH_WRITE_MASK_ALL ? TRUE : FALSE);
}

void TRenderSystem9::BindPass(TPassPtr pass, const cbGlobalParam& globalParam)
{
	TProgram9Ptr program = PtrCast(pass->mProgram).As1<TProgram9>();
	TPixelShader9* ps = PtrRaw(program->mPixel);
	TVertexShader9* vs = PtrRaw(program->mVertex);

	if (pass->mConstantBuffers.size() > 0) {
		UpdateConstBuffer(pass->mConstantBuffers[0].buffer, (void*)&globalParam, sizeof(globalParam));
	}

	for (size_t i = 0; i < pass->mConstantBuffers.size(); ++i) {
		IContantBufferPtr buffer = pass->mConstantBuffers[i].buffer;
		char* buffer9 = (char*)PtrCast(buffer).As<TContantBuffer9>()->GetBuffer9();
		TConstBufferDeclPtr decl = buffer->GetDecl();
		vs->mConstTable.SetValue(mDevice9, buffer9, *decl);
		ps->mConstTable.SetValue(mDevice9, buffer9, *decl);
	}

	mDevice9->SetVertexShader(vs->GetShader9());
	mDevice9->SetPixelShader(ps->GetShader9());

#if 1
	if (!pass->mSamplers.empty()) {
		for (size_t i = 0; i < pass->mSamplers.size(); ++i) {
			ISamplerStatePtr sampler = pass->mSamplers[i];
			std::map<D3DSAMPLERSTATETYPE, DWORD>& states = PtrCast(sampler).As<TSamplerState9>()->GetSampler9();
			for (auto& pair : states) {
				mDevice9->SetSamplerState(i, pair.first, pair.second);
			}
		}
	}
#else
	for (size_t slot = 0; slot < 8; ++slot) {
		mDevice9->SetSamplerState(slot, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		mDevice9->SetSamplerState(slot, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		mDevice9->SetSamplerState(slot, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);

		mDevice9->SetSamplerState(slot, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		mDevice9->SetSamplerState(slot, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		mDevice9->SetSamplerState(slot, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}
#endif
}

inline int CalPrimCount(int indexCount, D3DPRIMITIVETYPE topo) {
	switch (topo)
	{
	case D3DPT_POINTLIST: return indexCount;
	case D3DPT_LINELIST: return indexCount / 2;
	case D3DPT_LINESTRIP: return indexCount - 1;
	case D3DPT_TRIANGLELIST: return indexCount / 3;
	case D3DPT_TRIANGLESTRIP: return indexCount - 2;
	case D3DPT_TRIANGLEFAN: return indexCount - 2;
	default:break;
	}
	assert(FALSE);
	return 0;
}
void TRenderSystem9::RenderPass(TPassPtr pass, TTextureBySlot& textures, int iterCnt, IIndexBufferPtr indexBuffer, IVertexBufferPtr vertexBuffer, const cbGlobalParam& globalParam)
{
	if (iterCnt >= 0) {
		_PushRenderTarget(pass->mIterTargets[iterCnt]);
	}
	else if (pass->mRenderTarget) {
		_PushRenderTarget(pass->mRenderTarget);
	}

	if (iterCnt >= 0) {
		if (iterCnt + 1 < pass->mIterTargets.size())
			textures[0] = pass->mIterTargets[iterCnt + 1]->GetColorTexture();
	}
	else if (!pass->mIterTargets.empty()) {
		textures[0] = pass->mIterTargets[0]->GetColorTexture();
	}

	{
		for (size_t i = 0; i < textures.size(); ++i) {
			auto iTex = PtrCast(textures[i]).As<TTexture9>();
			if (iTex) {
				IDirect3DTexture9* texture = iTex->GetSRV9();
				IDirect3DCubeTexture9* textureCube = iTex->GetSRVCube9();
				if (texture) mDevice9->SetTexture(i, texture);
				else if (textureCube) mDevice9->SetTexture(i, textureCube);
			}
		}

		if (pass->OnBind)
			pass->OnBind(pass.get(), this, textures);

		BindPass(pass, globalParam);

		if (indexBuffer) {
			D3DPRIMITIVETYPE topo = D3DEnumCT::d3d11To9(pass->mTopoLogy);
			if (_CanDraw())
			mDevice9->DrawIndexedPrimitive(topo,
				0, 0, vertexBuffer->GetBufferSize() / vertexBuffer->GetStride(),
				0, CalPrimCount(indexBuffer->GetCount(), topo));
		}
		else {
			D3DPRIMITIVETYPE topo = D3DEnumCT::d3d11To9(pass->mTopoLogy);
			if (_CanDraw())
			mDevice9->DrawPrimitive(topo, 0, CalPrimCount(vertexBuffer->GetBufferSize() / vertexBuffer->GetStride(), topo));
		}

		if (pass->OnUnbind)
			pass->OnUnbind(pass.get(), this, textures);
	}

	if (iterCnt >= 0) {
		_PopRenderTarget();
	}
	else if (pass->mRenderTarget) {
		_PopRenderTarget();
	}
}

void TRenderSystem9::RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam)
{
	TTechniquePtr tech = op.mMaterial->CurTech();
	std::vector<TPassPtr> passes = tech->GetPassesByName(lightMode);
	for (auto& pass : passes)
	{
		mDevice9->SetVertexDeclaration(PtrCast(pass->mInputLayout).As<TInputLayout9>()->GetLayout9());
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

void TRenderSystem9::RenderLight(TDirectLight* light, enLightType lightType, const TRenderOperationQueue& opQueue, const std::string& lightMode)
{
	auto LightCam = light->GetLightCamera(*mSceneManager->mDefCamera);
	
	cbGlobalParam globalParam;
	MakeAutoParam(globalParam, LightCam, lightMode == E_PASS_SHADOWCASTER, light, lightType);

	for (int i = 0; i < opQueue.Count(); ++i)
		if (opQueue[i].mMaterial->IsLoaded()) {
			globalParam.World = opQueue[i].mWorldTransform;
			globalParam.WorldInv = XM::Inverse(globalParam.World);
			RenderOperation(opQueue[i], lightMode, globalParam);
		}
}

void TRenderSystem9::RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode)
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
		IDirect3DTexture9* depthMapView = PtrCast(mShadowPassRT->GetColorTexture()).As<TTexture9>()->GetSRV9();
		mDevice9->SetTexture(E_TEXTURE_DEPTH_MAP, depthMapView);

		auto& skyBox = mSceneManager->mSkyBox;
		if (skyBox && skyBox->mCubeSRV) {
			IDirect3DCubeTexture9* texture = PtrCast(skyBox->mCubeSRV).As<TTexture9>()->GetSRVCube9();
			mDevice9->SetTexture(E_TEXTURE_ENV, texture);
		}
	}
	else if (lightMode == E_PASS_POSTPROCESS) {
		IDirect3DTexture9* pSRV = PtrCast(mPostProcessRT->GetColorTexture()).As<TTexture9>()->GetSRV9();
		mDevice9->SetTexture(0, pSRV);
	}

	auto& lightsOrder = mSceneManager->mLightsOrder;
	if (!lightsOrder.empty()) {
		TBlendFunc orgBlend = mCurBlendFunc;
		SetBlendFunc(TBlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA));
		RenderLight(lightsOrder[0].first, lightsOrder[0].second, opQueue, lightMode);

		for (int i = 1; i < lightsOrder.size(); ++i) {
			auto order = lightsOrder[i];
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

		mDevice9->SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		IDirect3DTexture9* texViewNull = nullptr;
		mDevice9->SetTexture(E_TEXTURE_DEPTH_MAP, texViewNull);
	}
}

void TRenderSystem9::_RenderSkyBox()
{
	if (mSceneManager->mSkyBox) mSceneManager->mSkyBox->Draw();
}

void TRenderSystem9::_DoPostProcess()
{
	TDepthState orgState = mCurDepthState;
	SetDepthState(TDepthState(false));

	TRenderOperationQueue opQue;
	for (size_t i = 0; i < mSceneManager->mPostProcs.size(); ++i)
		mSceneManager->mPostProcs[i]->GenRenderOperation(opQue);
	RenderQueue(opQue, E_PASS_POSTPROCESS);

	SetDepthState(orgState);
}

bool TRenderSystem9::BeginScene()
{
	if (FAILED(mDevice9->BeginScene()))
		return false;

	mCastShdowFlag = false;

	if (!mSceneManager->mPostProcs.empty()) {
		SetRenderTarget(mPostProcessRT);
		ClearColorDepthStencil(XMFLOAT4(0, 0, 0, 0), 1.0, 0);
	}
	_RenderSkyBox();
	return true;
}

void TRenderSystem9::EndScene()
{
	if (!mSceneManager->mPostProcs.empty()) {
		SetRenderTarget(nullptr);
	}
	_DoPostProcess();

	mDevice9->EndScene();
	mDevice9->Present(NULL, NULL, NULL, NULL);
}