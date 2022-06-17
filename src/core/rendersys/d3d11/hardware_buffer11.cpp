#include "core/rendersys/d3d11/hardware_buffer11.h"

namespace mir {

void HardwareBuffer::Init(ComPtr<ID3D11Buffer>&& buffer, size_t bufferSize, HWMemoryUsage usage) {
	Buffer = std::move(buffer);
	BufferSize = bufferSize;
	Usage = usage;
}

/********** VertexBuffer11 **********/
void VertexBuffer11::Init(ComPtr<ID3D11Buffer>&& buffer, IVertexArrayPtr vao, int bufferSize, HWMemoryUsage usage, int stride, int offset) {
	hd.Init(std::move(buffer), bufferSize, usage);
	Vao = vao;
	Stride = stride;
	Offset = offset;
}

/********** IndexBuffer11 **********/
void IndexBuffer11::Init(ComPtr<ID3D11Buffer>&& buffer, IVertexArrayPtr vao, int bufferSize, ResourceFormat format, HWMemoryUsage usage) {
	hd.Init(std::move(buffer), bufferSize, usage);
	Vao = vao;
	Format = format;
}

int IndexBuffer11::GetWidth() const
{
	return BytePerPixel(Format);
}

/********** ContantBuffer11 **********/
void ContantBuffer11::Init(ComPtr<ID3D11Buffer>&& buffer, ConstBufferDeclPtr decl, HWMemoryUsage usage)
{
	hd.Init(std::move(buffer), decl->BufferSize, usage);
	mDecl = decl;
}

}