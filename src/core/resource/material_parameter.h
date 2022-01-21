#pragma once
#include <boost/noncopyable.hpp>
#include "core/predeclare.h"
#include "core/base/tpl/vector.h"
#include "core/base/tpl/binary.h"
#include "core/rendersys/hardware_buffer.h"

namespace mir {
namespace res {

enum CBufferShareMode {
	kCbShareNone = 0,
	kCbSharePerMaterial = 1,
	kCbSharePerFrame = 2,
	kCbShareMax
};

class UniformParameters
{
	friend class UniformParametersBuilder;
public:
	const ConstBufferDecl& GetDecl() const { return mDecl; }

	int FindProperty(const std::string& propertyName) const {
		return mDecl.IndexByName(propertyName);
	}
	bool HasProperty(const std::string& propertyName) const {
		return FindProperty(propertyName) >= 0;
	}
	template<typename T> T& GetProperty(const std::string& propertyName) {
		auto element = mDecl[propertyName];
		BOOST_ASSERT(element && DataType2CbElementType<T>::value == element->Type);
		return mData.As<T, 1>(element->Offset);
	}
	template<> BOOL& GetProperty<BOOL>(const std::string& propertyName) {
		auto element = mDecl[propertyName];
		BOOST_ASSERT(element && DataType2CbElementType<bool>::value == element->Type);
		return mData.As<BOOL, 1>(element->Offset);
	}
	template<typename T> const T& GetProperty(const std::string& propertyName) const {
		auto element = mDecl[propertyName];
		BOOST_ASSERT(element && DataType2CbElementType<T>::value == element->Type);
		return mData.As<T, 1>(element->Offset);
	}
	template<> const BOOL& GetProperty<BOOL>(const std::string& propertyName) const {
		auto element = mDecl[propertyName];
		BOOST_ASSERT(element && DataType2CbElementType<bool>::value == element->Type);
		return mData.As<BOOL, 1>(element->Offset);
	}
	template<typename T> T& operator[](const std::string& propertyName) {
		return GetProperty(propertyName);
	}
	template<typename T> const T& operator[](const std::string& propertyName) const {
		return GetProperty(propertyName);
	}
	bool SetPropertyByString(const std::string& propertyName, std::string strDefault);

	IContantBufferPtr CreateConstBuffer(Launch launchMode, ResourceManager& resMng, HWMemoryUsage usage) const;
	void WriteToConstBuffer(RenderSystem& renderSys, IContantBufferPtr cbuffer) const;
public:
	bool IsValid() const { return !mData.IsEmpty(); }
	const std::string& GetName() const { return mShortName; }
	size_t GetSlot() const { return mSlot; }
	CBufferShareMode GetShareMode() const { return mShareMode; }
	bool IsReadOnly() const { return mIsReadOnly; }
private:
	ConstBufferDecl mDecl;
	tpl::Binary<float> mData;
	std::string mShortName;
	size_t mSlot = 0;
	CBufferShareMode mShareMode = kCbShareNone;
	bool mIsReadOnly = false;
};

class GpuParameters
{
	friend class MaterialFactory;
	struct Element {
		bool IsValid() const { return CBuffer != nullptr; }
		int GetSlot() const { return Parameters->GetSlot(); }
		bool IsShared() const { return Parameters->GetShareMode() != kCbShareNone; }
		CBufferShareMode GetShareMode() const { return Parameters->GetShareMode(); }
		Element() {}
		Element(IContantBufferPtr cbuffer, UniformParametersPtr parameters) :CBuffer(cbuffer), Parameters(parameters) {}
		Element Clone(Launch launchMode, ResourceManager& resMng) const;
	public:
		IContantBufferPtr CBuffer;
		UniformParametersPtr Parameters;
	};
	using const_iterator = tpl::Vector<Element>::const_iterator;
public:
	void AddElement(const Element& element) {
		mElements.AddOrSet(element, element.GetSlot());
	}
	void AddElement(IContantBufferPtr cbuffer, UniformParametersPtr parameter) {
		mElements.AddOrSet(Element{ cbuffer, parameter });
	}
	void Merge(const GpuParameters& other) {
		for (const auto& element : other) {
			if (element.IsValid()) {
				BOOST_ASSERT(element.GetSlot() >= mElements.Count() || !mElements[element.GetSlot()].IsValid());
				AddElement(element);
			}
		}
	}
	GpuParametersPtr Clone(Launch launchMode, ResourceManager& resMng) const;
public:
	int FindProperty(const std::string& propertyName) const {
		int result = -1;
		for (const auto& iter : mElements) {
			if ((result = (*iter.Parameters).FindProperty(propertyName)) >= 0) {
				BOOST_ASSERT(result < 0x10000);
				result |= (*iter.Parameters).GetSlot() * 0x10000;
				break;
			}
		}
		return std::move(result);
	}
	bool HasProperty(const std::string& propertyName) {
		return FindProperty(propertyName) >= 0;
	}
	template<typename T> T& GetProperty(const std::string& propertyName) {
		for (const auto& iter : mElements)
			if ((*iter.Parameters).HasProperty(propertyName))
				return (*iter.Parameters).GetProperty(propertyName);
		BOOST_ASSERT(false);
		return At(0).GetProperty(propertyName);
	}
	template<typename T> const T& GetProperty(const std::string& propertyName) const {
		for (const auto& iter : mElements)
			if ((*iter.Parameters).HasProperty(propertyName))
				return (*iter.Parameters).GetProperty(propertyName);
		BOOST_ASSERT(false);
		return At(0).GetProperty(propertyName);
	}
	template<typename T> T& operator[](const std::string& propertyName) {
		return GetProperty(propertyName);
	}
	template<typename T> const T& operator[](const std::string& propertyName) const {
		return GetProperty(propertyName);
	}
	bool SetPropertyByString(const std::string& propertyName, std::string strDefault) {
		for (auto& iter : mElements) {
			if ((*iter.Parameters).SetPropertyByString(propertyName, strDefault))
				return true;
		}
		return false;
	}

	const_iterator begin() const { return mElements.begin(); }
	const_iterator end() const { return mElements.end(); }

	std::vector<IContantBufferPtr> GetConstBuffers() const;
private:
	tpl::Vector<Element> mElements;
};

class UniformParametersBuilder
{
public:
	UniformParametersBuilder(UniformParameters& result) :mResult(result) {}
	void AddParameter(const std::string& propertyName, CbElementType type, size_t size, size_t count, size_t offset, const std::string& defValue);
	std::string& ShortName() { return mResult.mShortName; }
	size_t& Slot() { return mResult.mSlot; }
	CBufferShareMode& ShareMode() { return mResult.mShareMode; }
	bool& IsReadOnly() { return mResult.mIsReadOnly; }
	UniformParameters& Build();
private:
	int mCurrentByteOffset = 0;
	UniformParameters& mResult;
};

}
template <> struct tpl::has_function_valid<res::UniformParameters> : public std::true_type {};
template <> struct tpl::has_function_name<res::UniformParameters> : public std::true_type {};
}