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
struct BlobData11 : public IBlobData {
	ID3DBlob* mBlob = nullptr;
public:
	BlobData11(ID3DBlob* pBlob);
	char* GetBufferPointer() override;
	size_t GetBufferSize() override;
};

struct InputLayout11 : public IInputLayout {
public:
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDescs;
	ID3D11InputLayout* mLayout = nullptr;
	IResourcePtr mRes;
public:
	InputLayout11();
	ID3D11InputLayout*& GetLayout11() { return mLayout; }
	IResourcePtr AsRes() override { return mRes; }
};

struct VertexShader11 : public IVertexShader {
	ID3D11VertexShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	IResourcePtr mRes;
public:
	VertexShader11(IBlobDataPtr pBlob);
	IResourcePtr AsRes() override { return mRes; }

	IBlobDataPtr GetBlob() override { return mBlob; }
	ID3D11VertexShader*& GetShader11() { return mShader; }
};

struct PixelShader11 : public IPixelShader {
	ID3D11PixelShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	IResourcePtr mRes;
public:
	PixelShader11(IBlobDataPtr pBlob);
	IResourcePtr AsRes() override { return mRes; }
	IBlobDataPtr GetBlob() override { return mBlob; }
	ID3D11PixelShader*& GetShader11() { return mShader; }
};

struct Program11 : public IProgram {
	VertexShader11Ptr mVertex;
	PixelShader11Ptr mPixel;
	IResourcePtr mRes;
public:
	Program11();
	IResourcePtr AsRes() override { return mRes; }
	void SetVertex(VertexShader11Ptr pVertex);
	void SetPixel(PixelShader11Ptr pPixel);
	IVertexShaderPtr GetVertex() override { return mVertex; }
	IPixelShaderPtr GetPixel() override { return mPixel; }
};

/********** HardwareBuffer **********/
struct HardwareBuffer {
	ID3D11Buffer* buffer;
	unsigned int bufferSize;
public:
	HardwareBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :buffer(__buffer), bufferSize(__bufferSize) {};
	HardwareBuffer() :buffer(nullptr), bufferSize(0) {};
};

struct VertexBuffer11 : public IVertexBuffer {
	unsigned int Stride, Offset;
	HardwareBuffer hd;
public:
	VertexBuffer11(ID3D11Buffer* buffer, unsigned int bufferSize, unsigned int stride, unsigned int offset)
		:hd(buffer, bufferSize), Stride(stride), Offset(offset) {};
	VertexBuffer11() :Stride(0), Offset(0) {};
	int GetCount();
public:
	ID3D11Buffer*& GetBuffer11() { return hd.buffer; }
	unsigned int GetBufferSize() override { return hd.bufferSize; }
	HardwareBufferType GetType() override { return kHWBufferVertex; }
	unsigned int GetStride() override { return Stride; }
	unsigned int GetOffset() override { return Offset; }
};

struct IndexBuffer11 : public IIndexBuffer {
	ResourceFormat Format;
	HardwareBuffer hd;
public:
	IndexBuffer11(ID3D11Buffer* buffer, unsigned int bufferSize, ResourceFormat format)
		:hd(buffer, bufferSize), Format(format) {};
	IndexBuffer11() :Format(kFormatUnknown) {};
public:
	ID3D11Buffer*& GetBuffer11() { return hd.buffer; }
	unsigned int GetBufferSize() override { return hd.bufferSize; }
	HardwareBufferType GetType() override { return kHWBufferIndex; }

	int GetWidth() override;
	ResourceFormat GetFormat() override { return Format; }
};

struct ContantBuffer11 : public IContantBuffer {
	ConstBufferDeclPtr mDecl;
	HardwareBuffer hd;
public:
	ContantBuffer11() {}
	ContantBuffer11(ID3D11Buffer* buffer, ConstBufferDeclPtr decl);
public:
	ConstBufferDeclPtr GetDecl() override { return mDecl; }
	HardwareBufferType GetType() override { return kHWBufferConstant; }

	ID3D11Buffer*& GetBuffer11() { return hd.buffer; }
	unsigned int GetBufferSize() override { return hd.bufferSize; }
};

/********** Texture **********/
struct Texture11 : public ITexture {
private:
	int mWidth, mHeight, mMipCount;
	ResourceFormat mFormat;
	ID3D11ShaderResourceView* mTexture;
	IResourcePtr mRes;
	std::string mPath;
public:
	Texture11(int width, int height, ResourceFormat format, int mipmap);
	Texture11(ID3D11ShaderResourceView* texture, const std::string& path);
	IResourcePtr AsRes() override { return mRes; }

	bool HasSRV() override { return mTexture != nullptr; }
	void SetSRV11(ID3D11ShaderResourceView* texture);
	ID3D11ShaderResourceView*& GetSRV11() { return mTexture; }

	const char* GetPath() override { return mPath.c_str(); }
	int GetWidth() override { return mWidth; }
	int GetHeight() override { return mHeight; }
	ResourceFormat GetFormat() override { return mFormat; }
	int GetMipmapCount() override { return mMipCount; }
private:
	D3D11_TEXTURE2D_DESC GetDesc();
};

struct RenderTexture11 : public IRenderTexture {
private:
	ITexturePtr mRenderTargetPtr;
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11ShaderResourceView* mRenderTargetSRV;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;

	ResourceFormat mFormat;
public:
	RenderTexture11(ID3D11Device* pDevice, int width, int height, ResourceFormat format = kFormatR32G32B32A32Float);
	ITexturePtr GetColorTexture() override { return mRenderTargetPtr; }

	ID3D11RenderTargetView*& GetColorBuffer11() { 	return mRenderTargetView; }
	ID3D11DepthStencilView*& GetDepthStencilBuffer11() { return mDepthStencilView; }
private:
	bool InitRenderTexture(ID3D11Device* pDevice, int width, int height);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);

	bool InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height);
	bool InitDepthStencilView(ID3D11Device* pDevice);
};

struct SamplerState11 : public ISamplerState {
	ID3D11SamplerState* mSampler = nullptr;
public:
	SamplerState11(ID3D11SamplerState* sampler = nullptr) :mSampler(sampler) {};
	ID3D11SamplerState*& GetSampler11() { return mSampler; }
};

}