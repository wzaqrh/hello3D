#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <windows.h>
#include <dxerr.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <IL/il.h>
#include "core/base/d3d.h"
#include "core/base/debug.h"
#include "core/base/input.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d11/interface_type11.h"
#include "core/rendersys/d3d11/thread_pump.h"
#include "core/rendersys/material_factory.h"

using Microsoft::WRL::ComPtr;

namespace mir {

#define MakePtr std::make_shared
#define PtrRaw(T) T.get()

RenderSystem11::RenderSystem11()
{
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

	SetDepthState(DepthState{ true, kCompareLessEqual, kDepthWriteMaskAll });
	SetBlendFunc(BlendState::MakeAlphaPremultiplied());

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

void RenderSystem11::ClearColorDepthStencil(const Eigen::Vector4f& color, float Depth, unsigned char Stencil)
{
	float colorArr[4] = { color.x(), color.y(), color.z(), color.w() };
	mDeviceContext->ClearRenderTargetView(mCurRenderTargetView, colorArr);
	mDeviceContext->ClearDepthStencilView(mCurDepthStencilView, D3D11_CLEAR_DEPTH, Depth, Stencil);
}

IResourcePtr RenderSystem11::CreateResource(DeviceResourceType deviceResType)
{
	switch (deviceResType) {
	case mir::kDeviceResourceInputLayout:
		return MakePtr<InputLayout11>();
	case mir::kDeviceResourceProgram:
		return MakePtr<Program11>();
	case mir::kDeviceResourceVertexBuffer:
		return MakePtr<VertexBuffer11>();
	case mir::kDeviceResourceIndexBuffer:
		return MakePtr<IndexBuffer11>();
	case mir::kDeviceResourceContantBuffer:
		return MakePtr<ContantBuffer11>();
	case mir::kDeviceResourceTexture:
		return MakePtr<Texture11>(nullptr);
	case mir::kDeviceResourceRenderTexture:
		return MakePtr<RenderTexture11>();
	case mir::kDeviceResourceSamplerState:
		return MakePtr<SamplerState11>();
	default:
		break;
	}
	return nullptr;
}

IRenderTexturePtr RenderSystem11::LoadRenderTexture(IResourcePtr res, int width, int height, ResourceFormat format)
{
	if (res == nullptr) res = CreateResource(kDeviceResourceRenderTexture);

	RenderTexture11Ptr ret = std::static_pointer_cast<RenderTexture11>(res);
	ret->Init(mDevice, width, height, format);
	return ret;
}

void RenderSystem11::_ClearRenderTexture(IRenderTexturePtr rendTarget, const Eigen::Vector4f& color, 
	float depth, unsigned char stencil)
{
	auto target11 = std::static_pointer_cast<RenderTexture11>(rendTarget);
	mDeviceContext->ClearRenderTargetView(target11->GetColorBuffer11(), (const float*)&color);
	mDeviceContext->ClearDepthStencilView(target11->GetDepthStencilBuffer11(), D3D11_CLEAR_DEPTH, depth, stencil);
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
IInputLayoutPtr RenderSystem11::LoadLayout(IResourcePtr res, IProgramPtr pProgram, const std::vector<LayoutInputElement>& descArr)
{
	if (res == nullptr) res = CreateResource(kDeviceResourceInputLayout);

	InputLayout11Ptr ret = std::static_pointer_cast<InputLayout11>(res);
	ret->mInputDescs.resize(descArr.size());
	for (size_t i = 0; i < descArr.size(); ++i) {
		const LayoutInputElement& descI = descArr[i];
		ret->mInputDescs[i] = D3D11_INPUT_ELEMENT_DESC {
			descI.SemanticName.c_str(),
			descI.SemanticIndex,
			static_cast<DXGI_FORMAT>(descI.Format),
			descI.InputSlot,
			descI.AlignedByteOffset,
			static_cast<D3D11_INPUT_CLASSIFICATION>(descI.InputSlotClass),
			descI.InstanceDataStepRate
		};
	}

	auto resource = AsRes(pProgram);
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
		/*ContantBuffer11Ptr cbuffer11 = std::static_pointer_cast<ContantBuffer11>(buffer);
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);*/
		UpdateConstBuffer(std::static_pointer_cast<ContantBuffer11>(buffer), data, dataSize);
	}break;
	case kHWBufferVertex:{
		VertexBuffer11Ptr cbuffer11 = std::static_pointer_cast<VertexBuffer11>(buffer);
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	case kHWBufferIndex: {
		IndexBuffer11Ptr cbuffer11 = std::static_pointer_cast<IndexBuffer11>(buffer);
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	default:
		break;
	}
	return true;
}

ISamplerStatePtr RenderSystem11::LoadSampler(IResourcePtr res, SamplerFilterMode filter, CompareFunc cmpFunc)
{
	if (res == nullptr) res = CreateResource(kDeviceResourceSamplerState);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = static_cast<D3D11_FILTER>(filter);
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;//D3D11_TEXTURE_ADDRESS_MIRROR
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
	sampDesc.ComparisonFunc = static_cast<D3D11_COMPARISON_FUNC>(cmpFunc);
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* pSamplerLinear = nullptr;
	if (CheckHR(mDevice->CreateSamplerState(&sampDesc, &pSamplerLinear)))
		return nullptr;

	SamplerState11Ptr ret = std::static_pointer_cast<SamplerState11>(res);
	ret->Init(pSamplerLinear);
	return ret;
}

struct IncludeStdIo : public ID3DInclude
{
	std::string mModelPath;
	std::vector<char> mBuffer;
	std::vector<std::string> mStrBuffer;
public:
	IncludeStdIo(const std::string& modelPath) :mModelPath(modelPath) {}
	STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, 
		LPCVOID *ppData, UINT *pBytes) 
	{
		std::string fullPath = mModelPath + pFileName;
		FILE* fd = fopen(fullPath.c_str(), "r");
		if (fd) {
			fseek(fd, 0, SEEK_END);
			size_t size = ftell(fd);
			fseek(fd, 0, SEEK_SET);
			size_t first = mBuffer.size();
			mBuffer.resize(first + size);
			fread(&mBuffer[first], sizeof(char), size, fd);
			fclose(fd);

			mStrBuffer.push_back(std::string(mBuffer.begin(), mBuffer.end()));
			if (ppData) *ppData = mStrBuffer.back().c_str();
			if (pBytes) *pBytes = mStrBuffer.back().size();

			//OutputDebugStringA(mStrBuffer.back().c_str());
			return S_OK;
		}
		return S_FALSE;
	}
	STDMETHOD(Close)(THIS_ LPCVOID pData) {
		return S_OK;
	}
};
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
PixelShader11Ptr RenderSystem11::_CreatePS(const std::string& filename, const std::string& entry, bool async)
{
	PixelShader11Ptr ret = MakePtr<PixelShader11>(MakePtr<BlobData11>(nullptr));
	std::string psEntry = !entry.empty() ? entry : "PS";
	const char* shaderModel = "ps_4_0";
	DWORD dwShaderFlags = GetShaderFlag();

	HRESULT hr;
	if (async) {
		hr = mThreadPump->AddWorkItem(AsRes(ret), [&](ID3DX11ThreadPump* pump, ThreadPumpEntryPtr entry)->HRESULT {
			const D3D10_SHADER_MACRO* pDefines = nullptr;
			LPD3D10INCLUDE pInclude = new IncludeStdIo("shader\\");
			return D3DX11CompileFromFileA(filename.c_str(), pDefines, pInclude, psEntry.c_str(), 
				shaderModel, dwShaderFlags, 0, pump,
				&static_cast<BlobData11*>(PtrRaw(ret->mBlob))->mBlob, &ret->mErrBlob, (HRESULT*)&entry->hr);
		}, [=](IResource* res, HRESULT hr) {
			if (!FAILED(hr)) {
				assert(dynamic_cast<VertexShader11*>(res) && ret->mBlob);
				if (!CheckHR(mDevice->CreatePixelShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), 
					nullptr, &ret->mShader))) {
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
		hr = D3DX11CompileFromFileA(filename.c_str(), &mShaderMacros[0], NULL, psEntry.c_str(), 
			shaderModel, dwShaderFlags, 0, nullptr,
			&static_cast<BlobData11*>(PtrRaw(ret->mBlob))->mBlob, &ret->mErrBlob, NULL);
		if (CheckCompileError(hr, ret->mErrBlob)
			&& !CheckHR(mDevice->CreatePixelShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), 
				NULL, &ret->mShader))) {
			AsRes(ret)->SetLoaded();
		}
		else {
			ret = nullptr;
		}
	}
	return ret;
}

PixelShader11Ptr RenderSystem11::_CreatePSByFXC(const std::string& filename)
{
	PixelShader11Ptr ret = MakePtr<PixelShader11>(nullptr);
	std::vector<char> buffer = input::ReadFile(filename.c_str(), "rb");
	if (!buffer.empty()) {
		ret->mBlob = MakePtr<BlobDataStandard>(buffer);
		HRESULT hr = mDevice->CreatePixelShader(&buffer[0], buffer.size(), NULL, &ret->mShader);
		if (!FAILED(hr)) {
			AsRes(ret)->SetLoaded();
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

VertexShader11Ptr RenderSystem11::_CreateVS(const std::string& filename, const std::string& entry, bool async)
{
	VertexShader11Ptr ret = MakePtr<VertexShader11>(MakePtr<BlobData11>(nullptr));
	std::string vsEntry = !entry.empty() ? entry : "VS";
	const char* shaderModel = "vs_4_0";
	DWORD dwShaderFlags = GetShaderFlag();

	HRESULT hr;
	if (async) {
		ID3DX11DataProcessor* pProcessor = nullptr;
		ID3DX11DataLoader* pDataLoader = nullptr;
		const D3D10_SHADER_MACRO* pDefines = nullptr;
		LPD3D10INCLUDE pInclude = new IncludeStdIo("shader\\");
		if (!CheckHR(D3DX11CreateAsyncCompilerProcessor(filename.c_str(), pDefines, pInclude, vsEntry.c_str(), 
			shaderModel, dwShaderFlags, 0,
			&std::static_pointer_cast<BlobData11>(ret->mBlob)->mBlob, &ret->mErrBlob, &pProcessor))
			&& !CheckHR(D3DX11CreateAsyncFileLoaderA(filename.c_str(), &pDataLoader))) {
			hr = mThreadPump->AddWorkItem(AsRes(ret), pDataLoader, pProcessor, [=](IResource* res, HRESULT hr) {
				if (!FAILED(hr))  {
					BOOST_ASSERT(dynamic_cast<VertexShader11*>(res) && ret->mBlob);
					if (!CheckHR(mDevice->CreateVertexShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), 
						NULL, &ret->mShader))) {
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
		hr = D3DX11CompileFromFileA(filename.c_str(), &mShaderMacros[0], NULL, vsEntry.c_str(), 
			shaderModel, dwShaderFlags, 0, nullptr,
			&std::static_pointer_cast<BlobData11>(ret->mBlob)->mBlob, &ret->mErrBlob, NULL);
		if (CheckCompileError(hr, ret->mErrBlob)
			&& !CheckHR(mDevice->CreateVertexShader(ret->mBlob->GetBufferPointer(), ret->mBlob->GetBufferSize(), 
				NULL, &ret->mShader))) {
			AsRes(ret)->SetLoaded();
		}
		else {
			ret = nullptr;
		}
	}
	return ret;
}

VertexShader11Ptr RenderSystem11::_CreateVSByFXC(const std::string& filename)
{
	VertexShader11Ptr ret = MakePtr<VertexShader11>(nullptr);
	std::vector<char> buffer = input::ReadFile(filename.c_str(), "rb");
	if (!buffer.empty()) {
		ret->mBlob = MakePtr<BlobDataStandard>(buffer);
		auto buffer_size = buffer.size();
		HRESULT hr = mDevice->CreateVertexShader(&buffer[0], buffer_size, NULL, &ret->mShader);
		if (!FAILED(hr)) {
			AsRes(ret)->SetLoaded();
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

IProgramPtr RenderSystem11::CreateProgramByCompile(IResourcePtr res, const std::string& vsPath, const std::string& psPath, 
	const std::string& vsEntry, const std::string& psEntry)
{
	TIME_PROFILE2(CreateProgramByCompile, std::string(vsPath));

	Program11Ptr program = std::static_pointer_cast<Program11>(res);
	program->SetVertex(_CreateVS(vsPath, vsEntry, false));
	program->SetPixel(_CreatePS(!psPath.empty() ? psPath : vsPath, psEntry, false));
	AsRes(program)->CheckAndSetLoaded();
	return program;
}

IProgramPtr RenderSystem11::CreateProgramByFXC(IResourcePtr res, const std::string& name, const std::string& vsEntry, const std::string& psEntry)
{
	TIME_PROFILE2(CreateProgramByFXC, (name));
	Program11Ptr program = std::static_pointer_cast<Program11>(res);

	std::string vsEntryOrVS = !vsEntry.empty() ? vsEntry : "VS";
	std::string vsName = name + "_" + vsEntryOrVS + FILE_EXT_CSO;
	program->SetVertex(_CreateVSByFXC(vsName.c_str()));

	std::string psEntryOrPS = !psEntry.empty() ? psEntry : "PS";
	std::string psName = name + "_" + psEntryOrPS + FILE_EXT_CSO;
	program->SetPixel(_CreatePSByFXC(psName.c_str()));

	AsRes(program)->CheckAndSetLoaded();
	return program;
}

ID3D11Buffer* RenderSystem11::_CreateVertexBuffer(int bufferSize, void* buffer)
{
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
	if (CheckHR(mDevice->CreateBuffer(&bd, &InitData, &pVertexBuffer)))
		return nullptr;
	return pVertexBuffer;
}

ID3D11Buffer* RenderSystem11::_CreateVertexBuffer(int bufferSize)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = bufferSize;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Buffer* pVertexBuffer = nullptr;
	if (CheckHR(mDevice->CreateBuffer(&bd, nullptr, &pVertexBuffer)))
		return nullptr;
	return pVertexBuffer;
}

IVertexBufferPtr RenderSystem11::LoadVertexBuffer(IResourcePtr res, int bufferSize, int stride, int offset, void* buffer/*=nullptr*/)
{
	if (res == nullptr) res = CreateResource(kDeviceResourceVertexBuffer);

	IVertexBufferPtr vertexBuffer;
	if (buffer) vertexBuffer = MakePtr<VertexBuffer11>(_CreateVertexBuffer(bufferSize, buffer), bufferSize, stride, offset);
	else vertexBuffer = MakePtr<VertexBuffer11>(_CreateVertexBuffer(bufferSize), bufferSize, stride, offset);
	
	vertexBuffer->SetLoaded();
	return vertexBuffer;
}

void RenderSystem11::SetVertexBuffer(IVertexBufferPtr vertexBuffer)
{
	UINT stride = vertexBuffer->GetStride();
	UINT offset = vertexBuffer->GetOffset();
	mDeviceContext->IASetVertexBuffers(0, 1, &std::static_pointer_cast<VertexBuffer11>(vertexBuffer)->GetBuffer11(), &stride, &offset);
}

IIndexBufferPtr RenderSystem11::LoadIndexBuffer(IResourcePtr res, int bufferSize, ResourceFormat format, void* buffer)
{
	if (res == nullptr) res = CreateResource(kDeviceResourceIndexBuffer);

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
	if (CheckHR(mDevice->CreateBuffer(&bd, &InitData, &pIndexBuffer)))
		return nullptr;

	IndexBuffer11Ptr indexBuffer = std::static_pointer_cast<IndexBuffer11>(res);
	indexBuffer->Init(pIndexBuffer, bufferSize, format);
	indexBuffer->SetLoaded();
	return indexBuffer;
}

void RenderSystem11::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	if (indexBuffer) mDeviceContext->IASetIndexBuffer(
		std::static_pointer_cast<IndexBuffer11>(indexBuffer)->GetBuffer11(), 
		static_cast<DXGI_FORMAT>(indexBuffer->GetFormat()),
		0);
	else mDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
}

IContantBufferPtr RenderSystem11::LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, void* data)
{
	if (res == nullptr) res = CreateResource(kDeviceResourceContantBuffer);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = cbDecl.BufferSize % sizeof(Eigen::Vector4f) == 0 
		? cbDecl.BufferSize 
		: (cbDecl.BufferSize / sizeof(Eigen::Vector4f) + 1) * sizeof(Eigen::Vector4f);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	
	ID3D11Buffer* pConstantBuffer = nullptr;
	if (CheckHR(mDevice->CreateBuffer(&bd, NULL, &pConstantBuffer)))
		return nullptr;
	
	ConstBufferDeclPtr declPtr = std::make_shared<ConstBufferDecl>(cbDecl);
	ContantBuffer11Ptr ret = std::static_pointer_cast<ContantBuffer11>(res);
	ret->Init(pConstantBuffer, declPtr);
	if (data) 
		UpdateConstBuffer(ret, data, ret->GetBufferSize());
	return ret;
}

void RenderSystem11::UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize)
{
	mDeviceContext->UpdateSubresource(std::static_pointer_cast<ContantBuffer11>(buffer)->GetBuffer11(), 0, NULL, data, 0, 0);
}

namespace il_helper {
	static const ILenum CSupportILTypes[] = {
		IL_PNG, IL_JPG, IL_JP2, IL_BMP, IL_TGA, IL_DDS, IL_GIF, IL_HDR, IL_ICO
	};
	static inline ILenum DetectType(const void* pData, int dataSize) {
		for (int i = 0; i < sizeof(CSupportILTypes) / sizeof(CSupportILTypes[0]); ++i)
			if (ilIsValidL(CSupportILTypes[i], (void*)pData, dataSize))
				return CSupportILTypes[i];
		return IL_TYPE_UNKNOWN;
	}
	static ILenum DetectType(FILE* fd) {
		for (int i = 0; i < sizeof(CSupportILTypes) / sizeof(CSupportILTypes[0]); ++i)
			if (ilIsValidF(CSupportILTypes[i], fd))
				return CSupportILTypes[i];
		return IL_TYPE_UNKNOWN;
	}
};

#if 0
ITexturePtr RenderSystem11::LoadTexture(IResourcePtr res, const std::string& filepath, 
	ResourceFormat format, bool async, bool isCube)
{
	if (res == nullptr) res = CreateResource(kDeviceResourceTexture);

	ITexturePtr ret = nullptr;
	boost::filesystem::path fullpath = boost::filesystem::system_complete(filepath);
	if (boost::filesystem::exists(fullpath)) {
		std::string imgFullpath = fullpath.string();

		static D3DX11_IMAGE_LOAD_INFO LoadInfo = {};
		LoadInfo.Format = static_cast<DXGI_FORMAT>(format);
		D3DX11_IMAGE_LOAD_INFO* pLoadInfo = format != kFormatUnknown ? &LoadInfo : nullptr;

		ITexturePtr texture = std::static_pointer_cast<Texture11>(res);
		IResourcePtr resource = AsRes(texture);
		if (!async) {
			if (!CheckHR(mThreadPump->AddWorkItem(resource, [=](ID3DX11ThreadPump* pump, ThreadPumpEntryPtr entry)->HRESULT {
				return D3DX11CreateShaderResourceViewFromFileA(mDevice, imgFullpath.c_str(), pLoadInfo, pump,
					&std::static_pointer_cast<Texture11>(texture)->GetSRV11(), nullptr);
			}, ResourceSetLoaded))) {
				ret = texture;
			}
		}
		else {
			FILE* fd = fopen(imgFullpath.c_str(), "rb");
			BOOST_ASSERT(fd);
			if (fd) {
				ILuint image = ilGenImage();
				ilBindImage(image);

				ILenum ilType = il_helper::DetectType(fd);
				if (ilType != IL_TYPE_UNKNOWN && ilLoadF(ilType, fd)) {
					ILuint width = ilGetInteger(IL_IMAGE_WIDTH), height = ilGetInteger(IL_IMAGE_HEIGHT);
					
					std::vector<unsigned char> bytes(width * height * 4);
					ilCopyPixels(0, 0, 0, width, height, 1, IL_RGBA, IL_UNSIGNED_BYTE, &bytes[0]);

					constexpr ResourceFormat bytes_format = kFormatR8G8B8A8UNorm;

					LoadTexture(texture, bytes_format, Eigen::Vector4i(width, height, 0, 1), 1, 
						&Data::Make(&bytes[0], bytes.size()));
					ret = texture;
				}

				ilDeleteImage(image);
				fclose(fd);
			}
		}
	}
	else {
		MessageBoxA(0, (boost::format("image file %s not exist\n") % filepath).str().c_str(), "", MB_OK);
	}
	return ret;
}
#endif

ITexturePtr RenderSystem11::LoadTexture(IResourcePtr res, ResourceFormat format, 
	const Eigen::Vector4i& size, int mipCount, const Data datas[])
{
	Texture11Ptr texture = std::static_pointer_cast<Texture11>(res);
	texture->Init(format, size.x(), size.y(), size.w(), mipCount);

	Data defaultData = Data{};
	if (datas == nullptr)
		datas = &defaultData;

	mipCount = texture->GetMipmapCount();
	const size_t faceCount = texture->GetFaceCount();
	BOOST_ASSERT(faceCount == 1 || datas[0].Bytes);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = texture->GetWidth();
	desc.Height = texture->GetHeight();
	desc.MipLevels = mipCount;
	desc.ArraySize = faceCount;
	desc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (datas[0].Bytes) {
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = (faceCount > 1) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
	}
	else {
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
	}

	std::vector<D3D11_SUBRESOURCE_DATA> initDatas(mipCount * faceCount, {});
	for (size_t face = 0; face < faceCount; ++face) {
		for (size_t mip = 0; mip < mipCount; ++mip) {
			size_t index = face * mipCount + mip;
			const Data& data = datas[index];
			D3D11_SUBRESOURCE_DATA& res_data = initDatas[index];
			res_data.pSysMem = data.Bytes;
			res_data.SysMemPitch = data.Size ? data.Size : d3d::BytePerPixel(desc.Format) * (desc.Width >> mip);//Line width in bytes
			res_data.SysMemSlicePitch = 0; //only used for 3d textures
		}
	}

	ID3D11Texture2D *pTexture = NULL;
	if (CheckHR(mDevice->CreateTexture2D(&desc, datas[0].Bytes ? &initDatas[0] : nullptr, &pTexture))) return nullptr;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
	srvDesc.ViewDimension = (texture->GetFaceCount() > 1) ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texture->GetMipmapCount();

	if (CheckHR(mDevice->CreateShaderResourceView(pTexture, &srvDesc, 
		&std::static_pointer_cast<Texture11>(texture)->GetSRV11()))) return nullptr; 
	
	texture->SetLoaded();
	return texture;
}
bool RenderSystem11::LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep)
{
	BOOST_ASSERT(dataStep * texture->GetHeight() <= dataSize);

	D3D11_SUBRESOURCE_DATA initData = { data, dataStep, 0 };

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = texture->GetWidth();
	desc.Height = texture->GetHeight();
	desc.MipLevels = desc.ArraySize = texture->GetMipmapCount();
	desc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	
	ComPtr<ID3D11Texture2D> tex;
	HRESULT hr = mDevice->CreateTexture2D(&desc, &initData, tex.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = texture->GetMipmapCount();

		ID3D11ShaderResourceView* texSRV;
		hr = mDevice->CreateShaderResourceView(tex.Get(), &SRVDesc, &texSRV);
		if (SUCCEEDED(hr)) 
		{
			Texture11Ptr tex11 = std::static_pointer_cast<Texture11>(texture);
			tex11->SetSRV11(texSRV);

			AsRes(tex11)->CheckAndSetLoaded();
		}
	}

	return SUCCEEDED(hr);
}

void RenderSystem11::SetBlendFunc(const BlendState& blendFunc)
{
	mCurBlendFunc = blendFunc;

	D3D11_BLEND_DESC blendDesc = { 0 };
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = static_cast<D3D11_BLEND>(blendFunc.Src);
	blendDesc.RenderTarget[0].DestBlend = static_cast<D3D11_BLEND>(blendFunc.Dst);
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (CheckHR(mDevice->CreateBlendState(&blendDesc, &mBlendState))) return;

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	mDeviceContext->OMSetBlendState(mBlendState, blendFactor, 0xffffffff);
}

void RenderSystem11::SetDepthState(const DepthState& depthState)
{
	mCurDepthState = depthState;

	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory(&DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	DSDesc.DepthEnable = depthState.DepthEnable;
	DSDesc.DepthWriteMask = static_cast<D3D11_DEPTH_WRITE_MASK>(depthState.WriteMask);
	DSDesc.DepthFunc = static_cast<D3D11_COMPARISON_FUNC>(depthState.CmpFunc);
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

void RenderSystem11::DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	//if (_CanDraw())
	mDeviceContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topo));
	mDeviceContext->Draw(op.mVertexBuffer->GetBufferSize() / op.mVertexBuffer->GetStride(), 0);
}
void RenderSystem11::DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	//if (_CanDraw())
	mDeviceContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topo));
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