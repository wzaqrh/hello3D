#pragma once
#include <Windows.h>
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"

namespace mir {

/********** Program **********/
interface IBlobData  
{
	virtual char* GetBufferPointer() = 0;
	virtual size_t GetBufferSize() = 0;
};

interface BlobDataStandard : public IBlobData 
{
	std::vector<char> mBuffer;
public:
	BlobDataStandard(const std::vector<char>& buffer);
	char* GetBufferPointer() override;
	size_t GetBufferSize() override;
};

interface IInputLayout  
{
	virtual IResourcePtr AsRes() = 0;
};

interface IVertexShader 
{
	virtual IBlobDataPtr GetBlob() = 0;
	virtual IResourcePtr AsRes() = 0;
};

interface IPixelShader 
{
public:
	virtual IBlobDataPtr GetBlob() = 0;
	virtual IResourcePtr AsRes() = 0;
};

interface IProgram 
{
	virtual IVertexShaderPtr GetVertex() = 0;
	virtual IPixelShaderPtr GetPixel() = 0;
	virtual IResourcePtr AsRes() = 0;
};

/********** HardwareBuffer **********/
enum HardwareBufferType {
	kHWBufferConstant,
	kHWBufferVertex,
	kHWBufferIndex
};
interface IHardwareBuffer 
{
	virtual HardwareBufferType GetType() = 0;
	virtual unsigned int GetBufferSize() = 0;
};

interface IVertexBuffer : public IHardwareBuffer 
{
	virtual unsigned int GetStride() = 0;
	virtual unsigned int GetOffset() = 0;
};

interface IIndexBuffer : public IHardwareBuffer 
{
	virtual int GetWidth() = 0;
	virtual ResourceFormat GetFormat() = 0;
	int GetCount();
};

interface IContantBuffer : public IHardwareBuffer 
{
	virtual ConstBufferDeclPtr GetDecl() = 0;
};

/********** Texture **********/
enum TextureType {
	kTextureDiffuse,
	kTextureSpecular,
	kTextureNormal
};
enum TexturePbrType {
	kTexturePbrAlbedo,
	kTexturePbrNormal,
	kTexturePbrMetalness,
	kTexturePbrRoughness,
	kTexturePbrAo
};
#define E_TEXTURE_MAIN 0
#define E_TEXTURE_DEPTH_MAP 8
#define E_TEXTURE_ENV 9

interface ITexture 
{
	virtual IResourcePtr AsRes() = 0;
	virtual bool HasSRV() = 0;

	virtual const char* GetPath() = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual ResourceFormat GetFormat() = 0;
	virtual int GetMipmapCount() = 0;
};

interface IRenderTexture  
{
	virtual ITexturePtr GetColorTexture() = 0;
};

interface ISamplerState 
{
};

}