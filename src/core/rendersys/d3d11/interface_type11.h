#pragma once
#include "core/rendersys/interface_type.h"

namespace mir {

typedef std::shared_ptr< struct BlobData11> TBlobDataD3d11Ptr;
typedef std::shared_ptr< struct InputLayout11> TInputLayout11Ptr;
typedef std::shared_ptr< struct VertexShader11> TVertexShader11Ptr;
typedef std::shared_ptr< struct PixelShader11> TPixelShader11Ptr;
typedef std::shared_ptr< struct Program11> TProgram11Ptr;
typedef std::shared_ptr< struct VertexBuffer11> TVertexBuffer11Ptr;
typedef std::shared_ptr< struct IndexBuffer11> TIndexBuffer11Ptr;
typedef std::shared_ptr< struct ContantBuffer11> TContantBuffer11Ptr;
typedef std::shared_ptr< struct Texture11> TTexture11Ptr;
typedef std::shared_ptr< struct RenderTexture11> TRenderTexture11Ptr;
typedef std::shared_ptr< struct SamplerState11> TSamplerState11Ptr;

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
	TResourcePtr mRes;
public:
	InputLayout11();
	ID3D11InputLayout*& GetLayout11();
	IResourcePtr AsRes() override;
};

struct VertexShader11 : public IVertexShader {
	ID3D11VertexShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	TResourcePtr mRes;
public:
	VertexShader11(IBlobDataPtr pBlob);
	IResourcePtr AsRes() override;

	IBlobDataPtr GetBlob() override;
	ID3D11VertexShader*& GetShader11();
};

struct PixelShader11 : public IPixelShader {
	ID3D11PixelShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	TResourcePtr mRes;
public:
	PixelShader11(IBlobDataPtr pBlob);
	IResourcePtr AsRes() override;
	IBlobDataPtr GetBlob() override;
	ID3D11PixelShader*& GetShader11();
};

struct Program11 : public IProgram {
	TVertexShader11Ptr mVertex;
	TPixelShader11Ptr mPixel;
	TResourcePtr mRes;
public:
	Program11();
	IResourcePtr AsRes() override;
	void SetVertex(TVertexShader11Ptr pVertex);
	void SetPixel(TPixelShader11Ptr pPixel);
	IVertexShaderPtr GetVertex() override;
	IPixelShaderPtr GetPixel() override;
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
	unsigned int stride, offset;
	HardwareBuffer hd;
public:
	VertexBuffer11(ID3D11Buffer* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:hd(__buffer, __bufferSize), stride(__stride), offset(__offset) {};
	VertexBuffer11() :stride(0), offset(0) {};
	int GetCount();
public:
	ID3D11Buffer*& GetBuffer11();
	unsigned int GetBufferSize() override;
	HardwareBufferType GetType() override {
		return kHWBufferVertex;
	}
	unsigned int GetStride() override;
	unsigned int GetOffset() override;
};

struct IndexBuffer11 : public IIndexBuffer {
	DXGI_FORMAT format;
	HardwareBuffer hd;
public:
	IndexBuffer11(ID3D11Buffer* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
		:hd(__buffer, __bufferSize), format(__format) {};
	IndexBuffer11() :format(DXGI_FORMAT_UNKNOWN) {};
public:
	ID3D11Buffer*& GetBuffer11();
	unsigned int GetBufferSize() override;
	HardwareBufferType GetType() override {
		return kHWBufferIndex;
	}

	int GetWidth() override;
	DXGI_FORMAT GetFormat() override;
};

struct ContantBuffer11 : public IContantBuffer {
	TConstBufferDeclPtr mDecl;
	HardwareBuffer hd;
public:
	ContantBuffer11() {}
	ContantBuffer11(ID3D11Buffer* __buffer, TConstBufferDeclPtr decl);
public:
	TConstBufferDeclPtr GetDecl() override;
	HardwareBufferType GetType() override {
		return kHWBufferConstant;
	}
	ID3D11Buffer*& GetBuffer11();
	unsigned int GetBufferSize() override;
};

/********** Texture **********/
struct Texture11 : public ITexture {
private:
	int mWidth = 0, mHeight = 0, mMipCount = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
	ID3D11ShaderResourceView *mTexture = nullptr;
	IResourcePtr mRes;
	std::string mPath;
public:
	Texture11(int width, int height, DXGI_FORMAT format, int mipmap);

	Texture11(ID3D11ShaderResourceView* __texture, const std::string& __path);
	IResourcePtr AsRes() override { return mRes; }

	bool HasSRV() override { return mTexture != nullptr; }
	void SetSRV11(ID3D11ShaderResourceView* __texture);
	ID3D11ShaderResourceView*& GetSRV11();

	const char* GetPath() override;
	int GetWidth() override;
	int GetHeight() override;
	DXGI_FORMAT GetFormat() override;
	int GetMipmapCount() override;
private:
	D3D11_TEXTURE2D_DESC GetDesc();
};

struct RenderTexture11 : public IRenderTexture {
private:
	ID3D11Texture2D* mRenderTargetTexture = nullptr;
	ID3D11ShaderResourceView* mRenderTargetSRV = nullptr;
	ITexturePtr mRenderTargetPtr;
	ID3D11RenderTargetView* mRenderTargetView = nullptr;

	ID3D11Texture2D* mDepthStencilTexture = nullptr;
	ID3D11DepthStencilView* mDepthStencilView = nullptr;

	DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
public:
	RenderTexture11(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	ITexturePtr GetColorTexture() override;

	ID3D11RenderTargetView*& GetColorBuffer11();
	ID3D11DepthStencilView*& GetDepthStencilBuffer11();
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
	ID3D11SamplerState*& GetSampler11();
};

}