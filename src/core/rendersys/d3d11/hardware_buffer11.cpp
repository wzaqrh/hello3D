#include "core/rendersys/d3d11/hardware_buffer11.h"
#include "core/base/d3d.h"

namespace mir {

/********** IndexBuffer11 **********/
int IndexBuffer11::GetWidth() const
{
	return d3d::BytePerPixel(static_cast<DXGI_FORMAT>(Format));
}

/********** ContantBuffer11 **********/
void ContantBuffer11::Init(ID3D11Buffer* buffer, ConstBufferDeclPtr decl, HWMemoryUsage usage)
{
	hd.Init(buffer, decl->BufferSize, usage);
	mDecl = decl;
}

}