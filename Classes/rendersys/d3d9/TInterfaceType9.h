#pragma once
#include "TInterfaceType.h"

struct INHERIT_COM("671C236B-006F-4C27-98B3-C57DF7915D16")
TBlobDataD3d9 : public ComBase<IBlobData> {
	ID3DXBuffer* mBlob = nullptr;
public:
	TBlobDataD3d9(ID3DXBuffer* pBlob);
	STDMETHODIMP_(char*) GetBufferPointer() override;
	STDMETHODIMP_(size_t) GetBufferSize() override;
};
typedef ComPtr<TBlobDataD3d9> TBlobDataD3d9Ptr;

struct INHERIT_COM("69ED611C-C5DB-460F-8110-63E89FC5DA7B")
TInputLayout9 : public ComBase<IInputLayout> {
public:
	std::vector<D3DVERTEXELEMENT9> mInputDescs;
	IDirect3DVertexDeclaration9* mLayout = nullptr;
	IResourcePtr mRes;
public:
	TInputLayout9();
	IDirect3DVertexDeclaration9*& GetLayout9();
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}
};
typedef ComPtr<TInputLayout9> TInputLayout9Ptr;

struct TConstantTable {
	ID3DXConstantTable* mTable = nullptr;
	std::vector<D3DXHANDLE> mHandles;
	std::map<std::string, D3DXHANDLE> mHandleByName;
	std::map<std::string, std::shared_ptr<TConstantTable>> mSubByName;
public:
	void ReInit();
	void Init(ID3DXConstantTable* constTable, D3DXHANDLE handle = NULL, int count = 0);
	ID3DXConstantTable* get();
	size_t size() const;
	D3DXHANDLE GetHandle(size_t pos) const;
	D3DXHANDLE GetHandle(const std::string& name) const;
	std::shared_ptr<TConstantTable> At(const std::string& name);
public:
	void SetValue(IDirect3DDevice9* device, char* buffer9, TConstBufferDeclElement& elem);
	void SetValue(IDirect3DDevice9* device, char* buffer9, TConstBufferDecl& decl);
};
typedef std::shared_ptr<TConstantTable> TConstantTablePtr;

struct INHERIT_COM("A86E63AD-B9A1-4C4A-828A-444BB3ABF478")
TVertexShader9 : public ComBase<IVertexShader> {
	TConstantTable mConstTable;
	IBlobDataPtr mBlob,mErrBlob;
	IDirect3DVertexShader9* mShader = nullptr;
	IResourcePtr mRes;
public:
	TVertexShader9();
	STDMETHODIMP_(IBlobDataPtr) GetBlob() override;
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}

	IDirect3DVertexShader9*& GetShader9();
	void SetConstTable(ID3DXConstantTable* constTable);
};
typedef ComPtr<TVertexShader9> TVertexShader9Ptr;

struct INHERIT_COM("71FE7436-1A61-4F32-B690-D9FD117433D5")
TPixelShader9 : public ComBase<IPixelShader> {
	TConstantTable mConstTable;
	IBlobDataPtr mBlob, mErrBlob;
	IDirect3DPixelShader9* mShader = nullptr;
	IResourcePtr mRes;
public:
	TPixelShader9();
	virtual STDMETHODIMP_(IBlobDataPtr) GetBlob() override;
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}

	IDirect3DPixelShader9*& GetShader9();
	void SetConstTable(ID3DXConstantTable* constTable);
};
typedef ComPtr<TPixelShader9> TPixelShader9Ptr;

struct INHERIT_COM("E752C828-54D6-44F5-BF79-26461299779D")
TProgram9 : public ComBase<IProgram> {
	TVertexShader9Ptr mVertex;
	TPixelShader9Ptr mPixel;
	IResourcePtr mRes;
public:
	TProgram9();
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}
	void SetVertex(TVertexShader9Ptr pVertex);
	void SetPixel(TPixelShader9Ptr pPixel);
	STDMETHODIMP_(IVertexShaderPtr) GetVertex() override {
		return mVertex;
	}
	STDMETHODIMP_(IPixelShaderPtr) GetPixel() override {
		return mPixel;
	}
};
typedef ComPtr<TProgram9> TProgram9Ptr;

/********** HardwareBuffer **********/
struct INHERIT_COM("8E88ACBA-26ED-4C83-A012-A6B161567D11")
TIndexBuffer9 : public ComBase<IIndexBuffer> {
	IDirect3DIndexBuffer9* buffer;
	unsigned int bufferSize;
	DXGI_FORMAT format;
public:
	TIndexBuffer9(IDirect3DIndexBuffer9* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
		:buffer(__buffer), bufferSize(__bufferSize), format(__format) {};
	TIndexBuffer9() :buffer(nullptr), bufferSize(0), format(DXGI_FORMAT_UNKNOWN) {};
public:
	IDirect3DIndexBuffer9*& GetBuffer9();
	STDMETHODIMP_(enHardwareBufferType) GetType() override {
		return E_HWBUFFER_INDEX;
	}
	STDMETHODIMP_(unsigned int) GetBufferSize() override;

	STDMETHODIMP_(int) GetWidth() override;
	STDMETHODIMP_(DXGI_FORMAT) GetFormat() override;
};
typedef ComPtr<TIndexBuffer9> TIndexBuffer9Ptr;

struct INHERIT_COM("6BEE478A-8730-4DC4-A732-A34A78722A30")
TVertexBuffer9 : public ComBase<IVertexBuffer> {
	IDirect3DVertexBuffer9* buffer;
	unsigned int bufferSize;
	unsigned int stride, offset;
public:
	TVertexBuffer9(IDirect3DVertexBuffer9* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:buffer(__buffer), bufferSize(__bufferSize), stride(__stride), offset(__offset) {};
	
	IDirect3DVertexBuffer9*& GetBuffer9();
	STDMETHODIMP_(enHardwareBufferType) GetType() override {
		return E_HWBUFFER_VERTEX;
	}
	STDMETHODIMP_(unsigned int) GetBufferSize() override;

	STDMETHODIMP_(unsigned int) GetStride() override;
	STDMETHODIMP_(unsigned int) GetOffset() override;
};
typedef ComPtr<TVertexBuffer9> TVertexBuffer9Ptr;

struct INHERIT_COM("FDCC7A1A-C780-4A83-BFFF-2721F3A0D562")
TContantBuffer9 : public ComBase<IContantBuffer> {
	TConstBufferDeclPtr mDecl;
	std::vector<char> mBuffer9;
public:
	TContantBuffer9(TConstBufferDeclPtr decl);
public:
	STDMETHODIMP_(enHardwareBufferType) GetType() override {
		return E_HWBUFFER_CONSTANT;
	}
	STDMETHODIMP_(unsigned int) GetBufferSize() override;

	STDMETHODIMP_(TConstBufferDeclPtr) GetDecl() override;
	char* GetBuffer9();
	void SetBuffer9(char* data, int dataSize);
};
typedef ComPtr<TContantBuffer9> TContantBuffer9Ptr;

/********** Texture **********/
struct INHERIT_COM("911D06C1-544B-4977-8367-1C74C3EC3113")
TTexture9 : public ComBase<ITexture> {
private:
	int mWidth, mHeight;
	DXGI_FORMAT mFormat;
	IDirect3DTexture9 *mTexture;
	IDirect3DCubeTexture9* mTextureCube;
	IResourcePtr mRes;
	std::string mPath;
public:
	TTexture9(int width, int height, DXGI_FORMAT format);
	TTexture9(IDirect3DTexture9 *__texture, const std::string& __path);
	STDMETHODIMP_(IResourcePtr) AsRes() override {
		return mRes;
	}

	STDMETHODIMP_(bool) HasSRV() override {
		return mTexture != nullptr || mTextureCube != nullptr;
	}
	void SetSRV9(IDirect3DTexture9* __texture);
	IDirect3DTexture9*& GetSRV9() {
		return mTexture;
	}
	IDirect3DCubeTexture9*& GetSRVCube9() {
		return mTextureCube;
	}

	STDMETHODIMP_(const char*) GetPath() override;
	STDMETHODIMP_(int) GetWidth() override;
	STDMETHODIMP_(int) GetHeight() override;
	STDMETHODIMP_(DXGI_FORMAT) GetFormat() override;
private:
	D3DSURFACE_DESC GetDesc();
};
typedef ComPtr<TTexture9> TTexture9Ptr;

struct INHERIT_COM("455BD4A6-AF0B-40B6-BAEB-CDF4C94189D8")
TRenderTexture9 : public ComBase<IRenderTexture> {
	IDirect3DSurface9* mDepthStencilBuffer;
	IDirect3DSurface9* mColorBuffer;
	TTexture9Ptr mColorTexture;
public:
	TRenderTexture9(TTexture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer);
	STDMETHODIMP_(ITexturePtr) GetColorTexture() override;
	IDirect3DSurface9*& GetColorBuffer9();
	IDirect3DSurface9*& GetDepthStencilBuffer9();
};
typedef ComPtr<TRenderTexture9> TRenderTexture9Ptr;

struct INHERIT_COM("7BC9743D-B003-4298-8574-4FFE3E7E97D5")
TSamplerState9 : public ComBase<ISamplerState> {
	std::map<D3DSAMPLERSTATETYPE, DWORD> mStates;
public:
	std::map<D3DSAMPLERSTATETYPE, DWORD>& GetSampler9();
};
typedef ComPtr<TSamplerState9> TSamplerState9Ptr;