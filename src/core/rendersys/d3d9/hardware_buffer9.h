#pragma once
#include <windows.h>
#include <d3d9.h>
#include "core/rendersys/hardware_buffer.h"

namespace mir {

class IndexBuffer9 : public ImplementResource<IIndexBuffer>
{
public:
	IndexBuffer9() : Buffer(nullptr), BufferSize(0), Format(kFormatUnknown) {}
	void Init(IDirect3DIndexBuffer9* buffer, unsigned int bufferSize, ResourceFormat format) {
		Buffer = buffer;
		BufferSize = bufferSize;
		Format = format;
	}

	HWMemoryUsage GetUsage() const override { return kHWUsageDefault; }
	HardwareBufferType GetType() const override { return kHWBufferIndex; }
	int GetBufferSize() const override { return BufferSize; }

	int GetWidth() const override;
	ResourceFormat GetFormat() const override { return Format; }

	IDirect3DIndexBuffer9*& GetBuffer9() { return Buffer; }
public:
	IDirect3DIndexBuffer9* Buffer;
	unsigned int BufferSize;
	ResourceFormat Format;
};

class VertexBuffer9 : public ImplementResource<IVertexBuffer>
{
public:
	VertexBuffer9(IDirect3DVertexBuffer9* buffer, unsigned int bufferSize, unsigned int stride, unsigned int offset)
		: Buffer(buffer), BufferSize(bufferSize), Stride(stride), Offset(offset) {}
	VertexBuffer9() :VertexBuffer9(nullptr, 0, 0, 0) {}

	HWMemoryUsage GetUsage() const override { return kHWUsageDefault; }
	HardwareBufferType GetType() const override { return kHWBufferVertex; }
	int GetBufferSize() const override { return BufferSize; }

	int GetStride() const override { return Stride; }
	int GetOffset() const override { return Offset; }

	IDirect3DVertexBuffer9*& GetBuffer9() { return Buffer; }
public:
	IDirect3DVertexBuffer9* Buffer;
	unsigned int BufferSize;
	unsigned int Stride, Offset;
};

class ContantBuffer9 : public ImplementResource<IContantBuffer>
{
public:
	ContantBuffer9(ConstBufferDeclPtr decl);
	ContantBuffer9() :ContantBuffer9(nullptr) {}

	HWMemoryUsage GetUsage() const override { return kHWUsageDefault; }
	HardwareBufferType GetType() const override { return kHWBufferConstant; }
	int GetBufferSize() const override;

	ConstBufferDeclPtr GetDecl() const override { return mDecl; }
	char* GetBuffer9();
	void SetBuffer9(const void* data, int dataSize);
public:
	ConstBufferDeclPtr mDecl;
	std::vector<char> mBuffer9;
};

}