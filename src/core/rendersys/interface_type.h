#pragma once
#include "core/rendersys/interface_type_pred.h"

namespace mir {

/********** Program **********/
interface IBlobData  {
	virtual char* GetBufferPointer() = 0;
	virtual size_t GetBufferSize() = 0;
};

interface TBlobDataStd : public IBlobData {
	std::vector<char> mBuffer;
public:
	TBlobDataStd(const std::vector<char>& buffer);
	char* GetBufferPointer() override;
	size_t GetBufferSize() override;
};

interface IInputLayout  {
	//virtual ID3D11InputLayout*& GetLayout11();
	//virtual IDirect3DVertexDeclaration9*& GetLayout9();
	virtual IResourcePtr AsRes() = 0;
};

interface IVertexShader {
	virtual IBlobDataPtr GetBlob() = 0;
	//virtual ID3D11VertexShader*& GetShader11();
	//virtual IDirect3DVertexShader9*& GetShader9();
	virtual IResourcePtr AsRes() = 0;
};

interface IPixelShader {
public:
	virtual IBlobDataPtr GetBlob() = 0;
	//virtual ID3D11PixelShader*& GetShader11();
	//virtual IDirect3DPixelShader9*& GetShader9();
	virtual IResourcePtr AsRes() = 0;
};

interface IProgram {
	virtual IVertexShaderPtr GetVertex() = 0;
	virtual IPixelShaderPtr GetPixel() = 0;
	virtual IResourcePtr AsRes() = 0;
};

/********** HardwareBuffer **********/
enum enHardwareBufferType {
	E_HWBUFFER_CONSTANT,
	E_HWBUFFER_VERTEX,
	E_HWBUFFER_INDEX
};
interface IHardwareBuffer  {
	virtual enHardwareBufferType GetType() = 0;
	//virtual ID3D11Buffer*& GetBuffer11();
	virtual unsigned int GetBufferSize() = 0;
};

interface IVertexBuffer : public IHardwareBuffer {
	//virtual enHardwareBufferType GetType() override final;

	//virtual IDirect3DVertexBuffer9*& GetBuffer9();
	virtual unsigned int GetStride() = 0;
	virtual unsigned int GetOffset() = 0;
};

interface IIndexBuffer : public IHardwareBuffer {
	//virtual enHardwareBufferType GetType() override final;

	//virtual IDirect3DIndexBuffer9*& GetBuffer9();
	virtual int GetWidth() = 0;
	virtual DXGI_FORMAT GetFormat() = 0;
	int GetCount();
};

interface IContantBuffer : public IHardwareBuffer {
	//virtual enHardwareBufferType GetType() override final;

	//virtual void* GetBuffer9();
	virtual TConstBufferDeclPtr GetDecl() = 0;
};

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

interface ITexture {
	//virtual void SetSRV11(ID3D11ShaderResourceView* __texture) {};
	//virtual ID3D11ShaderResourceView*& GetSRV11();
	
	//virtual void SetSRV9(IDirect3DTexture9* __texture) {};
	//virtual IDirect3DTexture9*& GetSRV9();
	//virtual IDirect3DCubeTexture9*& GetSRVCube9();
	virtual IResourcePtr AsRes() = 0;
	virtual bool HasSRV() = 0;

	virtual const char* GetPath() = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual DXGI_FORMAT GetFormat() = 0;
	virtual int GetMipmapCount() = 0;
};

interface IRenderTexture  {
	virtual ITexturePtr GetColorTexture() = 0;

	//virtual ID3D11RenderTargetView*& GetColorBuffer11();
	//virtual ID3D11DepthStencilView*& GetDepthStencilBuffer11();

	//virtual IDirect3DSurface9*& GetColorBuffer9();
	//virtual IDirect3DSurface9*& GetDepthStencilBuffer9();
};

interface ISamplerState {
	//virtual ID3D11SamplerState*& GetSampler11();
	//virtual std::map<D3DSAMPLERSTATETYPE, DWORD>& GetSampler9();
};

}