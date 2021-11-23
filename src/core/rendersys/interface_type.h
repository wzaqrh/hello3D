#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/resource/resource.h"

namespace mir {

/********** Program **********/
interface IBlobData : boost::noncopyable  
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

interface IInputLayout : public IResource  
{
};

/********** Program **********/
enum ShaderType {
	kShaderVertex,
	kShaderPixel
};

interface IShader : public IResource 
{
	virtual ShaderType GetType() = 0;
	virtual IBlobDataPtr GetBlob() = 0;
};

interface IVertexShader : public IShader 
{	
};

interface IPixelShader : public IShader 
{
};

interface IProgram : public IResource 
{
	virtual IVertexShaderPtr GetVertex() = 0;
	virtual IPixelShaderPtr GetPixel() = 0;
};

/********** HardwareBuffer **********/
enum HardwareBufferType {
	kHWBufferConstant,
	kHWBufferVertex,
	kHWBufferIndex
};
interface IHardwareBuffer : public IResource  
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

interface ITexture : public IResource  
{
	virtual bool HasSRV() = 0;

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual ResourceFormat GetFormat() = 0;
	virtual int GetMipmapCount() = 0;
};

interface IRenderTexture : public IResource   
{
	virtual ITexturePtr GetColorTexture() = 0;
};

interface ISamplerState : public IResource 
{
};

}