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

	hr = mDevice->CreateRenderTargetView(pBackBuffer, NULL, &mBackRenderTargetView); 
	mCurRenderTargetView = mBackRenderTargetView;
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
	if (CheckHR(hr = mDevice->CreateTexture2D(&descDepth, NULL, &mDepthStencil))) return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = mDevice->CreateDepthStencilView(mDepthStencil, &descDSV, &mBackDepthStencilView); mCurDepthStencilView = mBackDepthStencilView;
	return hr;
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

	SetViewPort(vp.left, vp.top, vpWidth, vpHeight);

	if (CheckHR(_SetRasterizerState())) return false;

	SetDepthState(DepthState{ true, kCompareLessEqual, kDepthWriteMaskAll });
	SetBlendFunc(BlendState::MakeAlphaPremultiplied());

	mScreenSize.x() = vpWidth;
	mScreenSize.y() = vpHeight;
	return true;
}

void RenderSystem11::Update(float dt)
{}
void RenderSystem11::CleanUp()
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
	case mir::kDeviceResourceRenderTarget:
		return MakePtr<RenderTarget11>();
	case mir::kDeviceResourceSamplerState:
		return MakePtr<SamplerState11>();
	default:
		break;
	}
	return nullptr;
}

IRenderTargetPtr RenderSystem11::LoadRenderTarget(IResourcePtr res, const Eigen::Vector2i& size, ResourceFormat format)
{
	BOOST_ASSERT(res);

	RenderTexture11Ptr ret = std::static_pointer_cast<RenderTarget11>(res);
	ret->Init(mDevice, size, format);
	return ret;
}
void RenderSystem11::ClearRenderTarget(IRenderTargetPtr rendTarget, const Eigen::Vector4f& color, float depth, uint8_t stencil)
{
	if (rendTarget) {
		auto target11 = std::static_pointer_cast<RenderTarget11>(rendTarget);
		mDeviceContext->ClearRenderTargetView(target11->GetColorBuffer11(), (const float*)&color);
		mDeviceContext->ClearDepthStencilView(target11->GetDepthStencilBuffer11(), D3D11_CLEAR_DEPTH, depth, stencil);
	}
	else {
		mDeviceContext->ClearRenderTargetView(mCurRenderTargetView, (const float*)&color);
		mDeviceContext->ClearDepthStencilView(mCurDepthStencilView, D3D11_CLEAR_DEPTH, depth, stencil);
	}
}
void RenderSystem11::SetRenderTarget(IRenderTargetPtr rendTarget)
{
	ID3D11ShaderResourceView* TextureNull = nullptr;
	mDeviceContext->PSSetShaderResources(0, 1, &TextureNull);

	auto target11 = std::static_pointer_cast<RenderTarget11>(rendTarget);
	mCurRenderTargetView = target11 != nullptr ? target11->GetColorBuffer11() : mBackRenderTargetView;
	mCurDepthStencilView = target11 != nullptr ? target11->GetDepthStencilBuffer11() : mBackDepthStencilView;
	mDeviceContext->OMSetRenderTargets(1, &mCurRenderTargetView, mCurDepthStencilView);
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
#if defined(_DEBUG) && defined(MIR_D3D11_DEBUG)
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
		case kShaderPixel:
			program->SetPixel(std::static_pointer_cast<PixelShader11>(iter));
		default:
			break;
		}
	}
	return program;
}
void RenderSystem11::SetProgram(IProgramPtr program)
{
	mDeviceContext->VSSetShader(std::static_pointer_cast<VertexShader11>(program->GetVertex())->GetShader11(), NULL, 0);
	mDeviceContext->PSSetShader(std::static_pointer_cast<PixelShader11>(program->GetPixel())->GetShader11(), NULL, 0);
}

IVertexBufferPtr RenderSystem11::LoadVertexBuffer(IResourcePtr res, int stride, int offset, const Data& data)
{
	BOOST_ASSERT(res);

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = data.Size;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	if (data.Bytes) {
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
	ret->Init(pVertexBuffer, data.Size, stride, offset);
	return ret;
}
void RenderSystem11::SetVertexBuffers(size_t slot, IVertexBufferPtr vertexBuffers[], size_t count)
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

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = data.Size;// sizeof(WORD) * Indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = data.Bytes;
	
	ID3D11Buffer* pIndexBuffer = nullptr;
	if (CheckHR(mDevice->CreateBuffer(&bd, &InitData, &pIndexBuffer))) return nullptr;

	IndexBuffer11Ptr ret = std::static_pointer_cast<IndexBuffer11>(res);
	ret->Init(pIndexBuffer, data.Size, format);
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

#define CBufferUsage D3D11_USAGE_DEFAULT

IContantBufferPtr RenderSystem11::LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, const Data& data)
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
	if (data.NotNull()) {
		BOOST_ASSERT(data.Size == ret->GetBufferSize());
		UpdateBuffer(ret, data);
	}
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

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	switch (buffer->GetType()) {
	case kHWBufferConstant: {
		ContantBuffer11Ptr cbuffer11 = std::static_pointer_cast<ContantBuffer11>(buffer);
	#if CBufferUsage == D3D11_USAGE_DEFAULT
		mDeviceContext->UpdateSubresource(cbuffer11->GetBuffer11(), 0,
			NULL, data.Bytes, 0, 0);
	#else
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0,
			D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return false;
		memcpy(MappedResource.pData, data.Bytes, data.Size);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	#endif
	}break;
	case kHWBufferVertex: {
		VertexBuffer11Ptr cbuffer11 = std::static_pointer_cast<VertexBuffer11>(buffer);
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0,
			D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return false;
		memcpy(MappedResource.pData, data.Bytes, data.Size);
		mDeviceContext->Unmap(cbuffer11->GetBuffer11(), 0);
	}break;
	case kHWBufferIndex: {
		IndexBuffer11Ptr cbuffer11 = std::static_pointer_cast<IndexBuffer11>(buffer);
		if (CheckHR(mDeviceContext->Map(cbuffer11->GetBuffer11(), 0,
			D3D11_MAP_WRITE_DISCARD, 0, &MappedResource))) return false;
		memcpy(MappedResource.pData, data.Bytes, data.Size);
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
static inline std::vector<ID3D11ShaderResourceView*> GetTextureViews11(ITexturePtr textures[], size_t count) {
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
void RenderSystem11::SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count)
{
	BOOST_ASSERT(count > 0);
	std::vector<ID3D11SamplerState*> passSamplers(count);
	for (size_t i = 0; i < count; ++i)
		passSamplers[i] = samplers[i] ? std::static_pointer_cast<SamplerState11>(samplers[i])->GetSampler11() : nullptr;
	mDeviceContext->PSSetSamplers(0, passSamplers.size(), &passSamplers[0]);
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
	mDeviceContext->Draw(op.mVertexBuffer->GetBufferSize() / op.mVertexBuffer->GetStride(), 0);
}
void RenderSystem11::DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	mDeviceContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topo));
	int indexCount = op.mIndexCount != 0 ? op.mIndexCount : op.mIndexBuffer->GetBufferSize() / op.mIndexBuffer->GetWidth();
	mDeviceContext->DrawIndexed(indexCount, op.mIndexPos, op.mIndexBase);
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