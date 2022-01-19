#pragma once
#include <boost/noncopyable.hpp>
#include "core/predeclare.h"
#include "core/base/tpl/vector.h"
#include "core/base/tpl/binary.h"
#include "core/rendersys/hardware_buffer.h"

namespace mir {

class UniformParameters 
{
	friend class UniformParametersBuilder;
public:
	const ConstBufferDecl& GetDecl() const { return mDecl; }

	int FindProperty(const std::string& name) const {
		return mDecl.IndexByName(name);
	}
	bool HasProperty(const std::string& name) const {
		return FindProperty(name) >= 0;
	}
	template<typename T> T& GetProperty(const std::string& name) {
		auto element = mDecl[name];
		BOOST_ASSERT(element && DataType2CbElementType<T>::value == element->Type);
		return mData.As<T, 1>(element->Offset);
	}
	template<> BOOL& GetProperty<BOOL>(const std::string& name) {
		auto element = mDecl[name];
		BOOST_ASSERT(element && DataType2CbElementType<bool>::value == element->Type);
		return mData.As<BOOL, 1>(element->Offset);
	}
	template<typename T> const T& GetProperty(const std::string& name) const {
		auto element = mDecl[name];
		BOOST_ASSERT(element && DataType2CbElementType<T>::value == element->Type);
		return mData.As<T, 1>(element->Offset);
	}
	template<> const BOOL& GetProperty<BOOL>(const std::string& name) const {
		auto element = mDecl[name];
		BOOST_ASSERT(element && DataType2CbElementType<bool>::value == element->Type);
		return mData.As<BOOL, 1>(element->Offset);
	}
	template<typename T> T& operator[](const std::string& name) {
		return GetProperty(name);
	}
	template<typename T> const T& operator[](const std::string& name) const {
		return GetProperty(name);
	}
	bool SetPropertyByString(const std::string& name, std::string strDefault);
	
	IContantBufferPtr CreateConstBuffer(Launch launchMode, ResourceManager& resMng, HWMemoryUsage usage) const;
	void WriteToConstBuffer(RenderSystem& renderSys, IContantBufferPtr cbuffer) const;
public:
	bool IsValid() const { return !mData.IsEmpty(); }
	const std::string& GetName() const { return mShortName; }
	size_t GetSlot() const { return mSlot; }
	bool IsUnique() const { return mIsUnique; }
private:
	ConstBufferDecl mDecl;
	tpl::Binary<float> mData;
	std::string mShortName;
	size_t mSlot;
	bool mIsUnique;
};
using UniformParametersPtr = std::shared_ptr<UniformParameters>;
template <> struct tpl::has_function_valid<UniformParameters> : public std::true_type {};
template <> struct tpl::has_function_name<UniformParameters> : public std::true_type {};

class GpuUniformsParameters : public tpl::Vector<UniformParametersPtr>
{
	friend class UniformParametersBuilder;
public:
	std::pair<int, int> FindProperty(const std::string& name) const {
		std::pair<int, int> result;
		result.first = -1;
		result.second = -1;
		for (const auto& iter : *this) {
			if ((result.second = (*iter).HasProperty(name)) > 0) {
				result.first = (*iter).GetSlot();
				break;
			}
		}
		return std::move(result);
	}
	bool HasProperty(const std::string& name) { 
		return FindProperty(name).first >= 0;
	}
	template<typename T> T& GetProperty(const std::string& name) {
		for (const auto& iter : *this)
			if ((*iter).HasProperty(name))
				return (*iter).GetProperty(name);
		BOOST_ASSERT(false);
		return At(0).GetProperty(name);
	}
	template<typename T> const T& GetProperty(const std::string& name) const {
		for (const auto& iter : *this)
			if ((*iter).HasProperty(name))
				return (*iter).GetProperty(name);
		BOOST_ASSERT(false);
		return At(0).GetProperty(name);
	}
	template<typename T> T& operator[](const std::string& name) {
		return GetProperty(name);
	}
	template<typename T> const T& operator[](const std::string& name) const {
		return GetProperty(name);
	}
	bool SetPropertyByString(const std::string& name, std::string strDefault) {
		for (auto& iter : *this) {
			if ((*iter).SetPropertyByString(name, strDefault))
				return true;
		}
		return false;
	}
};
using GpuUniformsParametersPtr = std::shared_ptr<GpuUniformsParameters>;

class UniformParametersBuilder
{
public:
	UniformParametersBuilder(UniformParameters& result) :mResult(result) {}
	void AddParameter(const std::string& name, CbElementType type, size_t size, size_t count, size_t offset, const std::string& defValue);
	std::string& ShortName() { return mResult.mShortName; }
	size_t& Slot() { return mResult.mSlot; }
	bool& IsUnique() { return mResult.mIsUnique; }
	UniformParameters& Build();
private:
	int mCurrentByteOffset = 0;
	UniformParameters& mResult;
};

}