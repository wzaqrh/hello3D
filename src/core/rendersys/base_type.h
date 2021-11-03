#pragma once
#include "core/base/std.h"

template<class T> T clamp(T minVal, T maxVal, T v) { return min(max(v, minVal),maxVal); }

struct TData 
{
	void* data;
	unsigned int dataSize;
	TData(void* __data, unsigned int __dataSize);
};
template <class T> inline TData make_data(const T& v) { return TData((void*)&v, sizeof(v)); }

struct TBlendFunc 
{
	D3D11_BLEND src, dst;

	TBlendFunc() :src(D3D11_BLEND_ONE), dst(D3D11_BLEND_INV_SRC_ALPHA) {};
	TBlendFunc(D3D11_BLEND __src, D3D11_BLEND __dst);

	static TBlendFunc DISABLE;
	static TBlendFunc ALPHA_PREMULTIPLIED;
	static TBlendFunc ALPHA_NON_PREMULTIPLIED;
	static TBlendFunc ADDITIVE;
};

struct TDepthState 
{
	BOOL depthEnable;
	D3D11_COMPARISON_FUNC depthFunc;
	D3D11_DEPTH_WRITE_MASK depthWriteMask;

	TDepthState() :depthEnable(false), depthFunc(D3D11_COMPARISON_LESS), depthWriteMask(D3D11_DEPTH_WRITE_MASK_ALL) {}
	TDepthState(bool __depthEnable, D3D11_COMPARISON_FUNC __depthFunc = D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK __depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL);

	static TDepthState For2D;
	static TDepthState For3D;
};

enum EConstBufferElementType
{
	E_CONSTBUF_ELEM_BOOL,
	E_CONSTBUF_ELEM_INT,
	E_CONSTBUF_ELEM_FLOAT,
	E_CONSTBUF_ELEM_FLOAT4,
	E_CONSTBUF_ELEM_MATRIX,
	E_CONSTBUF_ELEM_STRUCT,
	E_CONSTBUF_ELEM_MAX
};
struct TConstBufferDeclElement
{
	size_t offset;
	size_t size;
	size_t count;
	std::string name;
	EConstBufferElementType type;
public:
	TConstBufferDeclElement(const char* __name, EConstBufferElementType __type, size_t __size, size_t __count = 0, size_t __offset = 0);
};
struct TConstBufferDecl
{
	std::vector<TConstBufferDeclElement> elements;
	std::map<std::string, TConstBufferDecl> subDecls;
	size_t bufferSize = 0;
public:
	TConstBufferDeclElement& Add(const TConstBufferDeclElement& elem);
	TConstBufferDeclElement& Add(const TConstBufferDeclElement& elem, const TConstBufferDecl& subDecl);
	TConstBufferDeclElement& Last();
};
typedef std::shared_ptr<TConstBufferDecl> TConstBufferDeclPtr;