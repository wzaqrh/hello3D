#pragma once
#include "core/base/stl.h"
#include "core/base/tpl/traits.h"

namespace mir {
namespace tpl {

class EmptyContainer {};

template<class Element, class Parent = EmptyContainer> class Vector : public Parent {
public:
	using value_type = Element;
	using reference = Element & ;
	using const_reference = const Element&;
	using iterator = typename std::vector<value_type>::iterator;
	using const_iterator = typename std::vector<value_type>::const_iterator;
	using pointer = Element * ;
	using const_pointer = const Element*;
public:
	void Clear() {
		mElements.clear();
	}
	void Swap(Vector& other) {
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

	template<bool Override> void Merge(const Vector& other) {
		if constexpr (has_function_valid<value_type>::value) {
			if (mElements.size() < other.mElements.size())
				mElements.resize(other.mElements.size());
			for (size_t i = 0; i < other.mElements.size(); ++i) {
				bool valid;
				if constexpr (Override) {
					call_memfun(valid = other.mElements[i], IsValid());
					if (valid) mElements[i] = other.mElements[i];
				}
				else {
					call_memfun(valid = mElements[i], IsValid());
					if (!valid) mElements[i] = other.mElements[i];
				}
			}
		}
	}
	void Adds(const Vector& other) {
		for (size_t i = 0; i < other.mElements.size(); ++i)
			Add(other.mElements[i]);
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

	const_reference First() const { return mElements[0]; }
	const_reference Last() const { return mElements.back(); }
	reference First() { return mElements[0]; }
	reference Last() { return mElements.back(); }

	std::pair<const_iterator, const_iterator> Range(size_t first, size_t last = -1) const {
		return std::make_pair(mElements.begin() + first, last < mElements.size() ? mElements.begin() + last : mElements.end());
	}

	size_t IndexByName(const std::string& name) const {
		if constexpr (has_function_name<value_type>::value) {
			for (const auto& iter : mElements) {
				if constexpr (is_shared_ptr<value_type>::value) {
					if (iter->GetName() == name)
						return &iter - &mElements[0];
				}
				else {
					if (iter.GetName() == name)
						return &iter - &mElements[0];
				}
			}
		}
		return (size_t)-1;
	}
	static constexpr size_t IndexNotFound() { return (size_t)-1; }
	const_pointer operator[](const std::string& name) const {
		size_t index = IndexByName(name);
		return index < mElements.size() ? &mElements[index] : nullptr;
	}
protected:
	std::vector<value_type> mElements;
};

}
}