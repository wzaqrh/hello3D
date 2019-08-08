#pragma once
#include "TInterfaceType.h"

struct TIndexBuffer9 : public IIndexBuffer {
	IDirect3DIndexBuffer9* buffer;
	unsigned int bufferSize;
	DXGI_FORMAT format;
public:
	TIndexBuffer9(IDirect3DIndexBuffer9* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
		:buffer(__buffer), bufferSize(__bufferSize), format(__format) {};
	TIndexBuffer9() :buffer(nullptr), bufferSize(0), format(DXGI_FORMAT_UNKNOWN) {};
public:
	virtual IDirect3DIndexBuffer9*& GetBuffer9();
	virtual unsigned int GetBufferSize() override;

	virtual int GetWidth() override;
	virtual DXGI_FORMAT GetFormat() override;
};
typedef std::shared_ptr<TIndexBuffer9> TIndexBuffer9Ptr;

struct TVertexBuffer9 : public IVertexBuffer {
	IDirect3DVertexBuffer9* buffer;
	unsigned int bufferSize;
	unsigned int stride, offset;
public:
	TVertexBuffer9(IDirect3DVertexBuffer9* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:buffer(__buffer), bufferSize(__bufferSize), stride(__stride), offset(__offset) {};
	virtual unsigned int GetBufferSize() override;
	virtual IDirect3DVertexBuffer9*& GetBuffer9() override;
	
	virtual unsigned int GetStride() override;
	virtual unsigned int GetOffset() override;
};
typedef std::shared_ptr<TVertexBuffer9> TVertexBuffer9Ptr;

/********** Texture **********/
struct TTexture9 : public ITexture {
private:
	std::string path;
	IDirect3DTexture9 *texture;
public:
	TTexture9(IDirect3DTexture9* __texture, std::string __path);
	virtual IUnknown*& GetDeviceObject() override;

	virtual void SetSRV9(IDirect3DTexture9* __texture);
	virtual IDirect3DTexture9*& GetSRV9();

	const std::string& GetPath() const;
	int GetWidth();
	int GetHeight();
	DXGI_FORMAT GetFormat();
private:
	D3DSURFACE_DESC GetDesc();
};
typedef std::shared_ptr<TTexture9> TTexture9Ptr;

class TRenderTexture9 : public IRenderTexture {
	IDirect3DSurface9* mColorBuffer;
	IDirect3DSurface9* mDepthStencilBuffer;
	TTexture9Ptr mColorTexture;
public:
	TRenderTexture9(IDirect3DSurface9* colorBuffer, IDirect3DSurface9* depthStencilBuffer);
	virtual ITexturePtr GetColorTexture() override;
	virtual IDirect3DSurface9*& GetColorBuffer9() override;
	virtual IDirect3DSurface9*& GetDepthStencilBuffer9() override;
};
typedef std::shared_ptr<TRenderTexture9> TRenderTexture9Ptr;