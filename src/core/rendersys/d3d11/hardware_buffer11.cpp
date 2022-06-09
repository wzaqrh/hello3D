#include "core/rendersys/d3d11/hardware_buffer11.h"
#include "core/base/d3d.h"

namespace mir {

void HardwareBuffer::Init(ComPtr<ID3D11Buffer>&& buffer, size_t bufferSize, HWMemoryUsage usage) {
	Buffer = std::move(buffer);
	BufferSize = bufferSize;
	Usage = usage;
}

/********** VertexBuffer11 **********/
void VertexBuffer11::Init(ComPtr<ID3D11Buffer>&& buffer, int bufferSize, HWMemoryUsage usage, int stride, int offset) {
	hd.Init(std::move(buffer), bufferSize, usage);
	Stride = stride;
	Offset = offset;
}

/********** IndexBuffer11 **********/
void IndexBuffer11::Init(ComPtr<ID3D11Buffer>&& buffer, int bufferSize, ResourceFormat format, HWMemoryUsage usage) {
	hd.Init(std::move(buffer), bufferSize, usage);
	Format = format;
}

int IndexBuffer11::GetWidth() const
{
	return d3d::BytePerPixel(static_cast<DXGI_FORMAT>(Format));
}

/********** ContantBuffer11 **********/
void ContantBuffer11::Init(ComPtr<ID3D11Buffer>&& buffer, ConstBufferDeclPtr decl, HWMemoryUsage usage)
{
	hd.Init(std::move(buffer), decl->BufferSize, usage);
	mDecl = decl;
}

}