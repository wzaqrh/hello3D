#pragma once
#include <windows.h>
#include <d3d11.h>
#include "core/rendersys/hardware_buffer.h"

namespace mir {

class HardwareBuffer
{
public:
	HardwareBuffer(ID3D11Buffer* buffer, size_t bufferSize, HWMemoryUsage usage)
		:Buffer(buffer), BufferSize(bufferSize), Usage(usage) {}
	HardwareBuffer() :HardwareBuffer(nullptr, 0, kHWUsageDefault) {}
	void Init(ID3D11Buffer* buffer, size_t bufferSize, HWMemoryUsage usage) {
		Buffer = buffer, BufferSize = bufferSize, Usage = usage;
	}
public:
	ID3D11Buffer* Buffer;
	size_t BufferSize;
	HWMemoryUsage Usage;
};

class VertexBuffer11 : public ImplementResource<IVertexBuffer>
{
public:
	VertexBuffer11() :Stride(0), Offset(0) {}
	void Init(ID3D11Buffer* buffer, int bufferSize, HWMemoryUsage usage, int stride, int offset) {
		hd.Init(buffer, bufferSize, usage);
		Stride = stride;
		Offset = offset;
	}
	int GetCount() const {
		return hd.BufferSize / Stride;
	}
public:
	HWMemoryUsage GetUsage() const override { return hd.Usage; }
	int GetBufferSize() const override { return hd.BufferSize; }
	HardwareBufferType GetType() const override { return kHWBufferVertex; }

	int GetStride() const override { return Stride; }
	int GetOffset() const override { return Offset; }

	ID3D11Buffer*& GetBuffer11() { return hd.Buffer; }
public:
	unsigned int Stride, Offset;
	HardwareBuffer hd;
};

class IndexBuffer11 : public ImplementResource<IIndexBuffer>
{
public:
	IndexBuffer11() :Format(kFormatUnknown) {}
	void Init(ID3D11Buffer* buffer, int bufferSize, ResourceFormat format, HWMemoryUsage usage) {
		hd.Init(buffer, bufferSize, usage);
		Format = format;
	}
public:
	HWMemoryUsage GetUsage() const override { return hd.Usage; }
	int GetBufferSize() const override { return hd.BufferSize; }
	HardwareBufferType GetType() const override { return kHWBufferIndex; }

	int GetWidth() const override;
	ResourceFormat GetFormat() const override { return Format; }

	ID3D11Buffer*& GetBuffer11() { return hd.Buffer; }
public:
	ResourceFormat Format;
	HardwareBuffer hd;
};

class ContantBuffer11 : public ImplementResource<IContantBuffer>
{
public:
	ContantBuffer11() {}
	void Init(ID3D11Buffer* buffer, ConstBufferDeclPtr decl, HWMemoryUsage usage);
public:
	HWMemoryUsage GetUsage() const override { return hd.Usage; }
	int GetBufferSize() const override { return hd.BufferSize; }
	HardwareBufferType GetType() const override { return kHWBufferConstant; }

	ConstBufferDeclPtr GetDecl() const override { return mDecl; }

	ID3D11Buffer*& GetBuffer11() { return hd.Buffer; }
public:
	ConstBufferDeclPtr mDecl;
	HardwareBuffer hd;
};

}