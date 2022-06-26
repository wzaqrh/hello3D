#pragma once
#include <windows.h>
#include <glad/glad.h>
#include <GL/GL.h>
#include "core/mir_config.h"
#include "core/base/data.h"
#include "core/rendersys/blob.h"

namespace mir {

class BlobDataOGL : public IBlobData {
public:
	BlobDataOGL();
	const char* GetBytes() const override;
	size_t GetSize() const override;
public:
	std::vector<char> mBinary;
	std::string mSource;
};

}