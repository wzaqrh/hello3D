#pragma once
#include "core/base/tpl/vector.h"
#include "core/rendersys/base/hardware_memory_usage.h"
#include "core/rendersys/base/res_format.h"
#include "core/resource/resource.h"

namespace mir {

enum HardwareBufferType 
{
	kHWBufferConstant,
	kHWBufferVertex,
	kHWBufferIndex
};

struct CbDeclElement 
{
	enum class Type
	{
		Bool,
		Int,
		Int2,
		Int3,
		Int4,
		Float,
		Float2,
		Float3,
		Float4,
		Matrix,
		Max
	};
	template <typename T> struct NativeToType {};
	template <> struct NativeToType<bool> : public std::integral_constant<Type, Type::Bool> {};
	template <> struct NativeToType<int> : public std::integral_constant<Type, Type::Int> {};
	template <> struct NativeToType<Eigen::Vector2i> : public std::integral_constant<Type, Type::Int2> {};
	template <> struct NativeToType<Eigen::Vector3i> : public std::integral_constant<Type, Type::Int3> {};
	template <> struct NativeToType<Eigen::Vector4i> : public std::integral_constant<Type, Type::Int4> {};
	template <> struct NativeToType<float> : public std::integral_constant<Type, Type::Float> {};
	template <> struct NativeToType<Eigen::Vector2f> : public std::integral_constant<Type, Type::Float2> {};
	template <> struct NativeToType<Eigen::Vector3f> : public std::integral_constant<Type, Type::Float3> {};
	template <> struct NativeToType<Eigen::Vector4f> : public std::integral_constant<Type, Type::Float4> {};
	template <> struct NativeToType<Eigen::Matrix4f> : public std::integral_constant<Type, Type::Matrix> {};

	TemplateT static constexpr Type DetectType(const T& value) { return NativeToType<T>::value; }
	TemplateTArray static constexpr Type DetectType(const TArray& value) { return NativeToType<T>::value; }
	//TemplateT constexpr CbElementType DetectType(const std::vector<T>& value) { return NativeToType<T>::value; }
public:
	static size_t GetByteWidth(Type type);
	size_t GetByteWidth() const;

	bool IsValid() const { return !Name.empty(); }
	const std::string& GetName() const { return Name; }
public:
	std::string Name;
	Type Type1;
	size_t Size;
	size_t Count;
	size_t Offset;
};
template <> struct tpl::has_function_valid<CbDeclElement> : public std::true_type {};
template <> struct tpl::has_function_name<CbDeclElement> : public std::true_type {};

struct ConstBufferDecl : public tpl::Vector<CbDeclElement> { size_t BufferSize = 0; };

interface IVertexArray : public IResource {

};

interface IHardwareBuffer : public IResource
{
	virtual IVertexArrayPtr GetVAO() const { return nullptr; }
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