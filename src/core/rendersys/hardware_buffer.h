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

enum CbElementType {
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
	kCBElementMax
};
template <typename T> struct DataType2CbElementType {};
template <> struct DataType2CbElementType<bool> : public std::integral_constant<CbElementType, kCBElementBool> {};
template <> struct DataType2CbElementType<int> : public std::integral_constant<CbElementType, kCBElementInt> {};
template <> struct DataType2CbElementType<Eigen::Vector2i> : public std::integral_constant<CbElementType, kCBElementInt2> {};
template <> struct DataType2CbElementType<Eigen::Vector3i> : public std::integral_constant<CbElementType, kCBElementInt3> {};
template <> struct DataType2CbElementType<Eigen::Vector4i> : public std::integral_constant<CbElementType, kCBElementInt4> {};
template <> struct DataType2CbElementType<float> : public std::integral_constant<CbElementType, kCBElementFloat> {};
template <> struct DataType2CbElementType<Eigen::Vector2f> : public std::integral_constant<CbElementType, kCBElementFloat2> {};
template <> struct DataType2CbElementType<Eigen::Vector3f> : public std::integral_constant<CbElementType, kCBElementFloat3> {};
template <> struct DataType2CbElementType<Eigen::Vector4f> : public std::integral_constant<CbElementType, kCBElementFloat4> {};
template <> struct DataType2CbElementType<Eigen::Matrix4f> : public std::integral_constant<CbElementType, kCBElementMatrix> {};

TemplateT constexpr CbElementType DetectCbElementType(const T& value) { return DataType2CbElementType<T>::value; }
TemplateTArray constexpr CbElementType DetectCbElementType(const TArray& value) { return DataType2CbElementType<T>::value; }
//TemplateT constexpr CbElementType DetectCbElementType(const std::vector<T>& value) { return DataType2CbElementType<T>::value; }

size_t GetCbElementTypeByteWidth(CbElementType type);

struct CbDeclElement {
	bool IsValid() const { return !Name.empty(); }
	const std::string& GetName() const { return Name; }
public:
	std::string Name;
	CbElementType Type;
	size_t Size;
	size_t Count;
	size_t Offset;
};
template <> struct tpl::has_function_valid<CbDeclElement> : public std::true_type {};
template <> struct tpl::has_function_name<CbDeclElement> : public std::true_type {};

struct ConstBufferDecl : public tpl::Vector<CbDeclElement> {
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