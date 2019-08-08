#pragma once
#include "TInterfaceType.h"

struct TBlobDataD3d11 : public IBlobData {
	ID3DBlob* mBlob = nullptr;
public:
	TBlobDataD3d11(ID3DBlob* pBlob);
	virtual void* GetBufferPointer() override;
	virtual size_t GetBufferSize() override;
};

struct TInputLayout11 : public IInputLayout {
public:
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDescs;
	ID3D11InputLayout* mLayout = nullptr;
public:
	virtual ID3D11InputLayout*& GetLayout11();
};
typedef std::shared_ptr<TInputLayout11> TInputLayout11Ptr;

struct TVertexShader11 : public IVertexShader {
	ID3D11VertexShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
public:
	TVertexShader11(IBlobDataPtr pBlob);
	virtual IUnknown*& GetDeviceObject() override;

	virtual IBlobDataPtr GetBlob();
	virtual ID3D11VertexShader*& GetShader11();
};
typedef std::shared_ptr<TVertexShader11> TVertexShader11Ptr;

struct TPixelShader11 : public IPixelShader {
	ID3D11PixelShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
public:
	TPixelShader11(IBlobDataPtr pBlob);
	virtual IUnknown*& GetDeviceObject() override;

	virtual IBlobDataPtr GetBlob() override;
	virtual ID3D11PixelShader*& GetShader11() override;
};
typedef std::shared_ptr<TPixelShader11> TPixelShader11Ptr;

struct TTexture11 : public ITexture {
private:
	std::string path;
	ID3D11ShaderResourceView *texture;
public:
	TTexture11(ID3D11ShaderResourceView* __texture, std::string __path);
	virtual IUnknown*& GetDeviceObject() override;

	void SetSRV11(ID3D11ShaderResourceView* __texture);
	ID3D11ShaderResourceView*& GetSRV11();

	const std::string& GetPath() const;
	int GetWidth();
	int GetHeight();
	DXGI_FORMAT GetFormat();
private:
	D3D11_TEXTURE2D_DESC GetDesc();
};
typedef std::shared_ptr<TTexture11> TTexture11Ptr;

struct THardwareBuffer {
	ID3D11Buffer* buffer;
	unsigned int bufferSize;
public:
	THardwareBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :buffer(__buffer), bufferSize(__bufferSize) {};
	THardwareBuffer() :buffer(nullptr), bufferSize(0) {};
};

struct TVertexBuffer11
	: public IVertexBuffer
	, public THardwareBuffer {
	unsigned int stride, offset;
public:
	TVertexBuffer11(ID3D11Buffer* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:THardwareBuffer(__buffer, __bufferSize), stride(__stride), offset(__offset) {};
	TVertexBuffer11() :stride(0), offset(0) {};
	int GetCount();
public:
	virtual ID3D11Buffer*& GetBuffer11() override;
	virtual unsigned int GetBufferSize() override;

	virtual unsigned int GetStride() override;
	virtual unsigned int GetOffset() override;
};
typedef std::shared_ptr<TVertexBuffer11> TVertexBuffer11Ptr;

struct TIndexBuffer11
	: public IIndexBuffer
	, public THardwareBuffer {
	DXGI_FORMAT format;
public:
	TIndexBuffer11(ID3D11Buffer* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
		:THardwareBuffer(__buffer, __bufferSize), format(__format) {};
	TIndexBuffer11() :format(DXGI_FORMAT_UNKNOWN) {};
public:
	virtual ID3D11Buffer*& GetBuffer11() override;
	virtual unsigned int GetBufferSize() override;

	virtual int GetWidth() override;
	virtual DXGI_FORMAT GetFormat() override;
};
typedef std::shared_ptr<TIndexBuffer11> TIndexBuffer11Ptr;

struct TContantBuffer11
	: public IContantBuffer
	, public THardwareBuffer {
	TContantBuffer11() {}
	TContantBuffer11(ID3D11Buffer* __buffer, unsigned int __bufferSize) :THardwareBuffer(__buffer, __bufferSize) {}
public:
	virtual ID3D11Buffer*& GetBuffer11() override;
	virtual unsigned int GetBufferSize() override;
};
typedef std::shared_ptr<TContantBuffer11> TContantBuffer11Ptr;

class TRenderTexture11 : public IRenderTexture {
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
	virtual ITexturePtr GetColorTexture() override;

	virtual ID3D11RenderTargetView*& GetColorBuffer11() override;
	virtual ID3D11DepthStencilView*& GetDepthStencilBuffer11() override;
private:
	bool InitRenderTexture(ID3D11Device* pDevice, int width, int height);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);

	bool InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height);
	bool InitDepthStencilView(ID3D11Device* pDevice);
};
typedef std::shared_ptr<TRenderTexture11> TRenderTexture11Ptr;