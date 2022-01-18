#pragma once
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include "core/base/stl.h"

namespace mir {
namespace tpl {

template<class Element> class Binary {
public:
	using value_type = Element;
	using reference = Element&;
	using const_reference = const Element&;
	using iterator = typename std::vector<value_type>::iterator;
	using const_iterator = typename std::vector<value_type>::const_iterator;
	using pointer = Element*;
	using const_pointer = const Element*;
public:
	Binary(){}
	template <typename T> Binary(std::vector<T>&& elements) 
		: mElements(std::forward<std::vector<T>>(elements)) {}

	void SetCount(size_t count) {
		mElements.resize(count);
	}
	void SetByteSize(size_t size) {
		BOOST_ASSERT(size % sizeof(value_type) == 0);
		mElements.resize(size / sizeof(value_type));
	}

	template<typename T> bool Overflow(size_t position) const {
		return position + sizeof(T) > mElements.size() * sizeof(value_type);
	}

	template<typename T> T& As(size_t position) {
		BOOST_ASSERT(!Overflow<T>(position));
		return *(T*)&mElements[position];
	}
	template<typename T> const T& As(size_t position) const {
		BOOST_ASSERT(!Overflow<T>(position));
		return *(T*)&mElements[position];
	}

	void Emplaces(size_t count) {
		mElements.resize(mElements.size() + count);
	}
	template<typename T> T& Emplace() {
		BOOST_ASSERT(sizeof(T) % sizeof(value_type) == 0);
		size_t position = mElements.size();
		mElements.resize(position + sizeof(T) / sizeof(value_type));
		return *(T*)&mElements[position];
	}
	template<typename T> T& Add(const T& value) {
		Emplace<T>() = value;
	}
public:
	bool IsEmpty() const { return mElements.empty(); }
	size_t Count() const { return mElements.size(); }

	const_reference At(size_t pos) const { return mElements[pos]; }
	const_reference operator[](size_t pos) const { return mElements[pos]; }

	size_t ByteSize() const { return mElements.size() * sizeof(value_type); }
	const std::vector<value_type>& GetBytes() const { return mElements; }
	std::vector<value_type>&& MoveBytes() { return std::move(mElements); }
private:
	std::vector<value_type> mElements;
};

template<class Element> class BinaryWritter {
public:
	BinaryWritter(Binary<Element>& buffer) :mBuffer(buffer), mPosition(buffer.Count()){}

	void SetPosition(size_t position) {
		mPosition = position;
	}
	void MovePosition(int step) {
		mPosition += std::max<int>(step, -mPosition);
		mPosition = std::min<size_t>(mPosition, mElements.size());
	}
	void SetPositionBegin() {
		mPosition = 0;
	}
	void SetPositionEnd() {
		mPosition = mElements.size();
	}

	void WriteEmpties(size_t count) {
		mBuffer.Emplaces<Element>(count);
		mPosition += count;
	}
	template<typename T> void WriteElementsByParseString(const std::string& str, size_t elemCount) {
		std::vector<boost::iterator_range<std::string::iterator>> strArr;
		boost::split(strArr, str, boost::is_any_of(","));
		int i = 0;
		for (; i < strArr.size() && i < elemCount; ++i)
			mBuffer.Emplace<T>() = boost::lexical_cast<T>(strArr[i]);
		if (i < elemCount)
			mBuffer.Emplaces<T>(elemCount - i);
		mPosition += count;
	}
private:
	size_t mPosition;
	Binary<Element>& mBuffer;
};

}
}
