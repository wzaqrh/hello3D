#pragma once
#include "core/rendersys/base_type.h"
#include "core/resource/resource.h"

namespace mir {

enum HardwareBufferType {
	kHWBufferConstant,
	kHWBufferVertex,
	kHWBufferIndex
};

interface IHardwareBuffer : public IResource
{
	virtual HardwareBufferType GetType() const = 0;
	virtual int GetBufferSize() const = 0;
	virtual HWMemoryUsage GetUsage() const = 0;
};

interface IVertexBuffer : public IHardwareBuffer
{
	virtual int GetStride() const = 0;
	virtual int GetOffset() const = 0;

	int GetCount() const { return GetBufferSize() / GetStride(); }
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

}