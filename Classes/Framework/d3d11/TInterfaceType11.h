#pragma once
#include "TInterfaceType.h"

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