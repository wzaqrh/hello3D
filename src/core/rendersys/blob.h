#pragma once
#include <boost/noncopyable.hpp>
#include "core/rendersys/predeclare.h"

namespace mir {

#define interface struct

interface IBlobData : boost::noncopyable
{
	virtual ~IBlobData() {}
	virtual const char* GetBytes() const = 0;
	virtual size_t GetSize() const = 0;
};

interface BlobDataBytes : public IBlobData
{
public:
	template<typename T> BlobDataBytes(T&& buffer) :mBytes(std::forward<T>(buffer)) {}
	const char* GetBytes() const override { return mBytes.empty() ? nullptr : &mBytes[0]; }
	size_t GetSize() const override { return mBytes.size(); }
public:
	std::vector<char> mBytes;
};

}