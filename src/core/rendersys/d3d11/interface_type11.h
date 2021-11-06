#pragma once
#include "core/rendersys/interface_type.h"

namespace mir {

typedef std::shared_ptr< struct TBlobDataD3d11> TBlobDataD3d11Ptr;
typedef std::shared_ptr< struct TInputLayout11> TInputLayout11Ptr;
typedef std::shared_ptr< struct TVertexShader11> TVertexShader11Ptr;
typedef std::shared_ptr< struct TPixelShader11> TPixelShader11Ptr;
typedef std::shared_ptr< struct TProgram11> TProgram11Ptr;
typedef std::shared_ptr< struct TVertexBuffer11> TVertexBuffer11Ptr;
typedef std::shared_ptr< struct TIndexBuffer11> TIndexBuffer11Ptr;
typedef std::shared_ptr< struct TContantBuffer11> TContantBuffer11Ptr;
typedef std::shared_ptr< struct TTexture11> TTexture11Ptr;
typedef std::shared_ptr< struct TRenderTexture11> TRenderTexture11Ptr;
typedef std::shared_ptr< struct TSamplerState11> TSamplerState11Ptr;

/********** Program **********/
struct TBlobDataD3d11 : public IBlobData {
	ID3DBlob* mBlob = nullptr;
public:
	TBlobDataD3d11(ID3DBlob* pBlob);
	char* GetBufferPointer() override;
	size_t GetBufferSize() override;
};

struct TInputLayout11 : public IInputLayout {
public:
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDescs;
	ID3D11InputLayout* mLayout = nullptr;
	TResourcePtr mRes;
public:
	TInputLayout11();
	ID3D11InputLayout*& GetLayout11();
	IResourcePtr AsRes() override;
};

struct TVertexShader11 : public IVertexShader {
	ID3D11VertexShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	TResourcePtr mRes;
public:
	TVertexShader11(IBlobDataPtr pBlob);
	IResourcePtr AsRes() override;

	IBlobDataPtr GetBlob() override;
	ID3D11VertexShader*& GetShader11();
};

struct TPixelShader11 : public IPixelShader {
	ID3D11PixelShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	TResourcePtr mRes;
public:
	TPixelShader11(IBlobDataPtr pBlob);
	IResourcePtr AsRes() override;
	IBlobDataPtr GetBlob() override;
	ID3D11PixelShader*& GetShader11();
};

struct TProgram11 : public IProgram {
	TVertexShader11Ptr mVertex;
	TPixelShader11Ptr mPixel;
	TResourcePtr mRes;
public:
	TProgram11();
	IResourcePtr AsRes() override;
	void SetVertex(TVertexShader11Ptr pVertex);
	void SetPixel(TPixelShader11Ptr pPixel);
	IVertexShaderPtr GetVertex() override;
	IPixelShaderPtr GetPixel() override;
};

/********** HardwareBuffer **********/
struct THardwareBuffer {
	ID3D11Buffer* buffer;
	unsigned int bufferSize;
public:
	THardwareBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :buffer(__buffer), bufferSize(__bufferSize) {};
	THardwareBuffer() :buffer(nullptr), bufferSize(0) {};
};

struct TVertexBuffer11 : public IVertexBuffer {
	unsigned int stride, offset;
	THardwareBuffer hd;
public:
	TVertexBuffer11(ID3D11Buffer* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:hd(__buffer, __bufferSize), stride(__stride), offset(__offset) {};
	TVertexBuffer11() :stride(0), offset(0) {};
	int GetCount();
public:
	ID3D11Buffer*& GetBuffer11();
	unsigned int GetBufferSize() override;
	enHardwareBufferType GetType() override {
		return E_HWBUFFER_VERTEX;
	}
	unsigned int GetStride() override;
	unsigned int GetOffset() override;
};

struct TIndexBuffer11 : public IIndexBuffer {
	DXGI_FORMAT format;
	THardwareBuffer hd;
public:
	TIndexBuffer11(ID3D11Buffer* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
		:hd(__buffer, __bufferSize), format(__format) {};
	TIndexBuffer11() :format(DXGI_FORMAT_UNKNOWN) {};
public:
	ID3D11Buffer*& GetBuffer11();
	unsigned int GetBufferSize() override;
	enHardwareBufferType GetType() override {
		return E_HWBUFFER_INDEX;
	}

	int GetWidth() override;
	DXGI_FORMAT GetFormat() override;
};

struct TContantBuffer11 : public IContantBuffer {
	TConstBufferDeclPtr mDecl;
	THardwareBuffer hd;
public:
	TContantBuffer11() {}
	TContantBuffer11(ID3D11Buffer* __buffer, TConstBufferDeclPtr decl);
public:
	TConstBufferDeclPtr GetDecl() override;
	enHardwareBufferType GetType() override {
		return E_HWBUFFER_CONSTANT;
	}
	ID3D11Buffer*& GetBuffer11();
	unsigned int GetBufferSize() override;
};

/********** Texture **********/
struct TTexture11 : public ITexture {
private:
	int mWidth = 0, mHeight = 0, mMipCount = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
	ID3D11ShaderResourceView *mTexture = nullptr;
	IResourcePtr mRes;
	std::string mPath;
public:
	TTexture11(int width, int height, DXGI_FORMAT format, int mipmap);

	TTexture11(ID3D11ShaderResourceView* __texture, const std::string& __path);
	IResourcePtr AsRes() override {
		return mRes;
	}

	bool HasSRV() override {
		return mTexture != nullptr;
	}
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

struct TRenderTexture11 : public IRenderTexture {
private:
	ID3D11Texture2D* mRenderTargetTexture = nullptr;
	ID3D11ShaderResourceView* mRenderTargetSRV = nullptr;
	ITexturePtr mRenderTargetPtr;
	ID3D11RenderTargetView* mRenderTargetView = nullptr;

	ID3D11Texture2D* mDepthStencilTexture = nullptr;
	ID3D11DepthStencilView* mDepthStencilView = nullptr;

	DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
public:
	TRenderTexture11(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
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

struct TSamplerState11 : public ISamplerState {
	ID3D11SamplerState* mSampler = nullptr;
public:
	TSamplerState11(ID3D11SamplerState* sampler = nullptr) :mSampler(sampler) {};
	ID3D11SamplerState*& GetSampler11();
};

}