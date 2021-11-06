#include "core/rendersys/base_type.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/const_buffer_decl.h"
#include "core/base/utility.h"

namespace mir {

/********** TConstBufferDeclElement **********/
ConstBufferDeclElement::ConstBufferDeclElement(const char* __name, ConstBufferElementType __type, size_t __size, size_t __count, size_t __offset)
	:Name(__name)
	, Type(__type)
	, Size(__size)
	, Count(__count)
	, Offset(__offset)
{
}

ConstBufferDeclElement& ConstBufferDecl::Add(const ConstBufferDeclElement& elem)
{
	Elements.push_back(elem);
	return Elements.back();
}
ConstBufferDeclElement& ConstBufferDecl::Add(const ConstBufferDeclElement& elem, const ConstBufferDecl& subDecl)
{
	Elements.push_back(elem);
	SubDecls.insert(std::make_pair(elem.Name, subDecl));
	return Elements.back();
}
ConstBufferDeclElement& ConstBufferDecl::Last()
{
	return Elements.back();
}

/********** TConstBufferDeclBuilder **********/
ConstBufferDeclBuilder::ConstBufferDeclBuilder(ConstBufferDecl& decl)
	:mDecl(decl)
{
}

ConstBufferDeclBuilder& ConstBufferDeclBuilder::Add(const ConstBufferDeclElement& elem)
{
	mDecl.Add(elem).Offset = mDecl.BufferSize;
	mDecl.BufferSize += elem.Size;
	return *this;
}

ConstBufferDeclBuilder& ConstBufferDeclBuilder::Add(const ConstBufferDeclElement& elem, const ConstBufferDecl& subDecl)
{
	mDecl.Add(elem, subDecl).Offset = mDecl.BufferSize;
	mDecl.BufferSize += elem.Size;
	return *this;
}

ConstBufferDecl& ConstBufferDeclBuilder::Build()
{
	return mDecl;
}

/********** TBlendFunc **********/
BlendFunc::BlendFunc(D3D11_BLEND __src, D3D11_BLEND __dst)
{
	Src = __src;
	Dst = __dst;
}

BlendFunc BlendFunc::DISABLE = BlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_ZERO);
BlendFunc BlendFunc::ALPHA_PREMULTIPLIED = BlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA);
BlendFunc BlendFunc::ALPHA_NON_PREMULTIPLIED = BlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
BlendFunc BlendFunc::ADDITIVE = BlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE);

/********** TDepthState **********/
DepthState::DepthState(bool __depthEnable, D3D11_COMPARISON_FUNC __depthFunc /*= D3D11_COMPARISON_LESS*/, D3D11_DEPTH_WRITE_MASK __depthWriteMask /*= D3D11_DEPTH_WRITE_MASK_ALL*/)
{
	DepthEnable = __depthEnable;
	DepthFunc = __depthFunc;
	DepthWriteMask = __depthWriteMask;
}

DepthState DepthState::For2D = DepthState(false, D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK_ZERO);
DepthState DepthState::For3D = DepthState(true, D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK_ALL);

/********** TData **********/
TData::TData(void* __data, unsigned int __dataSize)
	:Data(__data)
	, DataSize(__dataSize)
{
}

}