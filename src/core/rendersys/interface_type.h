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
	virtual const char* GetBufferPointer() const = 0;
	virtual size_t GetBufferSize() const = 0;
};

interface BlobDataStandard : public IBlobData 
{
public:
	BlobDataStandard(const std::vector<char>& buffer) :mBuffer(buffer) {}
	const char* GetBufferPointer() const override { return mBuffer.empty() ? nullptr : &mBuffer[0]; }
	size_t GetBufferSize() const override { return mBuffer.size(); }
public:
	std::vector<char> mBuffer;
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
	virtual ShaderType GetType() const = 0;
	virtual IBlobDataPtr GetBlob() const = 0;
};

interface IVertexShader : public IShader 
{	
};

interface IPixelShader : public IShader 
{
};

interface IProgram : public IResource 
{
	virtual IVertexShaderPtr GetVertex() const = 0;
	virtual IPixelShaderPtr GetPixel() const = 0;
};

/********** HardwareBuffer **********/
enum HardwareBufferType {
	kHWBufferConstant,
	kHWBufferVertex,
	kHWBufferIndex
};
interface IHardwareBuffer : public IResource  
{
	virtual HardwareBufferType GetType() const = 0;
	virtual unsigned int GetBufferSize() const = 0;
};

interface IVertexBuffer : public IHardwareBuffer 
{
	virtual unsigned int GetStride() const = 0;
	virtual unsigned int GetOffset() const = 0;
};

interface IIndexBuffer : public IHardwareBuffer 
{
	virtual int GetWidth() const = 0;
	virtual ResourceFormat GetFormat() const = 0;
	int GetCount() const { return GetBufferSize() / GetWidth(); }
};

interface IContantBuffer : public IHardwareBuffer 
{
	virtual ConstBufferDeclPtr GetDecl() const = 0;
};

/********** Texture **********/
#define E_TEXTURE_MAIN 0
#define E_TEXTURE_DEPTH_MAP 8
#define E_TEXTURE_ENV 9

interface ITexture : public IResource  
{
	virtual bool HasSRV() const = 0;
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual ResourceFormat GetFormat() const = 0;
	virtual int GetMipmapCount() const = 0;
	virtual int GetFaceCount() const = 0;
	virtual bool IsAutoGenMipmap() const = 0;
};

interface IRenderTexture : public IResource   
{
	virtual ITexturePtr GetColorTexture() const = 0;
};

interface ISamplerState : public IResource 
{
};

}