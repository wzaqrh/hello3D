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
typedef std::shared_ptr< struct FrameBuffer11> FrameBuffer11Ptr;
typedef std::shared_ptr< struct SamplerState11> SamplerState11Ptr;

/********** Program **********/
class BlobData11 : public IBlobData {
public:
	BlobData11(ID3DBlob* pBlob);
	const char* GetBytes() const override;
	size_t GetSize() const override;
public:
	ID3DBlob* mBlob = nullptr;
};

class InputLayout11 : public ImplementResource<IInputLayout> 
{
public:
	InputLayout11();
	ID3D11InputLayout*& GetLayout11() { return mLayout; }
public:
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDescs;
	ID3D11InputLayout* mLayout = nullptr;
};

class VertexShader11 : public ImplementResource<IVertexShader> 
{
public:
	VertexShader11(IBlobDataPtr pBlob);
	IBlobDataPtr GetBlob() const override { return mBlob; }
	ID3D11VertexShader*& GetShader11() { return mShader; }
public:
	ID3D11VertexShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
};

class PixelShader11 : public ImplementResource<IPixelShader> 
{
public:
	PixelShader11(IBlobDataPtr pBlob);
	IBlobDataPtr GetBlob() const override { return mBlob; }
	ID3D11PixelShader*& GetShader11() { return mShader; }
public:
	ID3D11PixelShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
};

class Program11 : public ImplementResource<IProgram> 
{
public:
	Program11();
	void SetVertex(VertexShader11Ptr pVertex);
	void SetPixel(PixelShader11Ptr pPixel);
	IVertexShaderPtr GetVertex() const override { return mVertex; }
	IPixelShaderPtr GetPixel() const override { return mPixel; }
public:
	VertexShader11Ptr mVertex;
	PixelShader11Ptr mPixel;
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
	void Init(ID3D11Buffer* buffer, unsigned int bufferSize, unsigned int stride, unsigned int offset);
	VertexBuffer11() :Stride(0), Offset(0) {}
	int GetCount();
public:
	ID3D11Buffer*& GetBuffer11() { return hd.Buffer; }
	unsigned int GetBufferSize() const override { return hd.BufferSize; }
	HardwareBufferType GetType() const override { return kHWBufferVertex; }
	unsigned int GetStride() const override { return Stride; }
	unsigned int GetOffset() const override { return Offset; }
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
	unsigned int GetBufferSize() const override { return hd.BufferSize; }
	HardwareBufferType GetType() const override { return kHWBufferIndex; }

	int GetWidth() const override;
	ResourceFormat GetFormat() const override { return Format; }
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
	ConstBufferDeclPtr GetDecl() const override { return mDecl; }
	HardwareBufferType GetType() const override { return kHWBufferConstant; }

	ID3D11Buffer*& GetBuffer11() { return hd.Buffer; }
	unsigned int GetBufferSize() const override { return hd.BufferSize; }
public:
	ConstBufferDeclPtr mDecl;
	HardwareBuffer hd;
};

/********** Texture **********/
class Texture11 : public ImplementResource<ITexture> 
{
public:
	void Init(ResourceFormat format, int width, int height, int faceCount, int mipmap);

	bool HasSRV() const override { return mTexture != nullptr; }
	void SetSRV11(ID3D11ShaderResourceView* texture);
	ID3D11ShaderResourceView*& GetSRV11() { return mTexture; }

	ResourceFormat GetFormat() const override { return mFormat; }
	int GetWidth() const override { return mWidth; }
	int GetHeight() const override { return mHeight; }
	int GetMipmapCount() const override { return mMipCount; }
	int GetFaceCount() const override { return mFaceCount; }
	bool IsAutoGenMipmap() const override { return mAutoGenMipmap; }
private:
	D3D11_TEXTURE2D_DESC GetDesc();
private:
	bool mAutoGenMipmap;
	int mWidth, mHeight, mFaceCount, mMipCount;
	ResourceFormat mFormat;
	ID3D11ShaderResourceView* mTexture;
};

class FrameBuffer11 : public ImplementResource<IFrameBuffer>
{
public:
	FrameBuffer11();
	void Init(ID3D11Device* pDevice, const Eigen::Vector2i& size, ResourceFormat format = kFormatR32G32B32A32Float);
	ITexturePtr GetColorTexture() const override { return mRenderTargetPtr; }

	ID3D11RenderTargetView*& GetColorBuffer11() { 	return mRenderTargetView; }
	ID3D11DepthStencilView*& GetDepthStencilBuffer11() { return mDepthStencilView; }
private:
	bool InitRenderTexture(ID3D11Device* pDevice);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);

	bool InitDepthStencilTexture(ID3D11Device* pDevice);
	bool InitDepthStencilView(ID3D11Device* pDevice);
private:
	Texture11Ptr mRenderTargetPtr;
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11ShaderResourceView* mRenderTargetSRV;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;

	Eigen::Vector2i mSize;
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