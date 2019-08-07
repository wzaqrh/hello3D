#pragma once
#include "TInterfaceType.h"

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