#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/if.hpp>
#include <windows.h>
#include <dxerr.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <IL/il.h>
#include "core/mir_config.h"
#include "core/base/d3d.h"
#include "core/base/debug.h"
#include "core/base/input.h"
#include "core/base/macros.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d11/blob11.h"
#include "core/rendersys/d3d11/program11.h"
#include "core/rendersys/d3d11/input_layout11.h"
#include "core/rendersys/d3d11/hardware_buffer11.h"
#include "core/rendersys/d3d11/texture11.h"
#include "core/rendersys/d3d11/framebuffer11.h"
#include "core/resource/material_factory.h"
#include "core/renderable/renderable.h"

using Microsoft::WRL::ComPtr;

namespace mir {

#define MakePtr CreateInstance
#define PtrRaw(T) T.get()

RenderSystem11::RenderSystem11()
{}
RenderSystem11::~RenderSystem11()
{}

bool RenderSystem11::_CreateDeviceAndSwapChain(int width, int height)
{
	uint32_t createDeviceFlags = 0;
#if defined MIR_D3D11_DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	std::array<D3D_DRIVER_TYPE, 3> driverTypes = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	std::array<D3D_FEATURE_LEVEL,3> featureLevels = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	DXGI_SWAP_CHAIN_DESC sd = {};
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
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	for (size_t i = 0; i < driverTypes.size(); i++) {
		if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(NULL, driverTypes[i], NULL, createDeviceFlags,
			featureLevels.data(), featureLevels.size(), D3D11_SDK_VERSION, &sd,
			&mSwapChain, &mDevice, &mFeatureLevel, &mDeviceContext)))
			return true;
	}
	return false;
}
bool RenderSystem11::_FetchBackFrameBufferColor(int width, int height)
{
	ID3D11Texture2D* pBackBuffer = NULL;
	if (CheckHR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer))) return false;

	ID3D11RenderTargetView* pRTV = nullptr;
	if (CheckHR(mDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRTV))) return false;

	mBackFrameBuffer = CreateInstance<FrameBuffer11>();
	mBackFrameBuffer->SetSize(Eigen::Vector2i(width, height));
	mBackFrameBuffer->SetAttachColor(0, CreateInstance<FrameBufferAttachByView>(pRTV));
	mCurFrameBuffer = mBackFrameBuffer;
	return true;
}
bool RenderSystem11::_FetchBackBufferZStencil(int width, int height)
{
	D3D11_TEXTURE2D_DESC descDepth = {};
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
	if (CheckHR(mDevice->CreateTexture2D(&descDepth, NULL, &mDepthStencil))) return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	ID3D11DepthStencilView* pDSV = nullptr;
	if (CheckHR(mDevice->CreateDepthStencilView(mDepthStencil, &descDSV, &pDSV))) return false;

	mBackFrameBuffer->SetAttachZStencil(CreateInstance<FrameBufferAttachByView>(pDSV));
	
#if defined MIR_RESOURCE_DEBUG
	for (auto rtv : mBackFrameBuffer->AsRTVs())
		DEBUG_RES_ADD_DEVICE(mBackFrameBuffer, rtv);
	DEBUG_RES_ADD_DEVICE(mBackFrameBuffer, mBackFrameBuffer->AsDSV());
	DEBUG_SET_PRIV_DATA(mBackFrameBuffer, "device.backbuffer");
#endif
	return true;
}
bool RenderSystem11::_SetRasterizerState()
{
	D3D11_RASTERIZER_DESC wfdesc = {};
	wfdesc.FillMode = D3D11_FILL_SOLID;
	wfdesc.CullMode = D3D11_CULL_BACK;
	ID3D11RasterizerState* pRasterizerState = nullptr;
	if (CheckHR(mDevice->CreateRasterizerState(&wfdesc, &pRasterizerState))) return false;

	mDeviceContext->RSSetState(pRasterizerState);
	return true;
}
bool RenderSystem11::Initialize(HWND hWnd, RECT vp)
{
	mHWnd = hWnd;

	RECT rc;
	GetClientRect(mHWnd, &rc);
	UINT rcWidth = rc.right - rc.left;
	UINT rcHeight = rc.bottom - rc.top;

	if (!_CreateDeviceAndSwapChain(rcWidth, rcHeight)) return false;

	if (!_FetchBackFrameBufferColor(rcWidth, rcHeight) 
	 || !_FetchBackBufferZStencil(rcWidth, rcHeight)) return false;
	SetFrameBuffer(nullptr);

	if (!_SetRasterizerState()) return false;
	
	SetDepthState(DepthState{ true, kCompareLessEqual, kDepthWriteMaskAll });
	SetBlendFunc(BlendState::MakeAlphaPremultiplied());

	if (vp.right == 0 || vp.bottom == 0) 
		GetClientRect(mHWnd, &vp);
	mScreenSize.x() = vp.right - vp.left;
	mScreenSize.y() = vp.bottom - vp.top;
	SetViewPort(vp.left, vp.top, mScreenSize.x(), mScreenSize.y());
	return true;
}

void RenderSystem11::Update(float dt)
{}
void RenderSystem11::Dispose()
{}

void RenderSystem11::SetViewPort(int x, int y, int width, int height)
{
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = x;
	vp.TopLeftY = y;
	mDeviceContext->RSSetViewports(1, &vp);
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
	case mir::kDeviceResourceFrameBuffer:
		return MakePtr<FrameBuffer11>();
	case mir::kDeviceResourceSamplerState:
		return MakePtr<SamplerState11>();
	default:
		break;
	}
	return nullptr;
}

/********** LoadFrameBuffer **********/
static Texture11Ptr _CreateColorAttachTexture(ID3D11Device* pDevice, const Eigen::Vector2i& size, ResourceFormat format)
{
	Texture11Ptr texture = CreateInstance<Texture11>();
	texture->Init(format, kHWUsageDefault, size.x(), size.y(), 1, 1);

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = size.x();
	texDesc.Height = size.y();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = static_cast<DXGI_FORMAT>(format);
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* pTexture = nullptr;
	if (CheckHR(pDevice->CreateTexture2D(&texDesc, NULL, &pTexture))) return nullptr;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = static_cast<DXGI_FORMAT>(format);
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	if (CheckHR(pDevice->CreateShaderResourceView(pTexture, &srvDesc, &texture->AsSRV()))) return nullptr;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = static_cast<DXGI_FORMAT>(format);
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	if (CheckHR(pDevice->CreateRenderTargetView(pTexture, &rtvDesc, &texture->AsRTV()))) return nullptr;

	texture->SetLoaded();
	return texture;
}
static Texture11Ptr _CreateZStencilAttachTexture(ID3D11Device* pDevice, const Eigen::Vector2i& size, ResourceFormat format)
{
	BOOST_ASSERT(d3d::IsDepthStencil(static_cast<DXGI_FORMAT>(format)));
	BOOST_ASSERT(format == kFormatD24UNormS8UInt 
		|| format == kFormatD32Float
		|| format == kFormatD16UNorm);

	constexpr bool autoGen = false;
	constexpr size_t mipCount = 1;
	
	const DXGI_FORMAT dsvFmt = static_cast<DXGI_FORMAT>(format);
	const DXGI_FORMAT texFmt = (format == kFormatD24UNormS8UInt) ? DXGI_FORMAT_R24G8_TYPELESS : dsvFmt;
	const DXGI_FORMAT srvFmt = (format == kFormatD24UNormS8UInt) ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : dsvFmt;

	Texture11Ptr texture = CreateInstance<Texture11>();
	texture->Init(kFormatD24UNormS8UInt, kHWUsageDefault, size.x(), size.y(), 1, 1);

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = size.x();
	texDesc.Height = size.y();
	texDesc.MipLevels = autoGen ? 0 : mipCount;
	texDesc.ArraySize = 1;
	texDesc.Format = texFmt;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* pTexture = nullptr;
	if (CheckHR(pDevice->CreateTexture2D(&texDesc, NULL, &pTexture))) return nullptr;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = dsvFmt;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	if (CheckHR(pDevice->CreateDepthStencilView(pTexture, &dsvDesc, &texture->AsDSV()))) return nullptr;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = srvFmt;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = autoGen ? -1 : mipCount;
	if (CheckHR(pDevice->CreateShaderResourceView(pTexture, &srvDesc, &texture->AsSRV()))) return nullptr;

	texture->SetLoaded();
	return texture;
}
static FrameBufferAttachByTexture11Ptr _CreateFrameBufferAttachColor(ID3D11Device* pDevice, 
	const Eigen::Vector2i& size, ResourceFormat format)
{
	return format == kFormatUnknown ? nullptr : CreateInstance<FrameBufferAttachByTexture11>(
		_CreateColorAttachTexture(pDevice, size, format));
}
static FrameBufferAttachByTexture11Ptr _CreateFrameBufferAttachZStencil(ID3D11Device* pDevice, 
	const Eigen::Vector2i& size, ResourceFormat format) 
{
	return format == kFormatUnknown ? nullptr : CreateInstance<FrameBufferAttachByTexture11>(
		_CreateZStencilAttachTexture(pDevice, size, format));
}
IFrameBufferPtr RenderSystem11::LoadFrameBuffer(IResourcePtr res, const Eigen::Vector2i& size, const std::vector<ResourceFormat>& formats)
{
	BOOST_ASSERT(res);
	BOOST_ASSERT(formats.size() >= 1);
	BOOST_ASSERT(d3d::IsDepthStencil(static_cast<DXGI_FORMAT>(formats.back())) || formats.back() == kFormatUnknown);

	FrameBuffer11Ptr framebuffer = std::static_pointer_cast<FrameBuffer11>(res);
	framebuffer->SetSize(size);
	for (size_t i = 0; i + 1 < formats.size(); ++i)
		framebuffer->SetAttachColor(i, _CreateFrameBufferAttachColor(mDevice, size, formats[i]));
	framebuffer->SetAttachZStencil(_CreateFrameBufferAttachZStencil(mDevice, size, formats.size() >= 2 ? formats.back() : kFormatUnknown));

#if defined MIR_RESOURCE_DEBUG
	for (auto rtv : framebuffer->AsRTVs())
		DEBUG_RES_ADD_DEVICE(framebuffer, rtv);
	for (auto srv : framebuffer->AsSRVs())
		DEBUG_RES_ADD_DEVICE(framebuffer, srv);
	DEBUG_RES_ADD_DEVICE(framebuffer, framebuffer->AsDSV());
#endif
	return framebuffer;
}
void RenderSystem11::ClearFrameBuffer(IFrameBufferPtr fb, const Eigen::Vector4f& color, float depth, uint8_t stencil)
{
	FrameBuffer11Ptr fb11 = fb ? std::static_pointer_cast<FrameBuffer11>(fb) : mCurFrameBuffer;
	for (auto& rtv : fb11->AsRTVs()) {
		if (rtv) 
			mDeviceContext->ClearRenderTargetView(rtv, (const float*)&color);
	}
	if (fb11->AsDSV()) 
		mDeviceContext->ClearDepthStencilView(fb11->AsDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
}
void RenderSystem11::SetFrameBuffer(IFrameBufferPtr fb)
{
	ID3D11ShaderResourceView* TextureNull = nullptr;
	mDeviceContext->PSSetShaderResources(0, 1, &TextureNull);

	auto prevFbSize = mCurFrameBuffer->GetSize();

	auto fb11 = std::static_pointer_cast<FrameBuffer11>(fb);
	mCurFrameBuffer = fb11 ? fb11 : mBackFrameBuffer;

	if (mCurFrameBuffer->GetSize() != prevFbSize) {
		auto fbsize = mCurFrameBuffer->GetSize();
		SetViewPort(0, 0, fbsize.x(), fbsize.y());
	}

	std::vector<ID3D11RenderTargetView*> vecRTV = mCurFrameBuffer->AsRTVs();
	mDeviceContext->OMSetRenderTargets(vecRTV.size(), 
		vecRTV.empty() ? nullptr : &vecRTV[0], 
		mCurFrameBuffer->AsDSV());
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

	Program11Ptr program11 = std::static_pointer_cast<Program11>(pProgram);
	auto programBlob = program11->mVertex->GetBlob();
	if (CheckHR(mDevice->CreateInputLayout(&ret->mInputDescs[0], ret->mInputDescs.size(),
		programBlob->GetBytes(), programBlob->GetSize(), &ret->mLayout))) 
		return nullptr;

	return ret;
}
void RenderSystem11::SetVertexLayout(IInputLayoutPtr layout) 
{
	mDeviceContext->IASetInputLayout(std::static_pointer_cast<InputLayout11>(layout)->GetLayout11());
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
#if defined(MIR_D3D11_DEBUG)
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
IShaderPtr RenderSystem11::CreateShader(int type, IBlobDataPtr data)
{
#if 0
	ID3D11ShaderReflection* pReflector = NULL;
	D3DReflect(data->GetBytes(), data->GetSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);

	D3D11_SHADER_INPUT_BIND_DESC bindDesc;
	pReflector->GetResourceBindingDescByName("cbPerLight", &bindDesc);
#endif
	switch (type) {
	case kShaderVertex: {
		VertexShader11Ptr ret = MakePtr<VertexShader11>(data);
		if (debug::CheckCompileFailed(
			mDevice->CreateVertexShader(data->GetBytes(), data->GetSize(), NULL, &ret->mShader), data))
			return nullptr;		
		return ret;
	}break;
	case kShaderPixel: {
		PixelShader11Ptr ret = MakePtr<PixelShader11>(data);
		if (debug::CheckCompileFailed(
			mDevice->CreatePixelShader(data->GetBytes(), data->GetSize(), NULL, &ret->mShader), data)) 
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
	BOOST_ASSERT(res);

	Program11Ptr program = std::static_pointer_cast<Program11>(res);
	for (auto& iter : shaders) {
		switch (iter->GetType()) {
		case kShaderVertex:
			program->SetVertex(std::static_pointer_cast<VertexShader11>(iter));
			break;
		case kShaderPixel:
			program->SetPixel(std::static_pointer_cast<PixelShader11>(iter));
			break;
		default:
			break;
		}
	}
	
	DEBUG_RES_ADD_DEVICE(program, NULLABLE(program->mVertex, GetShader11()));
	DEBUG_RES_ADD_DEVICE(program, NULLABLE(program->mPixel, GetShader11()));
	return program;
}
void RenderSystem11::SetProgram(IProgramPtr program)
{
	mDeviceContext->VSSetShader(NULLABLE(std::static_pointer_cast<VertexShader11>(program->GetVertex()), GetShader11()), NULL, 0);
	mDeviceContext->PSSetShader(NULLABLE(std::static_pointer_cast<PixelShader11>(program->GetPixel()), GetShader11()), NULL, 0);
}

IVertexBufferPtr RenderSystem11::LoadVertexBuffer(IResourcePtr res, int stride, int offset, const Data& data)
{
	BOOST_ASSERT(res);

	HWMemoryUsage usage = data.Bytes ? kHWUsageImmutable : kHWUsageDynamic;

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = data.Size;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	if (usage == kHWUsageDefault) {
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;
	}
	else {
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = data.Bytes;

	ID3D11Buffer* pVertexBuffer = nullptr;
	if (CheckHR(mDevice->CreateBuffer(&bd, data.Bytes ? &InitData : nullptr, &pVertexBuffer))) return nullptr;

	VertexBuffer11Ptr ret = std::static_pointer_cast<VertexBuffer11>(res);
	ret->Init(pVertexBuffer, data.Size, usage, stride, offset);
	return ret;
}
void RenderSystem11::SetVertexBuffers(size_t slot, const IVertexBufferPtr vertexBuffers[], size_t count)
{
	BOOST_ASSERT(count >= 1);
	if (count == 1) {
		auto vertexBuffer = vertexBuffers[0];
		uint32_t stride = vertexBuffer->GetStride();
		uint32_t offset = vertexBuffer->GetOffset();
		mDeviceContext->IASetVertexBuffers(slot, count, 
			&std::static_pointer_cast<VertexBuffer11>(vertexBuffer)->GetBuffer11(), &stride, &offset);
	}
	else {
		std::vector<uint32_t> strides(count), offsets(count);
		std::vector<ID3D11Buffer*> buffer11s(count);
		for (size_t i = 0; i < count; ++i) {
			auto vertexBuffer = vertexBuffers[i];
			strides[i] = vertexBuffer->GetStride();
			offsets[i] = vertexBuffer->GetOffset();
			buffer11s[i] = std::static_pointer_cast<VertexBuffer11>(vertexBuffer)->GetBuffer11();
		}
		mDeviceContext->IASetVertexBuffers(slot, count, &buffer11s[0], &strides[0], &offsets[0]);
	}
}

IIndexBufferPtr RenderSystem11::LoadIndexBuffer(IResourcePtr res, ResourceFormat format, const Data& data)
{
	BOOST_ASSERT(res);

	HWMemoryUsage usage = data.Bytes ? kHWUsageImmutable : kHWUsageDynamic;

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = data.Size;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	if (usage == kHWUsageDynamic) {
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else {
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.CPUAccessFlags = 0;
	}

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = data.Bytes;
	
	ID3D11Buffer* pIndexBuffer = nullptr;
	if (CheckHR(mDevice->CreateBuffer(&bd, data.Bytes ? &InitData : nullptr, &pIndexBuffer))) return nullptr;

	IndexBuffer11Ptr ret = std::static_pointer_cast<IndexBuffer11>(res);
	ret->Init(pIndexBuffer, data.Size, format, usage);
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

IContantBufferPtr RenderSystem11::LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, HWMemoryUsage usage, const Data& data)
{
	BOOST_ASSERT(res);
	ContantBuffer11Ptr ret = std::static_pointer_cast<ContantBuffer11>(res);

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = (cbDecl.BufferSize + 15) / 16 * 16;
	cbDesc.Usage = static_cast<D3D11_USAGE>(usage);
	if (usage == kHWUsageDefault || usage == kHWUsageImmutable) {
		BOOST_ASSERT(data.Bytes);
		cbDesc.CPUAccessFlags = 0;
	}
	else if (usage == kHWUsageDynamic) {
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else {
		BOOST_ASSERT(false);
	}

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = data.Bytes;

	ID3D11Buffer* pConstantBuffer = nullptr;
	if (CheckHR(mDevice->CreateBuffer(&cbDesc, data.Bytes ? &initData : nullptr, &pConstantBuffer))) return nullptr;
	
	ret->Init(pConstantBuffer, CreateInstance<ConstBufferDecl>(cbDecl), usage);
	return ret;
}
void RenderSystem11::SetConstBuffers(size_t slot, IContantBufferPtr buffers[], size_t count, IProgramPtr program)
{
	std::vector<ID3D11Buffer*> passConstBuffers(count);
	for (size_t i = 0; i < count; ++i)
		passConstBuffers[i] = buffers[i] ? std::static_pointer_cast<ContantBuffer11>(buffers[i])->GetBuffer11() : nullptr;
	mDeviceContext->VSSetConstantBuffers(slot, count, &passConstBuffers[0]);
	mDeviceContext->PSSetConstantBuffers(slot, count, &passConstBuffers[0]);
}

bool RenderSystem11::UpdateBuffer(IHardwareBufferPtr buffer, const Data& data)
{
	BOOST_ASSERT(buffer);

	D3D11_MAPPED_SUBRESOURCE mapData;
	switch (buffer->GetType()) {
	case kHWBufferConstant: {
		ContantBuffer11Ptr cbuffer11 = std::static_pointer_cast<ContantBuffer11>(buffer);
		if (cbuffer11->GetUsage() == kHWUsageDefault) {
			mDeviceContext->UpdateSubresource(cbuffer11->GetBuffer11(), 0, NULL, data.Bytes, 0, 0);
		}
		else {
			if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData))) return false;
			memcpy(mapData.pData, data.Bytes, data.Size);
			mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
		}
	}break;
	case kHWBufferVertex: {
		VertexBuffer11Ptr cbuffer11 = std::static_pointer_cast<VertexBuffer11>(buffer);
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData))) return false;
		memcpy(mapData.pData, data.Bytes, std::min<int>(buffer->GetBufferSize(), data.Size));
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	case kHWBufferIndex: {
		IndexBuffer11Ptr cbuffer11 = std::static_pointer_cast<IndexBuffer11>(buffer);
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData))) return false;
		memcpy(mapData.pData, data.Bytes, std::min<int>(buffer->GetBufferSize(), data.Size));
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);										
	}break;
	default:
		break;
	}
	return true;
}

ITexturePtr RenderSystem11::LoadTexture(IResourcePtr res, ResourceFormat format, 
	const Eigen::Vector4i& size/*w_h_step_face*/, int mipCount, const Data datas[])
{
	BOOST_ASSERT(res);

	Data defaultData = Data{};
	datas = datas ? datas : &defaultData;
	const HWMemoryUsage usage = datas[0].Bytes ? kHWUsageDefault : kHWUsageDynamic;

	Texture11Ptr texture = std::static_pointer_cast<Texture11>(res);
	texture->Init(format, usage, size.x(), size.y(), size.w(), mipCount);

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
	if (usage == kHWUsageDefault) {
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
	if (CheckHR(mDevice->CreateShaderResourceView(pTexture, &srvDesc, &texture->AsSRV()))) return nullptr; 
	
	if (autoGen) {
		mDeviceContext->UpdateSubresource(pTexture, 0, nullptr, datas[0].Bytes, datas[0].Size, imageSize);
		mDeviceContext->GenerateMips(texture->AsSRV());
	}

	DEBUG_RES_ADD_DEVICE(texture, texture->AsSRV());
	return texture;
}
static inline std::vector<ID3D11ShaderResourceView*> GetTextureViews11(ITexturePtr textures[], size_t count) {
	std::vector<ID3D11ShaderResourceView*> views(count);
	for (int i = 0; i < views.size(); ++i) {
		auto iTex = std::static_pointer_cast<Texture11>(textures[i]);
		if (iTex != nullptr) views[i] = iTex->AsSRV();
	}
	return views;
}
void RenderSystem11::SetTextures(size_t slot, ITexturePtr textures[], size_t count) 
{
	std::vector<ID3D11ShaderResourceView*> texViews = GetTextureViews11(textures, count);
	mDeviceContext->PSSetShaderResources(slot, texViews.size(), !texViews.empty() ? &texViews[0] : nullptr);
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
	if (CheckHR(mDevice->CreateTexture2D(&desc, &initData, tex.GetAddressOf()))) return false;
	
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = static_cast<DXGI_FORMAT>(texture->GetFormat());
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = texture->GetMipmapCount();

	Texture11Ptr tex11 = std::static_pointer_cast<Texture11>(texture);
	if (CheckHR(mDevice->CreateShaderResourceView(tex.Get(), &SRVDesc, &tex11->AsSRV()))) return false;

	return true;
}

ISamplerStatePtr RenderSystem11::LoadSampler(IResourcePtr res, const SamplerDesc& desc)
{
	BOOST_ASSERT(res);
	
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = static_cast<D3D11_FILTER>(desc.Filter);
	sampDesc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(desc.AddressU);
	sampDesc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(desc.AddressV);
	sampDesc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(desc.AddressW);
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = (desc.Filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
	sampDesc.ComparisonFunc = static_cast<D3D11_COMPARISON_FUNC>(desc.CmpFunc);
	sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 1.0;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = 64;

	ID3D11SamplerState* pSampler = nullptr;
	if (CheckHR(mDevice->CreateSamplerState(&sampDesc, &pSampler)))
		return nullptr;

	SamplerState11Ptr ret = std::static_pointer_cast<SamplerState11>(res);
	ret->Init(pSampler);
#if defined MIR_RESOURCE_DEBUG
	ret->mDesc = desc;
#endif
	return ret;
}
void RenderSystem11::SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count)
{
	BOOST_ASSERT(count >= 0);
	std::vector<ID3D11SamplerState*> passSamplers(count);
	for (size_t i = 0; i < count; ++i)
		passSamplers[i] = samplers[i] ? std::static_pointer_cast<SamplerState11>(samplers[i])->GetSampler11() : nullptr;
	mDeviceContext->PSSetSamplers(0, passSamplers.size(), !passSamplers.empty() ? &passSamplers[0] : nullptr);
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

void RenderSystem11::DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	mDeviceContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topo));
	mDeviceContext->Draw(op.VertexBuffers[0]->GetCount(), 0);
}
void RenderSystem11::DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	mDeviceContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topo));
	int indexCount = op.IndexCount != 0 ? op.IndexCount : op.IndexBuffer->GetBufferSize() / op.IndexBuffer->GetWidth();
	mDeviceContext->DrawIndexed(indexCount, op.IndexPos, op.IndexBase);
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