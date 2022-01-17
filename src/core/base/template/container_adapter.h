#pragma once
#include "core/base/stl.h"

namespace mir {

#define TemplateArgs template <typename... T>
#define TemplateT template <typename T>

class EmptyContainer {};

template <typename T> struct has_function_valid_t : public std::false_type {};
template <typename T> struct has_function_name_t : public std::false_type {};
template <typename T> struct VectorAdapterTraits {
	using has_function_valid = has_function_valid_t<T>;
	using has_function_name = has_function_name_t<T>;
};

template <class Element, class Parent = EmptyContainer> class VectorAdapter : public Parent {
public:
	using value_type = Element;
	using reference = Element&;
	using const_reference = const Element&;
	using iterator = typename std::vector<value_type>::iterator ;
	using const_iterator = typename std::vector<value_type>::const_iterator;
	using pointer = Element*;
	using const_pointer = const Element*;
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

	template<typename HasFunctionValid = std::false_type> struct MergeOverrideFunctor {
		void operator()(std::vector<value_type>& mElements, const std::vector<value_type>& otherElements) {}
	};
	template<> struct MergeOverrideFunctor<std::true_type> {
		void operator()(std::vector<value_type>& mElements, const std::vector<value_type>& otherElements) {
			if (mElements.size() < otherElements.size())
				mElements.resize(otherElements.size());
			for (size_t i = 0; i < otherElements.size(); ++i) {
				if (otherElements[i].IsValid())
					mElements[i] = otherElements[i];
			}
		}
	};
	void MergeOverride(const VectorAdapter& other) {
		MergeOverrideFunctor<typename VectorAdapterTraits<Element>::has_function_valid::type>()(mElements, other.mElements);
	}

	template<typename HasFunctionValid = std::false_type> struct MergeNoOverrideFunctor {
		void operator()(std::vector<value_type>& mElements, const std::vector<value_type>& otherElements) {}
	};
	template<> struct MergeNoOverrideFunctor<std::true_type> {
		void operator()(std::vector<value_type>& mElements, const std::vector<value_type>& otherElements) {
			if (mElements.size() < otherElements.size())
				mElements.resize(otherElements.size());
			for (size_t i = 0; i < otherElements.size(); ++i) {
				if (!mElements[i].IsValid())
					mElements[i] = otherElements[i];
			}
		}
	};
	void MergeNoOverride(const VectorAdapter& other) {
		MergeNoOverrideFunctor<typename VectorAdapterTraits<Element>::has_function_valid::type>()(mElements, other.mElements);
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
		return std::make_pair<const_iterator, const_iterator>(mElements.begin() + first, last < mElements.size() ? mElements.begin() + last : mElements.end());
	}

	template<typename HasFunctionName = std::false_type> struct IndexByNameFunctor {
		size_t operator()(const std::vector<value_type>& mElements, const std::string& name) { return (size_t)-1; }
	};
	template<> struct IndexByNameFunctor<std::true_type> {
		size_t operator()(const std::vector<value_type>& mElements, const std::string& name) {
			for (const auto& iter : mElements)
				if (iter.GetName() == name)
					return &iter - &mElements[0];
			return (size_t)-1;
		}
	};
	size_t IndexByName(const std::string& name) const {
		return IndexByNameFunctor<typename VectorAdapterTraits<Element>::has_function_name::type>()(mElements, name);
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