#pragma once
#include "TInterfaceType.h"

/********** Program **********/
struct INHERIT_COM("6BA5DAD8-7EDC-49B6-9333-542204A24112")
TBlobDataD3d11 : public ComBase<IBlobData> {
	ID3DBlob* mBlob = nullptr;
public:
	TBlobDataD3d11(ID3DBlob* pBlob);
	STDMETHODIMP_(char*) GetBufferPointer() override;
	STDMETHODIMP_(size_t) GetBufferSize() override;
};
typedef ComPtr<TBlobDataD3d11> TBlobDataD3d11Ptr;

struct INHERIT_COM("13E4BBA6-FAAC-46FC-9C5F-8B742D0D1FC4")
TInputLayout11 : public ComBase<IInputLayout>{
public:
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDescs;
	ID3D11InputLayout* mLayout = nullptr;
	TResourcePtr mRes;
public:
	TInputLayout11();
	ID3D11InputLayout*& GetLayout11();
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}
};
typedef ComPtr<TInputLayout11> TInputLayout11Ptr;

struct INHERIT_COM("96CADE52-543E-4C68-A1C3-3BC9343FF753")
TVertexShader11 : public ComBase<IVertexShader>{
	ID3D11VertexShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	TResourcePtr mRes;
public:
	TVertexShader11(IBlobDataPtr pBlob);
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}

	STDMETHODIMP_(IBlobDataPtr) GetBlob() override;
	ID3D11VertexShader*& GetShader11();
};
typedef ComPtr<TVertexShader11> TVertexShader11Ptr;

struct INHERIT_COM("D9F113F6-E51D-442C-9054-E4C3BE121EEE")
TPixelShader11 : public ComBase<IPixelShader> {
	ID3D11PixelShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
	TResourcePtr mRes;
public:
	TPixelShader11(IBlobDataPtr pBlob);
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}

	STDMETHODIMP_(IBlobDataPtr) GetBlob() override;
	ID3D11PixelShader*& GetShader11();
};
typedef ComPtr<TPixelShader11> TPixelShader11Ptr;

struct INHERIT_COM("9D141D12-7F62-4EB5-9C65-07E42DB96408")
TProgram11 : public ComBase<IProgram> {
	TVertexShader11Ptr mVertex;
	TPixelShader11Ptr mPixel;
	TResourcePtr mRes;
public:
	TProgram11();
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}
	void SetVertex(TVertexShader11Ptr pVertex) {
		mVertex = pVertex;
	}
	void SetPixel(TPixelShader11Ptr pPixel) {
		mPixel = pPixel;
	}
	STDMETHODIMP_(IVertexShaderPtr) GetVertex() override {
		return mVertex;
	}
	STDMETHODIMP_(IPixelShaderPtr) GetPixel() override {
		return mPixel;
	}
};
typedef ComPtr<TProgram11> TProgram11Ptr;

/********** HardwareBuffer **********/
struct THardwareBuffer {
	ID3D11Buffer* buffer;
	unsigned int bufferSize;
public:
	THardwareBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :buffer(__buffer), bufferSize(__bufferSize) {};
	THardwareBuffer() :buffer(nullptr), bufferSize(0) {};
};

struct INHERIT_COM("E83BE20F-59D6-4302-97A6-651792B35EF6")
TVertexBuffer11 : public ComBase<IVertexBuffer> {
	unsigned int stride, offset;
	THardwareBuffer hd;
public:
	TVertexBuffer11(ID3D11Buffer* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:hd(__buffer, __bufferSize), stride(__stride), offset(__offset) {};
	TVertexBuffer11() :stride(0), offset(0) {};
	int GetCount();
public:
	ID3D11Buffer*& GetBuffer11();
	STDMETHODIMP_(unsigned int) GetBufferSize() override;
	STDMETHODIMP_(enHardwareBufferType) GetType() override {
		return E_HWBUFFER_VERTEX;
	}

	STDMETHODIMP_(unsigned int) GetStride() override;
	STDMETHODIMP_(unsigned int) GetOffset() override;
};
typedef ComPtr<TVertexBuffer11> TVertexBuffer11Ptr;

struct INHERIT_COM("DB357B0A-9CCA-45F0-9DD9-6CE0E7D6E399")
TIndexBuffer11 : public ComBase<IIndexBuffer> {
	DXGI_FORMAT format;
	THardwareBuffer hd;
public:
	TIndexBuffer11(ID3D11Buffer* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
		:hd(__buffer, __bufferSize), format(__format) {};
	TIndexBuffer11() :format(DXGI_FORMAT_UNKNOWN) {};
public:
	ID3D11Buffer*& GetBuffer11();
	STDMETHODIMP_(unsigned int) GetBufferSize() override;
	STDMETHODIMP_(enHardwareBufferType) GetType() override {
		return E_HWBUFFER_INDEX;
	}

	STDMETHODIMP_(int) GetWidth() override;
	STDMETHODIMP_(DXGI_FORMAT) GetFormat() override;
};
typedef ComPtr<TIndexBuffer11> TIndexBuffer11Ptr;

struct INHERIT_COM("242AE076-20AB-45B4-94A5-2AC05E9F3861")
TContantBuffer11 : public ComBase<IContantBuffer> {
	TConstBufferDeclPtr mDecl;
	THardwareBuffer hd;
public:
	TContantBuffer11() {}
	TContantBuffer11(ID3D11Buffer* __buffer, TConstBufferDeclPtr decl);
public:
	STDMETHODIMP_(TConstBufferDeclPtr) GetDecl() override;
	STDMETHODIMP_(enHardwareBufferType) GetType() override {
		return E_HWBUFFER_CONSTANT;
	}

	ID3D11Buffer*& GetBuffer11();
	STDMETHODIMP_(unsigned int) GetBufferSize() override;
};
typedef ComPtr<TContantBuffer11> TContantBuffer11Ptr;

/********** Texture **********/
struct INHERIT_COM("8D8622A5-CCFD-4BB1-ACA5-922A89E174A2")
TTexture11 : public ComBase<ITexture> {
private:
	std::string path;
	ID3D11ShaderResourceView *texture;
	IResourcePtr mRes;
public:
	TTexture11(ID3D11ShaderResourceView* __texture, std::string __path);
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}

	STDMETHODIMP_(bool) HasSRV() override {
		return texture != nullptr;
	}
	void SetSRV11(ID3D11ShaderResourceView* __texture);
	ID3D11ShaderResourceView*& GetSRV11();

	STDMETHODIMP_(const char*) GetPath() override;
	STDMETHODIMP_(int) GetWidth() override;
	STDMETHODIMP_(int) GetHeight() override;
	STDMETHODIMP_(DXGI_FORMAT) GetFormat() override;
private:
	D3D11_TEXTURE2D_DESC GetDesc();
};
typedef ComPtr<TTexture11> TTexture11Ptr;

struct INHERIT_COM("8CE37B57-8702-4A14-BD43-9FB19455D410")
TRenderTexture11 : public ComBase<IRenderTexture> {
private:
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11ShaderResourceView* mRenderTargetSRV;
	ITexturePtr mRenderTargetPtr;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;

	DXGI_FORMAT mFormat;
public:
	TRenderTexture11(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	STDMETHODIMP_(ITexturePtr) GetColorTexture() override;

	ID3D11RenderTargetView*& GetColorBuffer11();
	ID3D11DepthStencilView*& GetDepthStencilBuffer11();
private:
	bool InitRenderTexture(ID3D11Device* pDevice, int width, int height);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);

	bool InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height);
	bool InitDepthStencilView(ID3D11Device* pDevice);
};
typedef ComPtr<TRenderTexture11> TRenderTexture11Ptr;

struct INHERIT_COM("04059656-CA19-432B-BBEC-41E46EFB8CCD")
TSamplerState11 : public ComBase<ISamplerState> {
	ID3D11SamplerState* mSampler = nullptr;
public:
	TSamplerState11(ID3D11SamplerState* sampler = nullptr) :mSampler(sampler) {};
	ID3D11SamplerState*& GetSampler11();
};
typedef ComPtr<TSamplerState11> TSamplerState11Ptr;