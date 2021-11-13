#pragma once
#include "core/base/std.h"
#include "core/rendersys/d3d11/stddx11.h"

namespace mir {

struct Data 
{
	void* Datas;
	unsigned int DataSize;
	template<class T> static Data Make(const T& v) { return Data{ (void*)&v, sizeof(v) }; }
};

struct BlendFunc 
{
	D3D11_BLEND Src, Dst;

	BlendFunc() :Src(D3D11_BLEND_ONE), Dst(D3D11_BLEND_INV_SRC_ALPHA) {};
	BlendFunc(D3D11_BLEND __src, D3D11_BLEND __dst);

	static BlendFunc DISABLE;
	static BlendFunc ALPHA_PREMULTIPLIED;
	static BlendFunc ALPHA_NON_PREMULTIPLIED;
	static BlendFunc ADDITIVE;
};

struct DepthState 
{
	BOOL DepthEnable;
	D3D11_COMPARISON_FUNC DepthFunc;
	D3D11_DEPTH_WRITE_MASK DepthWriteMask;

	DepthState() :DepthEnable(false), DepthFunc(D3D11_COMPARISON_LESS), DepthWriteMask(D3D11_DEPTH_WRITE_MASK_ALL) {}
	DepthState(bool __depthEnable, D3D11_COMPARISON_FUNC __depthFunc = D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK __depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL);

	static DepthState For2D;
	static DepthState For3D;
};

enum ConstBufferElementType
{
	kCBElementBool,
	kCBElementInt,
	kCBElementFloat,
	kCBElementFloat4,
	kCBElementMatrix,
	kCBElementStruct,
	kCBElementMax
};
struct ConstBufferDeclElement
{
	std::string Name;
	ConstBufferElementType Type;
	size_t Size;
	size_t Count;
	size_t Offset;
public:
	ConstBufferDeclElement(const char* __name, ConstBufferElementType __type, size_t __size, size_t __count = 0, size_t __offset = 0);
};
struct ConstBufferDecl
{
	std::vector<ConstBufferDeclElement> Elements;
	std::map<std::string, ConstBufferDecl> SubDecls;
	size_t BufferSize = 0;
public:
	ConstBufferDeclElement& Add(const ConstBufferDeclElement& elem);
	ConstBufferDeclElement& Add(const ConstBufferDeclElement& elem, const ConstBufferDecl& subDecl);
	ConstBufferDeclElement& Last();
};

}