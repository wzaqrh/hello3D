#pragma once
#include "TPredefine.h"
#include "IResource.h"

/********** Program **********/
struct IBlobData {
	virtual void* GetBufferPointer() = 0;
	virtual size_t GetBufferSize() = 0;
};
struct TBlobDataStd : public IBlobData {
	std::vector<char> mBuffer;
public:
	TBlobDataStd(const std::vector<char>& buffer);
	virtual void* GetBufferPointer() override;
	virtual size_t GetBufferSize() override;
};
typedef std::shared_ptr<IBlobData> IBlobDataPtr;

struct IInputLayout : public IResource {
	virtual ID3D11InputLayout*& GetLayout11();
	virtual IDirect3DVertexDeclaration9*& GetLayout9();
};
typedef std::shared_ptr<IInputLayout> IInputLayoutPtr;

struct IVertexShader : public IResource {
	virtual IBlobDataPtr GetBlob() = 0;
	virtual ID3D11VertexShader*& GetShader11();
	virtual IDirect3DVertexShader9*& GetShader9();
};
typedef std::shared_ptr<IVertexShader> IVertexShaderPtr;

struct IPixelShader : public IResource {
public:
	virtual IBlobDataPtr GetBlob() = 0;
	virtual ID3D11PixelShader*& GetShader11();
	virtual IDirect3DPixelShader9*& GetShader9();
};
typedef std::shared_ptr<IPixelShader> IPixelShaderPtr;

struct TProgram : public IResource {
	IVertexShaderPtr mVertex;
	IPixelShaderPtr mPixel;
public:
	void SetVertex(IVertexShaderPtr pVertex);
	void SetPixel(IPixelShaderPtr pPixel);
};
typedef std::shared_ptr<TProgram> TProgramPtr;

/********** HardwareBuffer **********/
enum enHardwareBufferType {
	E_HWBUFFER_CONSTANT,
	E_HWBUFFER_VERTEX,
	E_HWBUFFER_INDEX
};
struct IHardwareBuffer {
	virtual enHardwareBufferType GetType() = 0;
	virtual ID3D11Buffer*& GetBuffer11();
	virtual unsigned int GetBufferSize();
};

struct IVertexBuffer : public IHardwareBuffer {
	virtual enHardwareBufferType GetType() override final;

	virtual IDirect3DVertexBuffer9*& GetBuffer9();
	virtual unsigned int GetStride() = 0;
	virtual unsigned int GetOffset() = 0;
};
typedef std::shared_ptr<IVertexBuffer> IVertexBufferPtr;

struct IIndexBuffer : public IHardwareBuffer {
	virtual enHardwareBufferType GetType() override final;

	virtual IDirect3DIndexBuffer9*& GetBuffer9();
	virtual int GetWidth() = 0;
	virtual DXGI_FORMAT GetFormat() = 0;
	int GetCount();
};
typedef std::shared_ptr<IIndexBuffer> IIndexBufferPtr;

struct IContantBuffer : public IHardwareBuffer {
	virtual enHardwareBufferType GetType() override final;

	virtual void* GetBuffer9();
	virtual TConstBufferDeclPtr GetDecl() = 0;
};
typedef std::shared_ptr<IContantBuffer> IContantBufferPtr;

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
#define E_TEXTURE_DEPTH_MAP 4
#define E_TEXTURE_ENV 5

struct ITexture : public IResource {
	virtual void SetSRV11(ID3D11ShaderResourceView* __texture) {};
	virtual ID3D11ShaderResourceView*& GetSRV11();
	
	virtual void SetSRV9(IDirect3DTexture9* __texture) {};
	virtual IDirect3DTexture9*& GetSRV9();
	virtual IDirect3DCubeTexture9*& GetSRVCube9();

	virtual const std::string& GetPath() const = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual DXGI_FORMAT GetFormat() = 0;
};
typedef std::shared_ptr<ITexture> ITexturePtr;

struct IRenderTexture {
	virtual ITexturePtr GetColorTexture() = 0;

	virtual ID3D11RenderTargetView*& GetColorBuffer11();
	virtual ID3D11DepthStencilView*& GetDepthStencilBuffer11();

	virtual IDirect3DSurface9*& GetColorBuffer9();
	virtual IDirect3DSurface9*& GetDepthStencilBuffer9();
};
typedef std::shared_ptr<IRenderTexture> IRenderTexturePtr;

struct ISamplerState {
	virtual ID3D11SamplerState*& GetSampler11();
	virtual std::map<D3DSAMPLERSTATETYPE, DWORD>& GetSampler9();
};
typedef std::shared_ptr<ISamplerState> ISamplerStatePtr;