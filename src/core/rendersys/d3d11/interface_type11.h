#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include "core/rendersys/interface_type.h"

namespace mir {

typedef std::shared_ptr< struct BlobData11> BlobData11Ptr;
typedef std::shared_ptr< struct InputLayout11> InputLayout11Ptr;
typedef std::shared_ptr< struct VertexShader11> VertexShader11Ptr;
typedef std::shared_ptr< struct PixelShader11> PixelShader11Ptr;
typedef std::shared_ptr< struct Program11> Program11Ptr;
typedef std::shared_ptr< struct VertexBuffer11> VertexBuffer11Ptr;
typedef std::shared_ptr< struct IndexBuffer11> IndexBuffer11Ptr;
typedef std::shared_ptr< struct ContantBuffer11> ContantBuffer11Ptr;
typedef std::shared_ptr< struct Texture11> Texture11Ptr;
typedef std::shared_ptr< struct RenderTexture11> RenderTexture11Ptr;
typedef std::shared_ptr< struct SamplerState11> SamplerState11Ptr;

/********** Program **********/
class BlobData11 : public IBlobData {
public:
	BlobData11(ID3DBlob* pBlob);
	char* GetBufferPointer() override;
	size_t GetBufferSize() override;
public:
	ID3DBlob* mBlob = nullptr;
};

class InputLayout11 : public ImplementResource<IInputLayout> 
{
public:
	InputLayout11();
	//IResourcePtr AsRes() override { return mRes; }

	ID3D11InputLayout*& GetLayout11() { return mLayout; }
public:
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDescs;
	ID3D11InputLayout* mLayout = nullptr;
	//IResourcePtr mRes;
};

class VertexShader11 : public ImplementResource<IVertexShader> 
{
public:
	ID3D11VertexShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	//IResourcePtr mRes;
public:
	VertexShader11(IBlobDataPtr pBlob);
	//IResourcePtr AsRes() override { return mRes; }

	IBlobDataPtr GetBlob() override { return mBlob; }
	ID3D11VertexShader*& GetShader11() { return mShader; }
};

class PixelShader11 : public ImplementResource<IPixelShader> 
{
public:
	PixelShader11(IBlobDataPtr pBlob);
	//IResourcePtr AsRes() override { return mRes; }
	
	IBlobDataPtr GetBlob() override { return mBlob; }
	ID3D11PixelShader*& GetShader11() { return mShader; }
public:
	ID3D11PixelShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	//IResourcePtr mRes;
};

class Program11 : public ImplementResource<IProgram> 
{
public:
	Program11();
	//IResourcePtr AsRes() override { return mRes; }
	
	void SetVertex(VertexShader11Ptr pVertex);
	void SetPixel(PixelShader11Ptr pPixel);
	IVertexShaderPtr GetVertex() override { return mVertex; }
	IPixelShaderPtr GetPixel() override { return mPixel; }
public:
	VertexShader11Ptr mVertex;
	PixelShader11Ptr mPixel;
	//IResourcePtr mRes;
};

/********** HardwareBuffer **********/
class HardwareBuffer 
{
public:
	HardwareBuffer(ID3D11Buffer* buffer, unsigned int bufferSize) :Buffer(buffer), BufferSize(bufferSize) {}
	HardwareBuffer() :HardwareBuffer(nullptr, 0) {}
	void Init(ID3D11Buffer* buffer, unsigned int bufferSize) { Buffer = buffer; BufferSize = bufferSize; }
public:
	ID3D11Buffer* Buffer;
	unsigned int BufferSize;
};

class VertexBuffer11 : public ImplementResource<IVertexBuffer> 
{
public:
	VertexBuffer11(ID3D11Buffer* buffer, unsigned int bufferSize, unsigned int stride, unsigned int offset)
		:hd(buffer, bufferSize), Stride(stride), Offset(offset) {}
	VertexBuffer11() :Stride(0), Offset(0) {}
	int GetCount();
public:
	ID3D11Buffer*& GetBuffer11() { return hd.Buffer; }
	unsigned int GetBufferSize() override { return hd.BufferSize; }
	HardwareBufferType GetType() override { return kHWBufferVertex; }
	unsigned int GetStride() override { return Stride; }
	unsigned int GetOffset() override { return Offset; }
public:
	unsigned int Stride, Offset;
	HardwareBuffer hd;
};

class IndexBuffer11 : public ImplementResource<IIndexBuffer> 
{
public:
	IndexBuffer11() :Format(kFormatUnknown) {}
	void Init(ID3D11Buffer* buffer, unsigned int bufferSize, ResourceFormat format) {
		hd.Init(buffer, bufferSize); 
		Format = format; 
	}
public:
	ID3D11Buffer*& GetBuffer11() { return hd.Buffer; }
	unsigned int GetBufferSize() override { return hd.BufferSize; }
	HardwareBufferType GetType() override { return kHWBufferIndex; }

	int GetWidth() override;
	ResourceFormat GetFormat() override { return Format; }
public:
	ResourceFormat Format;
	HardwareBuffer hd;
};

class ContantBuffer11 : public ImplementResource<IContantBuffer> 
{
public:
	ContantBuffer11() {}
	void Init(ID3D11Buffer* buffer, ConstBufferDeclPtr decl);
public:
	ConstBufferDeclPtr GetDecl() override { return mDecl; }
	HardwareBufferType GetType() override { return kHWBufferConstant; }

	ID3D11Buffer*& GetBuffer11() { return hd.Buffer; }
	unsigned int GetBufferSize() override { return hd.BufferSize; }
public:
	ConstBufferDeclPtr mDecl;
	HardwareBuffer hd;
};

/********** Texture **********/
class Texture11 : public ImplementResource<ITexture> 
{
public:
	Texture11(ID3D11ShaderResourceView* texture);
	void Init(ResourceFormat format, int width, int height, int faceCount, int mipmap);
	//IResourcePtr AsRes() override { return mRes; }

	bool HasSRV() override { return mTexture != nullptr; }
	void SetSRV11(ID3D11ShaderResourceView* texture);
	ID3D11ShaderResourceView*& GetSRV11() { return mTexture; }

	ResourceFormat GetFormat() override { return mFormat; }
	int GetWidth() override { return mWidth; }
	int GetHeight() override { return mHeight; }
	int GetFaceCount() { return mFaceCount; }
	int GetMipmapCount() override { return mMipCount; }
private:
	D3D11_TEXTURE2D_DESC GetDesc();
private:
	int mWidth, mHeight, mFaceCount, mMipCount;
	ResourceFormat mFormat;
	ID3D11ShaderResourceView* mTexture;
	//IResourcePtr mRes;
};

class RenderTexture11 : public ImplementResource<IRenderTexture>
{
public:
	RenderTexture11();
	void Init(ID3D11Device* pDevice, int width, int height, ResourceFormat format = kFormatR32G32B32A32Float);
	ITexturePtr GetColorTexture() override { return mRenderTargetPtr; }

	ID3D11RenderTargetView*& GetColorBuffer11() { 	return mRenderTargetView; }
	ID3D11DepthStencilView*& GetDepthStencilBuffer11() { return mDepthStencilView; }
private:
	bool InitRenderTexture(ID3D11Device* pDevice, int width, int height);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);

	bool InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height);
	bool InitDepthStencilView(ID3D11Device* pDevice);
private:
	ITexturePtr mRenderTargetPtr;
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11ShaderResourceView* mRenderTargetSRV;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;

	ResourceFormat mFormat;
};

class SamplerState11 : public ImplementResource<ISamplerState> {
public:
	void Init(ID3D11SamplerState* sampler) { mSampler = sampler; }
	ID3D11SamplerState*& GetSampler11() { return mSampler; }
public:
	ID3D11SamplerState* mSampler = nullptr;
};

}