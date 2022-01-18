#pragma once
#include "core/base/base_type.h"
#include "core/base/tpl/vector.h"
#include "core/resource/resource.h"

namespace mir {

enum HardwareBufferType {
	kHWBufferConstant,
	kHWBufferVertex,
	kHWBufferIndex
};

enum ConstBufferElementType {
	kCBElementBool,
	kCBElementInt,
	kCBElementInt2,
	kCBElementInt3,
	kCBElementInt4,
	kCBElementFloat,
	kCBElementFloat2,
	kCBElementFloat3,
	kCBElementFloat4,
	kCBElementMatrix,
	kCBElementStruct,
	kCBElementMax
};
struct ConstBufferDeclElement {
	bool IsValid() const { return !Name.empty(); }
	const std::string& GetName() const { return Name; }
public:
	std::string Name;
	ConstBufferElementType Type;
	size_t Size;
	size_t Count;
	size_t Offset;
};
template <> struct tpl::has_function_valid_t<ConstBufferDeclElement> : public std::true_type {};
template <> struct tpl::has_function_name_t<ConstBufferDeclElement> : public std::true_type {};

struct ConstBufferDecl : public tpl::Vector<ConstBufferDeclElement> {
	size_t BufferSize = 0;
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