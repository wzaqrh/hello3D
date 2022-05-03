#pragma once
#include <boost/noncopyable.hpp>
#include "core/predeclare.h"
#include "core/base/data.h"
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
	TemplateT const T& GetProperty(const std::string& propertyName) const {
		auto element = mDecl[propertyName]; 
		BOOST_ASSERT(element && DetectCbElementType(T()) == element->Type);
		return mData.As<T, 1>(element->Offset);
	}
	template<> const BOOL& GetProperty<BOOL>(const std::string& propertyName) const {
		auto element = mDecl[propertyName];
		BOOST_ASSERT(element && DetectCbElementType(bool()) == element->Type);
		return mData.As<BOOL, 1>(element->Offset);
	}
	TemplateT T& GetProperty(const std::string& propertyName) {
		mDataDirty = true;
		return const_cast<T&>(const_cast<const UniformParameters*>(this)->GetProperty<T>(propertyName));
	}
	TemplateT T& operator[](const std::string& propertyName) { return GetProperty<T>(propertyName); }
	TemplateT const T& operator[](const std::string& propertyName) const { return GetProperty<T>(propertyName); }
	void SetProperty(const std::string& propertyName, const Data& data) {
		auto element = mDecl[propertyName];
		BOOST_ASSERT((element && !mData.Overflow<char, 1>(element->Offset)));
		memcpy(&mData.As<char, 1>(element->Offset), data.Bytes, data.Size);
	}
	bool SetPropertyByString(const std::string& propertyName, std::string strDefault);

	IContantBufferPtr CreateConstBuffer(Launch launchMode, ResourceManager& resMng, HWMemoryUsage usage) const;
	void WriteToConstBuffer(RenderSystem& renderSys, IContantBufferPtr cbuffer) const;
	void SetDataDirty(bool dirty) { mDataDirty = dirty; }
public:
	bool IsValid() const { return !mData.IsEmpty(); }
	const std::string& GetName() const { return mShortName; }
	CBufferShareMode GetShareMode() const { return mShareMode; }
	size_t GetSlot() const { return mSlot; }
	bool IsReadOnly() const { return mIsReadOnly; }
	bool IsDataDirty() const { return mDataDirty; }
private:
	ConstBufferDecl mDecl;
	std::string mShortName;
	CBufferShareMode mShareMode = kCbShareNone;
	size_t mSlot = 0;
	bool mIsReadOnly = false;
private:
	tpl::Binary<float> mData;
	mutable bool mDataDirty = false;
};

class GpuParameters
{
	friend class MaterialFactory;
	struct Element {
		const std::string& GetName() const { return Parameters->GetName(); }
		bool IsValid() const { return CBuffer != nullptr; }
		operator bool() const { return IsValid(); }
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
		mElements.AddOrSet(Element(cbuffer, parameter));
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
			if (iter && (result = (*iter.Parameters).FindProperty(propertyName)) >= 0) {
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
	template<typename T> const T& GetProperty(const std::string& propertyName) const {
		for (auto& iter : mElements)
			if (iter && (*iter.Parameters).HasProperty(propertyName))
				return (*iter.Parameters).GetProperty<T>(propertyName);
		BOOST_ASSERT(false);
	}
	template<typename T> T& GetProperty(const std::string& propertyName) {
		return const_cast<T&>(const_cast<const GpuParameters*>(this)->GetProperty<T>(propertyName));
	}
	template<typename T> T& operator[](const std::string& propertyName) { return GetProperty(propertyName); }
	template<typename T> const T& operator[](const std::string& propertyName) const { return GetProperty(propertyName); }
	void SetProperty(const std::string& propertyName, const Data& data) {
		for (auto& iter : mElements) {
			if (iter && (*iter.Parameters).HasProperty(propertyName)) {
				(*iter.Parameters).SetProperty(propertyName, data);
				break;
			}
		}
	}
	bool SetPropertyByString(const std::string& propertyName, std::string strDefault) {
		for (auto& iter : mElements) {
			if (iter && (*iter.Parameters).SetPropertyByString(propertyName, strDefault)) {
				return true;
			}
		}
		return false;
	}
	
	void WriteToElementCb(RenderSystem& renderSys, const std::string& cbName, Data data);
	void FlushToGpu(RenderSystem& renderSys);
public:
	std::vector<IContantBufferPtr> GetConstBuffers() const;
	const_iterator begin() const { return mElements.begin(); }
	const_iterator end() const { return mElements.end(); }
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