#include "core/rendersys/base_type.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/const_buffer_decl.h"
#include "core/base/utility.h"

/********** TConstBufferDeclElement **********/
TConstBufferDeclElement::TConstBufferDeclElement(const char* __name, EConstBufferElementType __type, size_t __size, size_t __count, size_t __offset)
	:name(__name)
	, type(__type)
	, size(__size)
	, count(__count)
	, offset(__offset)
{
}

TConstBufferDeclElement& TConstBufferDecl::Add(const TConstBufferDeclElement& elem)
{
	elements.push_back(elem);
	return elements.back();
}
TConstBufferDeclElement& TConstBufferDecl::Add(const TConstBufferDeclElement& elem, const TConstBufferDecl& subDecl)
{
	elements.push_back(elem);
	subDecls.insert(std::make_pair(elem.name, subDecl));
	return elements.back();
}
TConstBufferDeclElement& TConstBufferDecl::Last()
{
	return elements.back();
}

/********** TConstBufferDeclBuilder **********/
TConstBufferDeclBuilder::TConstBufferDeclBuilder(TConstBufferDecl& decl)
	:mDecl(decl)
{
}

TConstBufferDeclBuilder& TConstBufferDeclBuilder::Add(const TConstBufferDeclElement& elem)
{
	mDecl.Add(elem).offset = mDecl.bufferSize;
	mDecl.bufferSize += elem.size;
	return *this;
}

TConstBufferDeclBuilder& TConstBufferDeclBuilder::Add(const TConstBufferDeclElement& elem, const TConstBufferDecl& subDecl)
{
	mDecl.Add(elem, subDecl).offset = mDecl.bufferSize;
	mDecl.bufferSize += elem.size;
	return *this;
}

TConstBufferDecl& TConstBufferDeclBuilder::Build()
{
	return mDecl;
}

/********** TBlendFunc **********/
TBlendFunc::TBlendFunc(D3D11_BLEND __src, D3D11_BLEND __dst)
{
	src = __src;
	dst = __dst;
}

TBlendFunc TBlendFunc::DISABLE = TBlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_ZERO);
TBlendFunc TBlendFunc::ALPHA_PREMULTIPLIED = TBlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA);
TBlendFunc TBlendFunc::ALPHA_NON_PREMULTIPLIED = TBlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
TBlendFunc TBlendFunc::ADDITIVE = TBlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE);

/********** TDepthState **********/
TDepthState::TDepthState(bool __depthEnable, D3D11_COMPARISON_FUNC __depthFunc /*= D3D11_COMPARISON_LESS*/, D3D11_DEPTH_WRITE_MASK __depthWriteMask /*= D3D11_DEPTH_WRITE_MASK_ALL*/)
{
	depthEnable = __depthEnable;
	depthFunc = __depthFunc;
	depthWriteMask = __depthWriteMask;
}

TDepthState TDepthState::For2D = TDepthState(false, D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK_ZERO);
TDepthState TDepthState::For3D = TDepthState(true, D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK_ALL);

/********** TData **********/
TData::TData(void* __data, unsigned int __dataSize)
	:data(__data)
	, dataSize(__dataSize)
{
}
