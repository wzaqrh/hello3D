#pragma once
#include "TInterfaceType.h"

struct TBlobDataD3d9 : public IBlobData {
	ID3DXBuffer* mBlob = nullptr;
public:
	TBlobDataD3d9(ID3DXBuffer* pBlob);
	virtual void* GetBufferPointer() override;
	virtual size_t GetBufferSize() override;
};
typedef std::shared_ptr<TBlobDataD3d9> TBlobDataD3d9Ptr;

struct TInputLayout9 : public IInputLayout {
public:
	std::vector<D3DVERTEXELEMENT9> mInputDescs;
	IDirect3DVertexDeclaration9* mLayout = nullptr;
public:
	virtual IDirect3DVertexDeclaration9*& GetLayout9() override;
};
typedef std::shared_ptr<TInputLayout9> TInputLayout9Ptr;

struct TVertexShader9 : public IVertexShader {
	ID3DXConstantTable* mConstTable = nullptr;
	std::vector<D3DXHANDLE> mConstHandles;
	IBlobDataPtr mBlob,mErrBlob;
	IDirect3DVertexShader9* mShader = nullptr;
public:
	virtual IBlobDataPtr GetBlob() override;
	virtual IDirect3DVertexShader9*& GetShader9() override;
	void SetConstTable(ID3DXConstantTable* constTable);
};
typedef std::shared_ptr<TVertexShader9> TVertexShader9Ptr;

struct TPixelShader9 : public IPixelShader {
	ID3DXConstantTable* mConstTable = nullptr;
	std::vector<D3DXHANDLE> mConstHandles;
	IBlobDataPtr mBlob, mErrBlob;
	IDirect3DPixelShader9* mShader = nullptr;
public:
	virtual IBlobDataPtr GetBlob() override;
	virtual IDirect3DPixelShader9*& GetShader9() override;
	void SetConstTable(ID3DXConstantTable* constTable);
};
typedef std::shared_ptr<TPixelShader9> TPixelShader9Ptr;

/********** HardwareBuffer **********/
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

struct TContantBuffer9 : public IContantBuffer {
	TConstBufferDeclPtr mDecl;
	std::vector<char> mBuffer9;
public:
	TContantBuffer9(TConstBufferDeclPtr decl);
public:
	virtual TConstBufferDeclPtr GetDecl() override;
	virtual unsigned int GetBufferSize() override;
	virtual void* GetBuffer9() override;
};
typedef std::shared_ptr<TContantBuffer9> TContantBuffer9Ptr;

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

struct TSamplerState9 : public ISamplerState {
	std::map<D3DSAMPLERSTATETYPE, DWORD> mStates;
public:
	virtual std::map<D3DSAMPLERSTATETYPE, DWORD>& GetSampler9() override;
};
typedef std::shared_ptr<TSamplerState9> TSamplerState9Ptr;