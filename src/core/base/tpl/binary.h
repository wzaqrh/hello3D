#pragma once
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
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
	static constexpr size_t value_size = sizeof(value_type);
public:
	Binary(){}
	template <typename T> Binary(std::vector<T>&& elements) 
		: mElements(std::forward<std::vector<T>>(elements)) {}

	void SetCount(size_t count) {
		mElements.resize(count);
	}
	void SetByteSize(size_t size) {
		BOOST_ASSERT(size % value_size == 0);
		mElements.resize(size / value_size);
	}

	template<typename T, size_t BytePerIndex = value_size> bool Overflow(size_t position) const {
		return position * BytePerIndex + sizeof(T) > mElements.size() * value_size;
	}
	template<typename T, size_t BytePerIndex = value_size> bool Overflow(size_t position, size_t count) const {
		return position * BytePerIndex + sizeof(T) * count > mElements.size() * value_size;
	}

	template<typename T, size_t BytePerIndex = value_size> T& As(size_t position) {
		BOOST_ASSERT((!Overflow<T, BytePerIndex>(position)));
		BOOST_ASSERT(position * BytePerIndex % value_size == 0);
		return *(T*)&mElements[position * BytePerIndex / value_size];
	}
	template<typename T, size_t BytePerIndex = value_size> const T& As(size_t position) const {
		BOOST_ASSERT((!Overflow<T, BytePerIndex>(position)));
		BOOST_ASSERT(position * BytePerIndex % value_size == 0);
		return *(T*)&mElements[position * BytePerIndex / value_size];
	}

	template<typename T, size_t BytePerIndex = value_size> void SetByParseString(size_t position, size_t elemCount, std::string str) {
		BOOST_ASSERT((!Overflow<T, BytePerIndex>(position, elemCount)));
		BOOST_ASSERT(position * BytePerIndex % value_size == 0);
		BOOST_ASSERT(value_size % BytePerIndex == 0);
		size_t i = 0;
		if (!str.empty()) {
			std::vector<boost::iterator_range<std::string::iterator>> strArr;
			boost::split(strArr, str, boost::is_any_of(","));
			for (; i < strArr.size() && i < elemCount; ++i)
				As<T, BytePerIndex>(position + i * value_size / BytePerIndex) = boost::lexical_cast<T>(strArr[i]);
		}
		for (; i < elemCount; ++i)
			As<T, BytePerIndex>(position + i * value_size / BytePerIndex) = T();
	}

	template<typename T> void Emplaces(size_t count) {
		BOOST_ASSERT(sizeof(T) % value_size == 0);
		mElements.resize(mElements.size() + count * sizeof(T) / value_size);
	}
	template<typename T> T& Emplace() {
		BOOST_ASSERT(sizeof(T) % value_size == 0);
		size_t position = mElements.size();
		mElements.resize(position + sizeof(T) / value_size);
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

	size_t ByteSize() const { return mElements.size() * value_size; }
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

	template<typename T> void WriteEmpties(size_t count) {
		mBuffer.Emplaces<T>(count);
		mPosition += count;
	}
	template<typename T> void WriteElementsByParseString(std::string str, size_t elemCount) {
		std::vector<boost::iterator_range<std::string::iterator>> strArr;
		boost::split(strArr, str, boost::is_any_of(","));
		int i = 0;
		for (; i < strArr.size() && i < elemCount; ++i)
			mBuffer.Emplace<T>() = boost::lexical_cast<T>(strArr[i]);
		if (i < elemCount)
			mBuffer.Emplaces<T>(elemCount - i);
		mPosition += elemCount;
	}
private:
	size_t mPosition;
	Binary<Element>& mBuffer;
};

}
}
