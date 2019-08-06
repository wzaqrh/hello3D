#pragma once
#include "TPredefine.h"
#include "IResource.h"

/********** Program **********/
struct TVertexShader : public IResource {
	ID3D11VertexShader* mShader = nullptr;
	ID3DBlob* mBlob = nullptr;
	ID3DBlob* mErrBlob = nullptr;
};
typedef std::shared_ptr<TVertexShader> TVertexShaderPtr;

struct TPixelShader : public IResource {
	ID3D11PixelShader* mShader = nullptr;
	ID3DBlob* mBlob = nullptr;
	ID3DBlob* mErrBlob = nullptr;
};
typedef std::shared_ptr<TPixelShader> TPixelShaderPtr;

struct TProgram : public IResource {
	TVertexShaderPtr mVertex;
	TPixelShaderPtr mPixel;
};
typedef std::shared_ptr<TProgram> TProgramPtr;

struct TInputLayout : public IResource {
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDescs;
	ID3D11InputLayout* mLayout = nullptr;
};
typedef std::shared_ptr<TInputLayout> TInputLayoutPtr;

/********** HardwareBuffer **********/
struct THardwareBuffer {
	ID3D11Buffer* buffer;
	unsigned int bufferSize;
public:
	THardwareBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :buffer(__buffer), bufferSize(__bufferSize) {};
	THardwareBuffer() :buffer(nullptr), bufferSize(0) {};
};

struct TVertexBuffer : public THardwareBuffer {
	unsigned int stride, offset;
public:
	TVertexBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:THardwareBuffer(__buffer, __bufferSize), stride(__stride), offset(__offset) {};
	TVertexBuffer() :stride(0), offset(0) {};
	int GetCount();
};
typedef std::shared_ptr<TVertexBuffer> TVertexBufferPtr;

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
struct TTexture : public IResource {
private:
	std::string path;
	ID3D11ShaderResourceView *texture;
public:
	//TTexture();
	TTexture(ID3D11ShaderResourceView* __texture, std::string __path);
	void SetSRV(ID3D11ShaderResourceView* __texture);
public:
	const std::string& GetPath() const;
	ID3D11ShaderResourceView*& GetSRV();
public:
	D3D11_TEXTURE2D_DESC GetDesc();
	int GetWidth();
	int GetHeight();
	DXGI_FORMAT GetFormat();
};
typedef std::shared_ptr<TTexture> TTexturePtr;

class TRenderTexture {
public:
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11ShaderResourceView* mRenderTargetSRV;
	TTexturePtr mRenderTargetPtr;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;

	DXGI_FORMAT mFormat;
public:
	TRenderTexture(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	TTexturePtr GetRenderTargetSRV();
private:
	bool InitRenderTexture(ID3D11Device* pDevice, int width, int height);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);

	bool InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height);
	bool InitDepthStencilView(ID3D11Device* pDevice);
};
typedef std::shared_ptr<TRenderTexture> TRenderTexturePtr;

