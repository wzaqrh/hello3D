#pragma once
#include "TPredefine.h"
#include "IResource.h"

/********** Program **********/
struct IBlobData {
	virtual void* GetBufferPointer() = 0;
	virtual size_t GetBufferSize() = 0;
};
struct TBlobDataD3d : public IBlobData {
	ID3DBlob* mBlob = nullptr;
public:
	TBlobDataD3d(ID3DBlob* pBlob);
	virtual void* GetBufferPointer() override;
	virtual size_t GetBufferSize() override;
};
struct TBlobDataStd : public IBlobData {
	std::vector<char> mBuffer;
public:
	TBlobDataStd(const std::vector<char>& buffer);
	virtual void* GetBufferPointer() override;
	virtual size_t GetBufferSize() override;
};
typedef std::shared_ptr<IBlobData> IBlobDataPtr;

struct TVertexShader : public IResource {
	ID3D11VertexShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
public:
	TVertexShader(IBlobDataPtr pBlob);
	virtual IUnknown*& GetDeviceObject() override;
};
typedef std::shared_ptr<TVertexShader> TVertexShaderPtr;

struct TPixelShader : public IResource {
	ID3D11PixelShader* mShader = nullptr;
	IBlobDataPtr mBlob;
	ID3DBlob* mErrBlob = nullptr;
public:
	TPixelShader(IBlobDataPtr pBlob);
	virtual IUnknown*& GetDeviceObject() override;
};
typedef std::shared_ptr<TPixelShader> TPixelShaderPtr;

struct TProgram : public IResource {
	TVertexShaderPtr mVertex;
	TPixelShaderPtr mPixel;
public:
	void SetVertex(TVertexShaderPtr pVertex);
	void SetPixel(TPixelShaderPtr pPixel);
};
typedef std::shared_ptr<TProgram> TProgramPtr;

struct TInputLayout : public IResource {
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDescs;
	ID3D11InputLayout* mLayout = nullptr;
};
typedef std::shared_ptr<TInputLayout> TInputLayoutPtr;

/********** HardwareBuffer **********/
struct IHardwareBuffer {
	virtual ID3D11Buffer*& GetBuffer11();
	virtual unsigned int GetBufferSize();
};

struct THardwareBuffer {
	ID3D11Buffer* buffer;
	unsigned int bufferSize;
public:
	THardwareBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :buffer(__buffer), bufferSize(__bufferSize) {};
	THardwareBuffer() :buffer(nullptr), bufferSize(0) {};
};

struct IVertexBuffer : public IHardwareBuffer {
	virtual IDirect3DVertexBuffer9*& GetBuffer9();
	virtual unsigned int GetStride();
	virtual unsigned int GetOffset();
};
typedef std::shared_ptr<IVertexBuffer> IVertexBufferPtr;

struct TIndexBuffer : public THardwareBuffer {
	DXGI_FORMAT format;
public:
	TIndexBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
	:THardwareBuffer(__buffer, __bufferSize), format(__format) {};
	TIndexBuffer() :format(DXGI_FORMAT_UNKNOWN) {};
	int GetWidth();
};
typedef std::shared_ptr<TIndexBuffer> TIndexBufferPtr;

struct TContantBuffer : public THardwareBuffer {
	TContantBuffer() {}
	TContantBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :THardwareBuffer(__buffer, __bufferSize) {}
};
typedef std::shared_ptr<TContantBuffer> TContantBufferPtr;

/********** Texture **********/
enum enTextureType {
	E_TEXTURE_DIFFUSE,
	E_TEXTURE_SPECULAR,
	E_TEXTURE_NORMAL
};
enum enTexturePbrType {
	E_TEXTURE_PBR_ALBEDO,
	E_TEXTURE_PBR_NORMAL,
	E_TEXTURE_PBR_METALNESS,
	E_TEXTURE_PBR_ROUGHNESS,
	E_TEXTURE_PBR_AO
};
#define E_TEXTURE_DEPTH_MAP 8
#define E_TEXTURE_ENV 9

struct ITexture : public IResource {
	virtual void SetSRV11(ID3D11ShaderResourceView* __texture) {};
	virtual ID3D11ShaderResourceView*& GetSRV11();
	
	virtual void SetSRV9(IDirect3DTexture9* __texture) {};
	virtual IDirect3DTexture9*& GetSRV9();

	virtual const std::string& GetPath() const = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual DXGI_FORMAT GetFormat() = 0;
};
typedef std::shared_ptr<ITexture> ITexturePtr;

class TRenderTexture {
public:
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11ShaderResourceView* mRenderTargetSRV;
	ITexturePtr mRenderTargetPtr;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;

	DXGI_FORMAT mFormat;
public:
	TRenderTexture(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	ITexturePtr GetRenderTargetSRV();
private:
	bool InitRenderTexture(ID3D11Device* pDevice, int width, int height);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);

	bool InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height);
	bool InitDepthStencilView(ID3D11Device* pDevice);
};
typedef std::shared_ptr<TRenderTexture> TRenderTexturePtr;

