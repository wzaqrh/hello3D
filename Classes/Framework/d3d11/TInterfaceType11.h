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

struct TVertex11Buffer
	: public IVertexBuffer
	, public THardwareBuffer {
	unsigned int stride, offset;
public:
	TVertex11Buffer(ID3D11Buffer* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:THardwareBuffer(__buffer, __bufferSize), stride(__stride), offset(__offset) {};
	TVertex11Buffer() :stride(0), offset(0) {};
	int GetCount();
public:
	virtual ID3D11Buffer*& GetBuffer11() override;
	virtual unsigned int GetBufferSize() override;
	virtual unsigned int GetStride() override;
	virtual unsigned int GetOffset() override;
};
typedef std::shared_ptr<TVertex11Buffer> TVertex11BufferPtr;