#include "render_system9.h"
#include "interface_type9.h"
#include "core/rendersys/scene_manager.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/post_process.h"
#include "core/renderable/skybox.h"
#include "core/base/utility.h"

namespace mir {

RenderSystem9::RenderSystem9()
{
	//mMaterialFac = std::make_shared<MaterialFactory>(*this);
	mFXCDir = "d3d9\\";
}

RenderSystem9::~RenderSystem9()
{

}

bool RenderSystem9::Initialize(HWND hWnd, RECT vp)
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

	SetDepthState(DepthState(TRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL));
	SetBlendFunc(BlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA));

	mDevice9->GetRenderTarget(0, &mBackColorBuffer); mCurColorBuffer = mBackColorBuffer;
	mDevice9->GetDepthStencilSurface(&mBackDepthStencilBuffer); mCurDepthStencilBuffer = mBackDepthStencilBuffer;

	mScreenWidth = width;
	mScreenHeight = height;

	mShadowPassRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R32_FLOAT);
	SET_DEBUG_NAME(mShadowPassRT->mDepthStencilView, "mShadowPassRT");

	mPostProcessRT = CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R16G16B16A16_UNORM);// , DXGI_FORMAT_R8G8B8A8_UNORM);
	SET_DEBUG_NAME(mPostProcessRT->mDepthStencilView, "mPostProcessRT");

	//mSceneManager = MakePtr<SceneManager>(*this, *mMaterialFac, XMINT2(mScreenWidth, mScreenHeight), mPostProcessRT, Camera::CreatePerspective(mScreenWidth, mScreenHeight));

	D3DXMACRO Shader_Macros[] = { "SHADER_MODEL", "30000", NULL, NULL };
	mShaderMacros.assign(Shader_Macros, Shader_Macros + ARRAYSIZE(Shader_Macros));
	return true;
}

void RenderSystem9::SetViewPort(int x, int y, int w, int h)
{

}

bool RenderSystem9::_CreateDeviceAndSwapChain()
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

bool RenderSystem9::_GetDeviceCaps()
{
	if (CheckHR(mD3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &mD3DCaps))) return false;
	__log(mD3DCaps);
	return true;
}

void RenderSystem9::_SetRasterizerState()
{
	mDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	mDevice9->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}

void RenderSystem9::Update(float dt)
{
}

void RenderSystem9::CleanUp()
{
}

static inline D3DCOLOR XMFLOAT2D3DCOLOR(XMFLOAT4 color) {
	D3DCOLOR dc = D3DCOLOR_RGBA(int(color.x * 255), int(color.y * 255), int(color.z * 255), int(color.w * 255));
	return dc;
}
void RenderSystem9::ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil)
{
	if (mCurColorBuffer != mBackColorBuffer) {
		//mDevice9->ColorFill(mCurColorBuffer, NULL, XMFLOAT2D3DCOLOR(color));
	}
	mDevice9->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, XMFLOAT2D3DCOLOR(color), Depth, Stencil);
}

IRenderTexturePtr RenderSystem9::CreateRenderTexture(int width, int height, DXGI_FORMAT format/*=DXGI_FORMAT_R32G32B32A32_FLOAT*/)
{
	TTexture9Ptr pTextureRV = MakePtr<Texture9>(nullptr, "");
	D3DFORMAT Format = D3dEnumConvert::d3d11To9(format);
	if (CheckHR(mDevice9->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &pTextureRV->GetSRV9(), NULL))) return nullptr;

	IDirect3DSurface9 *pSurfaceDepthStencil = nullptr;
	if (CheckHR(mDevice9->CreateDepthStencilSurface(width, height, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE, &pSurfaceDepthStencil, NULL))) return false;

	TRenderTexture9Ptr ret = MakePtr<RenderTexture9>(pTextureRV, pSurfaceDepthStencil);
	return ret;
}

void RenderSystem9::SetRenderTarget(IRenderTexturePtr rendTarget)
{
	if (rendTarget) {
		mCurColorBuffer = std::static_pointer_cast<RenderTexture9>(rendTarget)->GetColorBuffer9();
		mCurDepthStencilBuffer = std::static_pointer_cast<RenderTexture9>(rendTarget)->GetDepthStencilBuffer9();
	}
	else {
		mCurColorBuffer = mBackColorBuffer;
		mCurDepthStencilBuffer = mBackDepthStencilBuffer;
	}
	if (CheckHR(mDevice9->SetRenderTarget(0, mCurColorBuffer))) return;
	if (CheckHR(mDevice9->SetDepthStencilSurface(mCurDepthStencilBuffer))) return;
}

IContantBufferPtr RenderSystem9::CloneConstBuffer(IContantBufferPtr buffer)
{
	TContantBuffer9Ptr ret = MakePtr<ContantBuffer9>(buffer->GetDecl());
	return ret;
}

IContantBufferPtr RenderSystem9::CreateConstBuffer(const ConstBufferDecl& cbDecl, void* data /*= nullptr*/)
{
	TContantBuffer9Ptr ret = MakePtr<ContantBuffer9>(std::make_shared<ConstBufferDecl>(cbDecl));
	if (data) UpdateBuffer((ret), data, ret->GetBufferSize());
	return ret;
}

IIndexBufferPtr RenderSystem9::CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer)
{
	TIndexBuffer9Ptr ret;
	IDirect3DIndexBuffer9* pIndexBuffer = nullptr;
	D3DFORMAT Format = D3dEnumConvert::d3d11To9(format);
	if (! CheckHR(mDevice9->CreateIndexBuffer(bufferSize, D3DUSAGE_WRITEONLY, Format, D3DPOOL_MANAGED, &pIndexBuffer, NULL))) {
		ret = MakePtr<IndexBuffer9>(pIndexBuffer, bufferSize, format);
	}
	if (buffer) UpdateBuffer((ret), buffer, bufferSize);
	return ret;
}

void RenderSystem9::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	mDevice9->SetIndices(indexBuffer ? std::static_pointer_cast<IndexBuffer9>(indexBuffer)->GetBuffer9() : nullptr);
}

IVertexBufferPtr RenderSystem9::CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer/*=nullptr*/)
{
	TVertexBuffer9Ptr ret;
	IDirect3DVertexBuffer9* pVertexBuffer = nullptr;
	if (! CheckHR(mDevice9->CreateVertexBuffer(bufferSize, D3DUSAGE_WRITEONLY, 0/*non-FVF*/, D3DPOOL_MANAGED, &pVertexBuffer, NULL))) {
		ret = MakePtr<VertexBuffer9>(pVertexBuffer, bufferSize, stride, offset);
	}
	if (buffer) UpdateBuffer(ret, buffer, bufferSize);
	return ret;
}

void RenderSystem9::SetVertexBuffer(IVertexBufferPtr vertexBuffer)
{
	UINT offset = vertexBuffer->GetOffset();
	UINT stride = vertexBuffer->GetStride();
	IDirect3DVertexBuffer9* buffer = std::static_pointer_cast<VertexBuffer9>(vertexBuffer)->GetBuffer9();
	mDevice9->SetStreamSource(0, buffer, offset, stride);
}

bool RenderSystem9::UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize)
{
	assert(buffer != nullptr);
	HardwareBufferType bufferType = buffer->GetType();
	switch (bufferType)
	{
	case kHWBufferConstant: {
		IContantBufferPtr cbuffer = std::static_pointer_cast<IContantBuffer>(buffer);
		std::static_pointer_cast<ContantBuffer9>(cbuffer)->SetBuffer9((char*)data, dataSize);
	}break;
	case kHWBufferVertex: {
		IVertexBufferPtr vbuffer = std::static_pointer_cast<IVertexBuffer>(buffer);
		IDirect3DVertexBuffer9* buffer9 = std::static_pointer_cast<VertexBuffer9>(vbuffer)->GetBuffer9();
		void* pByteDest = nullptr;
		if (CheckHR(buffer9->Lock(0, dataSize, &pByteDest, 0))) return false;
		memcpy(pByteDest, data, dataSize);
		if (CheckHR(buffer9->Unlock())) return false;
	}break;
	case kHWBufferIndex: {
		IIndexBufferPtr ibuffer = std::static_pointer_cast<IIndexBuffer>(buffer);
		IDirect3DIndexBuffer9* buffer9 = std::static_pointer_cast<IndexBuffer9>(ibuffer)->GetBuffer9();
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

void RenderSystem9::UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize)
{
	UpdateBuffer((buffer), data, dataSize);
}

static TVertexShader9Ptr _CreateVSByBlob(IDirect3DDevice9* pDevice9, IBlobDataPtr pBlob) {
	TVertexShader9Ptr ret = MakePtr< VertexShader9>();

	if (CheckHR(pDevice9->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), &ret->mShader))) return nullptr;

	ID3DXConstantTable* constTable = nullptr;
	if (CheckHR(D3DXGetShaderConstantTableEx((DWORD*)pBlob->GetBufferPointer(), D3DXCONSTTABLE_LARGEADDRESSAWARE, &constTable))) return nullptr;
	ret->SetConstTable(constTable);

	ret->mBlob = pBlob;
	ret->AsRes()->SetLoaded();
	return ret;
}
TVertexShader9Ptr RenderSystem9::_CreateVS(const char* filename, const char* entry /*= nullptr*/)
{
	DWORD Flag = 0;
#ifdef _DEBUG
	Flag = D3DXSHADER_DEBUG;
#endif
	entry = entry ? entry : "VS";
	TBlobDataD3d9Ptr blob = MakePtr<BlobData9>(nullptr);
	TBlobDataD3d9Ptr errBlob = MakePtr<BlobData9>(nullptr);
	if (FAILED(D3DXCompileShaderFromFileA(filename, &mShaderMacros[0], nullptr, entry, "vs_3_0", Flag, &blob->mBlob, &errBlob->mBlob, nullptr))) {
		OutputDebugStringA((LPCSTR)errBlob->mBlob->GetBufferPointer());
		assert(FALSE);
		return nullptr;
	}

	TVertexShader9Ptr ret = _CreateVSByBlob(mDevice9, blob);
	ret->mErrBlob = errBlob;
	return ret;
}
TVertexShader9Ptr RenderSystem9::_CreateVSByFXC(const char* filename)
{
	TVertexShader9Ptr ret;
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret = _CreateVSByBlob(mDevice9, MakePtr<BlobDataStandard>(buffer));
	}
	return ret;
}

static TPixelShader9Ptr _CreatePSByBlob(IDirect3DDevice9* pDevice9, IBlobDataPtr pBlob) {
	TPixelShader9Ptr ret = MakePtr<PixelShader9>();
	ret->mBlob = pBlob;
	
	if (CheckHR(pDevice9->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), &ret->mShader))) return nullptr;

	ID3DXConstantTable* constTable = nullptr;
	if (CheckHR(D3DXGetShaderConstantTableEx((DWORD*)pBlob->GetBufferPointer(), D3DXCONSTTABLE_LARGEADDRESSAWARE, &constTable))) return nullptr;
	ret->SetConstTable(constTable);

	ret->AsRes()->SetLoaded();
	return ret;
}
TPixelShader9Ptr RenderSystem9::_CreatePS(const char* filename, const char* entry /*= nullptr*/)
{
	DWORD Flag = 0;
#ifdef _DEBUG
	Flag = D3DXSHADER_DEBUG;
#endif
	entry = entry ? entry : "PS";
	TBlobDataD3d9Ptr blob = MakePtr<BlobData9>(nullptr);
	TBlobDataD3d9Ptr errBlob = MakePtr<BlobData9>(nullptr);
	if (FAILED(D3DXCompileShaderFromFileA(filename, &mShaderMacros[0], nullptr, entry, "ps_3_0", Flag, &blob->mBlob, &errBlob->mBlob, nullptr))) {
		OutputDebugStringA((LPCSTR)errBlob->mBlob->GetBufferPointer());
		assert(FALSE);
		return nullptr;
	}

	TPixelShader9Ptr ret = _CreatePSByBlob(mDevice9, blob);
	ret->mErrBlob = errBlob;
	return ret;
}
TPixelShader9Ptr RenderSystem9::_CreatePSByFXC(const char* filename)
{
	TPixelShader9Ptr ret;
	std::vector<char> buffer = ReadFile(filename, "rb");
	if (!buffer.empty()) {
		ret = _CreatePSByBlob(mDevice9, MakePtr<BlobDataStandard>(buffer));
	}
	return ret;
}

IProgramPtr RenderSystem9::CreateProgramByCompile(const char* vsPath, const char* psPath /*= nullptr*/, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	TIME_PROFILE2(CreateProgramByCompile, std::string(vsPath));
	psPath = psPath ? psPath : vsPath;
	TProgram9Ptr program = MakePtr<Program9>();
	program->SetVertex(_CreateVS(vsPath, vsEntry));
	program->SetPixel(_CreatePS(psPath, psEntry));
	program->AsRes()->CheckAndSetLoaded();
	return program;
}

IProgramPtr RenderSystem9::CreateProgramByFXC(const std::string& name, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	TIME_PROFILE2(CreateProgramByFXC, (name));
	TProgram9Ptr program = MakePtr<Program9>();

	vsEntry = vsEntry ? vsEntry : "VS";
	std::string vsName = (name)+"_" + vsEntry + FILE_EXT_CSO;
	program->SetVertex(_CreateVSByFXC(vsName.c_str()));

	psEntry = psEntry ? psEntry : "PS";
	std::string psName = (name)+"_" + psEntry + FILE_EXT_CSO;
	program->SetPixel(_CreatePSByFXC(psName.c_str()));

	program->AsRes()->CheckAndSetLoaded();
	return program;
}

ISamplerStatePtr RenderSystem9::CreateSampler(D3D11_FILTER filter /*= D3D11_FILTER_MIN_MAG_MIP_LINEAR*/, D3D11_COMPARISON_FUNC comp /*= D3D11_COMPARISON_NEVER*/)
{
	TSamplerState9Ptr ret = MakePtr<SamplerState9>();
	
	ret->mStates[D3DSAMP_ADDRESSU] = D3dEnumConvert::d3d11To9(D3D11_TEXTURE_ADDRESS_WRAP);
	ret->mStates[D3DSAMP_ADDRESSV] = D3dEnumConvert::d3d11To9(D3D11_TEXTURE_ADDRESS_WRAP);
	ret->mStates[D3DSAMP_ADDRESSW] = D3dEnumConvert::d3d11To9(D3D11_TEXTURE_ADDRESS_WRAP);
	ret->mStates[D3DSAMP_BORDERCOLOR] = D3DCOLOR_ARGB(0,0,0,0);

	std::map<D3DSAMPLERSTATETYPE, D3DTEXTUREFILTERTYPE> fts = D3dEnumConvert::d3d11To9(filter);
	for (auto& iter : fts)
		ret->mStates[iter.first] = iter.second;

	ret->mStates[D3DSAMP_MIPMAPLODBIAS] = 0.0f;
	ret->mStates[D3DSAMP_MAXANISOTROPY] = (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
	ret->mStates[D3DSAMP_MAXMIPLEVEL] = D3D11_FLOAT32_MAX;

	return ret;
}

IDirect3DVertexDeclaration9* RenderSystem9::_CreateInputLayout(Program9* _, const std::vector<D3DVERTEXELEMENT9>& descArr)
{
	IDirect3DVertexDeclaration9* ret = nullptr;
	if (CheckHR(mDevice9->CreateVertexDeclaration(&descArr[0], &ret))) return nullptr;
	return ret;
}

IInputLayoutPtr RenderSystem9::CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount)
{
	TInputLayout9Ptr ret = MakePtr<InputLayout9>();
	for (size_t i = 0; i < descCount; ++i)
		ret->mInputDescs.push_back(D3dEnumConvert::d3d11To9(descArray[i]));
	ret->mInputDescs.push_back(D3DDECL_END());

	auto resource = pProgram->AsRes();
	if (resource->IsLoaded()) {
		ret->mLayout = _CreateInputLayout(std::static_pointer_cast<Program9>(pProgram).get(), ret->mInputDescs);
		resource->SetLoaded();
	}
	else {
		resource->AddOnLoadedListener([=](IResource* res) {
			ret->mLayout = _CreateInputLayout(std::static_pointer_cast<Program9>(pProgram).get(), ret->mInputDescs);
			res->SetLoaded();
		});
	}
	return ret;
}

ITexturePtr RenderSystem9::_CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube)
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
		pTextureRV = MakePtr<Texture9>(nullptr, imgPath);
		if (isCube) {
			if (CheckHR(D3DXCreateCubeTextureFromFileExA(mDevice9, pSrcFile, 
				D3DX_DEFAULT, 1, 0/*D3DUSAGE_RENDERTARGET|D3DUSAGE_DYNAMIC*/, D3dEnumConvert::d3d11To9(format)/*D3DFMT_A16B16G16R16F*/,
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

ITexturePtr RenderSystem9::CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap)
{
	assert(mipmap == 1);
	TTexture9Ptr texture = MakePtr<Texture9>(width, height, format, mipmap);
	IDirect3DTexture9* pTexture = nullptr;
	if (FAILED(mDevice9->CreateTexture(width, height, 0, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, NULL)))
		return nullptr;
	texture->SetSRV9(pTexture);
	return texture;
}

bool RenderSystem9::LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep)
{
	assert(dataStep * texture->GetHeight() <= dataSize);
	TTexture9Ptr tex9 = std::static_pointer_cast<Texture9>(texture);

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

void RenderSystem9::SetBlendFunc(const BlendFunc& blendFunc)
{
	mCurBlendFunc = blendFunc;
	mDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, (blendFunc.Src == D3D11_BLEND_ONE && blendFunc.Dst == D3D11_BLEND_ZERO) ? FALSE : TRUE);
	mDevice9->SetRenderState(D3DRS_SRCBLEND, D3dEnumConvert::d3d11To9(blendFunc.Src));
	mDevice9->SetRenderState(D3DRS_DESTBLEND, D3dEnumConvert::d3d11To9(blendFunc.Dst));
}

void RenderSystem9::SetDepthState(const DepthState& depthState)
{
	mCurDepthState = depthState;
	mDevice9->SetRenderState(D3DRS_ZENABLE, depthState.DepthEnable);
	mDevice9->SetRenderState(D3DRS_ZFUNC, D3dEnumConvert::d3d11To9(depthState.DepthFunc));
	mDevice9->SetRenderState(D3DRS_ZWRITEENABLE, depthState.DepthWriteMask == D3D11_DEPTH_WRITE_MASK_ALL ? TRUE : FALSE);
}

void RenderSystem9::BindPass(PassPtr pass, const cbGlobalParam& globalParam)
{
	TProgram9Ptr program = std::static_pointer_cast<Program9>(pass->mProgram);
	PixelShader9* ps = PtrRaw(program->mPixel);
	VertexShader9* vs = PtrRaw(program->mVertex);

	if (pass->mConstantBuffers.size() > 0) {
		UpdateConstBuffer(pass->mConstantBuffers[0].buffer, (void*)&globalParam, sizeof(globalParam));
	}

	for (size_t i = 0; i < pass->mConstantBuffers.size(); ++i) {
		IContantBufferPtr buffer = pass->mConstantBuffers[i].buffer;
		char* buffer9 = (char*)std::static_pointer_cast<ContantBuffer9>(buffer)->GetBuffer9();
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
			std::map<D3DSAMPLERSTATETYPE, DWORD>& states = std::static_pointer_cast<SamplerState9>(sampler)->GetSampler9();
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
void RenderSystem9::RenderPass(PassPtr pass, TextureBySlot& textures, int iterCnt, IIndexBufferPtr indexBuffer, IVertexBufferPtr vertexBuffer, const cbGlobalParam& globalParam)
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
		for (size_t i = 0; i < textures.Count(); ++i) {
			auto iTex = std::static_pointer_cast<Texture9>(textures[i]);
			if (iTex) {
				IDirect3DTexture9* texture = iTex->GetSRV9();
				IDirect3DCubeTexture9* textureCube = iTex->GetSRVCube9();
				if (texture) mDevice9->SetTexture(i, texture);
				else if (textureCube) mDevice9->SetTexture(i, textureCube);
			}
		}

		if (pass->OnBind)
			pass->OnBind(*pass, *this, textures);

		BindPass(pass, globalParam);

		if (indexBuffer) {
			D3DPRIMITIVETYPE topo = D3dEnumConvert::d3d11To9(pass->mTopoLogy);
			if (_CanDraw())
			mDevice9->DrawIndexedPrimitive(topo,
				0, 0, vertexBuffer->GetBufferSize() / vertexBuffer->GetStride(),
				0, CalPrimCount(indexBuffer->GetCount(), topo));
		}
		else {
			D3DPRIMITIVETYPE topo = D3dEnumConvert::d3d11To9(pass->mTopoLogy);
			if (_CanDraw())
			mDevice9->DrawPrimitive(topo, 0, CalPrimCount(vertexBuffer->GetBufferSize() / vertexBuffer->GetStride(), topo));
		}

		if (pass->OnUnbind)
			pass->OnUnbind(*pass, *this, textures);
	}

	if (iterCnt >= 0) {
		_PopRenderTarget();
	}
	else if (pass->mRenderTarget) {
		_PopRenderTarget();
	}
}

void RenderSystem9::RenderOp(const RenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam)
{
	TechniquePtr tech = op.mMaterial->CurTech();
	std::vector<PassPtr> passes = tech->GetPassesByLightMode(lightMode);
	for (auto& pass : passes)
	{
		mDevice9->SetVertexDeclaration(std::static_pointer_cast<InputLayout9>(pass->mInputLayout)->GetLayout9());
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

void RenderSystem9::RenderLight(cbDirectLight* light, LightType lightType, const RenderOperationQueue& opQueue, const std::string& lightMode)
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

void RenderSystem9::RenderQueue(const RenderOperationQueue& opQueue, const std::string& lightMode)
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
		IDirect3DTexture9* depthMapView = std::static_pointer_cast<Texture9>(mShadowPassRT->GetColorTexture())->GetSRV9();
		mDevice9->SetTexture(E_TEXTURE_DEPTH_MAP, depthMapView);

		auto& skyBox = mSceneManager->mSkyBox;
		if (skyBox && skyBox->mCubeSRV) {
			IDirect3DCubeTexture9* texture = std::static_pointer_cast<Texture9>(skyBox->mCubeSRV)->GetSRVCube9();
			mDevice9->SetTexture(E_TEXTURE_ENV, texture);
		}
	}
	else if (lightMode == E_PASS_POSTPROCESS) {
		IDirect3DTexture9* pSRV = std::static_pointer_cast<Texture9>(mPostProcessRT->GetColorTexture())->GetSRV9();
		mDevice9->SetTexture(0, pSRV);
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

		mDevice9->SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		IDirect3DTexture9* texViewNull = nullptr;
		mDevice9->SetTexture(E_TEXTURE_DEPTH_MAP, texViewNull);
	}
}

void RenderSystem9::_RenderSkyBox()
{
	if (mSceneManager->mSkyBox) mSceneManager->mSkyBox->Draw();
}

void RenderSystem9::_DoPostProcess()
{
	DepthState orgState = mCurDepthState;
	SetDepthState(DepthState(false));

	RenderOperationQueue opQue;
	for (size_t i = 0; i < mSceneManager->mPostProcs.size(); ++i)
		mSceneManager->mPostProcs[i]->GenRenderOperation(opQue);
	RenderQueue(opQue, E_PASS_POSTPROCESS);

	SetDepthState(orgState);
}

bool RenderSystem9::BeginScene()
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

void RenderSystem9::EndScene()
{
	if (!mSceneManager->mPostProcs.empty()) {
		SetRenderTarget(nullptr);
	}
	_DoPostProcess();

	mDevice9->EndScene();
	mDevice9->Present(NULL, NULL, NULL, NULL);
}

}