#pragma once
#include "core/base/stl.h"

namespace mir {

#define TemplateArgs template <typename... T>
#define TemplateT template <typename T>

class EmptyContainer {};

template <class Element, class Parent = EmptyContainer> class VectorAdapter : public Parent {
	using value_type = Element;
	using reference = Element&;
	using const_reference = const Element&;
	using iterator = typename std::vector<value_type>::iterator ;
	using const_iterator = typename std::vector<value_type>::const_iterator;
public:
	void Clear() {
		mElements.clear();
	}
	void Swap(VectorAdapter& other) {
		mElements.swap(other.mElements);
	}
	void Resize(size_t size) {
		mElements.resize(size);
	}
	reference& Emplace() {
		mElements.resize(mElements.size() + 1);
		return mElements.back();
	}
	TemplateT void Add(T&& value) {
		mElements.push_back(std::forward<T>(value));
	}
	TemplateT void AddOrSet(T&& value, int slot = -1) {
		if (slot >= 0) {
			if (mElements.size() < slot + 1)
				mElements.resize(slot + 1);
			mElements[slot] = std::forward<T>(value);
		}
		else {
			mElements.push_back(std::forward<T>(value));
		}
	}
public:
	bool IsEmpty() const { return mElements.empty(); }
	size_t Count() const { return mElements.size(); }
	
	reference At(size_t pos) { return mElements[pos]; }
	const_reference At(size_t pos) const { return mElements[pos]; }
	reference operator[](size_t pos) { return mElements[pos]; }
	const_reference operator[](size_t pos) const { return mElements[pos]; }

	iterator begin() { return mElements.begin(); }
	iterator end() { return mElements.end(); }
	const_iterator begin() const { return mElements.begin(); }
	const_iterator end() const { return mElements.end(); }

	std::pair<const_iterator, const_iterator> Range(size_t first, size_t last = -1) const {
		return std::make_pair<const_iterator, const_iterator>(mElements.begin() + first, last < mElements.size() ? mElements.begin() + last : mElements.end());
	}
protected:
	std::vector<value_type> mElements;
};
template <class Element, class Parent = EmptyContainer> class VectorAdapterEx : public VectorAdapter<Element, Parent> {
	typedef std::function<bool(const Element& v)> UnaryFuncCheckValid;
public:
	VectorAdapterEx(UnaryFuncCheckValid unaryChkValid) :mUnaryCheckValid(unaryChkValid) {}

	void MergeOverride(const VectorAdapterEx& other) {
		if (mElements.size() < other.mElements.size())
			mElements.resize(other.mElements.size());
		for (size_t i = 0; i < other.mElements.size(); ++i) {
			if (mUnaryCheckValid(other.mElements[i]))
				mElements[i] = other.mElements[i];
		}
	}
	void MergeNoOverride(const VectorAdapterEx& other) {
		if (mElements.size() < other.mElements.size())
			mElements.resize(other.mElements.size());
		for (size_t i = 0; i < other.mElements.size(); ++i) {
			if (!mUnaryCheckValid(mElements[i]))
				mElements[i] = other.mElements[i];
		}
	}
protected:
	UnaryFuncCheckValid mUnaryCheckValid;
};

}