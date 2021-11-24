#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/if.hpp>
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
#include "core/resource/material_factory.h"
#include "core/renderable/renderable.h"

using Microsoft::WRL::ComPtr;

namespace mir {

#define MakePtr std::make_shared
#define PtrRaw(T) T.get()

RenderSystem11::RenderSystem11()
{
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

	mScreenSize.x() = vpWidth;
	mScreenSize.y() = vpHeight;
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
{}

void RenderSystem11::CleanUp()
{}

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
		return MakePtr<Texture11>();
	case mir::kDeviceResourceRenderTexture:
		return MakePtr<RenderTexture11>();
	case mir::kDeviceResourceSamplerState:
		return MakePtr<SamplerState11>();
	default:
		break;
	}
	return nullptr;
}

IRenderTexturePtr RenderSystem11::LoadRenderTexture(IResourcePtr res, const Eigen::Vector2i& size, ResourceFormat format)
{
	BOOST_ASSERT(res);
	RenderTexture11Ptr ret = std::static_pointer_cast<RenderTexture11>(res);
	ret->Init(mDevice, size, format);
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
	BOOST_ASSERT(res && AsRes(pProgram)->IsLoaded());

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

	ret->mLayout = _CreateInputLayout(std::static_pointer_cast<Program11>(pProgram).get(), ret->mInputDescs);
	return ret;
}
void RenderSystem11::SetVertexLayout(IInputLayoutPtr layout) 
{
	mDeviceContext->IASetInputLayout(std::static_pointer_cast<InputLayout11>(layout)->GetLayout11());
}

#define CBufferUsage D3D11_USAGE_DEFAULT;

bool RenderSystem11::UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize)
{
	BOOST_ASSERT(buffer);

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	switch (buffer->GetType())
	{
	case kHWBufferConstant: {
		ContantBuffer11Ptr cbuffer11 = std::static_pointer_cast<ContantBuffer11>(buffer);
	#if CBufferUsage == D3D11_USAGE_DEFAULT
		mDeviceContext->UpdateSubresource(cbuffer11->GetBuffer11(), 0,
			NULL, data, 0, 0);
	#else
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0,
			D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	#endif
	}break;
	case kHWBufferVertex:{
		VertexBuffer11Ptr cbuffer11 = std::static_pointer_cast<VertexBuffer11>(buffer);
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, 
			D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return false;
		memcpy(MappedResource.pData, data, dataSize);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	case kHWBufferIndex: {
		IndexBuffer11Ptr cbuffer11 = std::static_pointer_cast<IndexBuffer11>(buffer);
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, 
			D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return false;
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
	BOOST_ASSERT(res);

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

IBlobDataPtr RenderSystem11::CompileShader(const ShaderCompileDesc& compile, const Data& data)
{
	BlobData11Ptr blob = MakePtr<BlobData11>(nullptr);

	std::vector<D3D_SHADER_MACRO> shaderMacros(compile.Macros.size());
	if (!compile.Macros.empty()) {
		for (size_t i = 0; i < compile.Macros.size(); ++i) {
			const auto& cdm = compile.Macros[i];
			shaderMacros[i] = D3D_SHADER_MACRO{ cdm.Name.c_str(), cdm.Definition.c_str() };
		}
		shaderMacros.push_back(D3D_SHADER_MACRO{ NULL, NULL });
	}

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG) && defined(D3D11_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* blobError = nullptr;
	HRESULT hr = D3DCompile(data.Bytes, data.Size, compile.SourcePath.c_str(),
		shaderMacros.empty() ? nullptr : &shaderMacros[0], D3D_COMPILE_STANDARD_FILE_INCLUDE,
		compile.EntryPoint.c_str(), compile.ShaderModel.c_str(),
		shaderFlags, 0,
		&blob->mBlob, &blobError);
	if (debug::CheckCompileFailed(hr, blobError)) return nullptr;

	return blob;
}
IShaderPtr RenderSystem11::CreateShader(ShaderType type, const ShaderCompileDesc& desc, IBlobDataPtr data)
{
	switch (type) {
	case kShaderVertex: {
		VertexShader11Ptr ret = MakePtr<VertexShader11>(data);
		if (debug::CheckCompileFailed(
			mDevice->CreateVertexShader(data->GetBufferPointer(), data->GetBufferSize(), NULL, &ret->mShader), data))
			return nullptr;
		return ret;
	}break;
	case kShaderPixel: {
		PixelShader11Ptr ret = MakePtr<PixelShader11>(data);
		if (debug::CheckCompileFailed(
			mDevice->CreatePixelShader(data->GetBufferPointer(), data->GetBufferSize(), NULL, &ret->mShader), data)) 
			return nullptr;
		return ret;
	}break;
	default:
		break;
	}
	return nullptr;
}
IProgramPtr RenderSystem11::LoadProgram(IResourcePtr res, const std::vector<IShaderPtr>& shaders)
{
	Program11Ptr program = std::static_pointer_cast<Program11>(res);
	for (auto& iter : shaders) {
		switch (iter->GetType()) {
		case kShaderVertex:
			program->SetVertex(std::static_pointer_cast<VertexShader11>(iter));
		case kShaderPixel:
			program->SetPixel(std::static_pointer_cast<PixelShader11>(iter));
		default:
			break;
		}
	}
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
	BOOST_ASSERT(res);

	VertexBuffer11Ptr ret = std::static_pointer_cast<VertexBuffer11>(res);
	if (buffer) ret->Init(_CreateVertexBuffer(bufferSize, buffer), bufferSize, stride, offset);
	else ret->Init(_CreateVertexBuffer(bufferSize), bufferSize, stride, offset);
	
	return ret;
}

void RenderSystem11::SetVertexBuffer(IVertexBufferPtr vertexBuffer)
{
	UINT stride = vertexBuffer->GetStride();
	UINT offset = vertexBuffer->GetOffset();
	mDeviceContext->IASetVertexBuffers(0, 1, &std::static_pointer_cast<VertexBuffer11>(vertexBuffer)->GetBuffer11(), &stride, &offset);
}

IIndexBufferPtr RenderSystem11::LoadIndexBuffer(IResourcePtr res, int bufferSize, ResourceFormat format, void* buffer)
{
	BOOST_ASSERT(res);

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

	IndexBuffer11Ptr ret = std::static_pointer_cast<IndexBuffer11>(res);
	ret->Init(pIndexBuffer, bufferSize, format);
	return ret;
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
	BOOST_ASSERT(res);

	D3D11_BUFFER_DESC cbDesc = {};
#if CBufferUsage == D3D11_USAGE_DEFAULT
	cbDesc.ByteWidth = (cbDecl.BufferSize + 15) / 16 * 16;
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
#else
	cbDesc.ByteWidth = (cbDecl.BufferSize + 15) / 16 * 16;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
#endif
	ID3D11Buffer* pConstantBuffer = nullptr;
	if (CheckHR(mDevice->CreateBuffer(&cbDesc, NULL, &pConstantBuffer))) return nullptr;
	
	ContantBuffer11Ptr ret = std::static_pointer_cast<ContantBuffer11>(res);
	ret->Init(pConstantBuffer, std::make_shared<ConstBufferDecl>(cbDecl));
	if (data) UpdateBuffer(ret, data, ret->GetBufferSize());
	return ret;
}

ITexturePtr RenderSystem11::LoadTexture(IResourcePtr res, ResourceFormat format, 
	const Eigen::Vector4i& size/*w_h_step_face*/, int mipCount, const Data datas[])
{
	Texture11Ptr texture = std::static_pointer_cast<Texture11>(res);
	texture->Init(format, size.x(), size.y(), size.w(), mipCount);

	Data defaultData = Data{};
	if (datas == nullptr)
		datas = &defaultData;

	mipCount = texture->GetMipmapCount();
	const bool autoGen = texture->IsAutoGenMipmap();
	const size_t faceCount = texture->GetFaceCount();
	constexpr int imageSize = 0;//only used for 3d textures
	BOOST_ASSERT_IF_THEN(faceCount > 1, datas[0].Bytes);
	BOOST_ASSERT_IF_THEN(autoGen, datas[0].Bytes && faceCount == 1);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = texture->GetWidth();
	desc.Height = texture->GetHeight();
	desc.MipLevels = autoGen ? 0 : mipCount;
	desc.ArraySize = faceCount;
	desc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (datas[0].Bytes) {
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		if (autoGen) {
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
		else if (faceCount > 1) {
			desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		}
		else {
			desc.MiscFlags = 0;
		}
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
			res_data.SysMemSlicePitch = imageSize; //only used for 3d textures
		}
	}

	ID3D11Texture2D *pTexture = NULL;
	if (CheckHR(mDevice->CreateTexture2D(&desc, (datas[0].Bytes && !autoGen) ? &initDatas[0] : nullptr, &pTexture))) return nullptr;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
	srvDesc.ViewDimension = (faceCount > 1) ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = autoGen ? -1 : texture->GetMipmapCount();

	if (CheckHR(mDevice->CreateShaderResourceView(pTexture, &srvDesc, 
		&texture->GetSRV11()))) return nullptr; 
	
	if (autoGen) {
		mDeviceContext->UpdateSubresource(pTexture, 0, nullptr, 
			datas[0].Bytes, datas[0].Size, imageSize);
		mDeviceContext->GenerateMips(texture->GetSRV11());
	}

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