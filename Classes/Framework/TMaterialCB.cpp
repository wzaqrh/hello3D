#include "TMaterialCB.h"

TConstBufferDeclElement::TConstBufferDeclElement(size_t __size, EConstBufferElementType __type, D3DTRANSFORMSTATETYPE __mtype /*= D3DTS_FORCE_DWORD*/, size_t __offset /*= 0*/)
	:offset(__offset)
	,size(__size)
	,type(__type)
	,matrixType(__mtype)
{
}

TConstBufferDeclElement& TConstBufferDecl::Add(const TConstBufferDeclElement& elem)
{
	elements.push_back(elem);
	return elements.back();
}

TConstBufferDecl TConstBufferDeclBuilder::Build()
{
	return mDecl;
}

TConstBufferDeclBuilder& TConstBufferDeclBuilder::Add(const TConstBufferDeclElement& elem)
{
	mDecl.Add(elem).offset = mDecl.bufferSize;
	mDecl.bufferSize += elem.size;
	return *this;
}

/********** cbGlobalParam **********/
cbGlobalParam::cbGlobalParam()
{
	auto Ident = XMMatrixIdentity();
	mWorld = Ident;
	mView = Ident;
	mProjection = Ident;
}

TConstBufferDecl& cbGlobalParam::GetDesc()
{
	static TConstBufferDecl desc = MKDesc();
	return desc;
}

TConstBufferDecl cbGlobalParam::MKDesc()
{
	TConstBufferDeclBuilder builder;
	for (size_t i = 0; i < 6; ++i) {
		builder.Add(CBELEMNT(XMMATRIX, E_CONSTBUF_ELEM_MATRIX));
	}
	builder.Add(CBELEMNT(TINT4, E_CONSTBUF_ELEM_INT4));
	for (size_t i = 0; i < MAX_LIGHTS; ++i) {
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
	}
	for (size_t i = 0; i < MAX_LIGHTS; ++i) {
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
	}
	for (size_t i = 0; i < MAX_LIGHTS; ++i) {
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
		builder.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4));
	}
	builder.Add(CBELEMNT(XMMATRIX, E_CONSTBUF_ELEM_MATRIX));
	builder.Add(CBELEMNT(XMMATRIX, E_CONSTBUF_ELEM_MATRIX));
	builder.Add(CBELEMNT(BOOL, E_CONSTBUF_ELEM_BOOL));
	return builder.Build();
}

/********** cbWeightedSkin **********/
TConstBufferDecl& cbWeightedSkin::GetDesc()
{
	static TConstBufferDecl desc = MKDesc();
	return desc;
}

TConstBufferDecl cbWeightedSkin::MKDesc()
{
	TConstBufferDeclBuilder builder;
	builder.Add(CBELEMNT(XMMATRIX, E_CONSTBUF_ELEM_MATRIX));
	for (size_t i = 0; i < MAX_MATRICES; ++i)
		builder.Add(CBELEMNT(XMMATRIX, E_CONSTBUF_ELEM_MATRIX));
	builder.Add(CBELEMNT(TINT4, E_CONSTBUF_ELEM_INT4));
	return builder.Build();
}

/********** cbUnityMaterial **********/
cbUnityMaterial::cbUnityMaterial()
{
	_SpecColor = XMFLOAT4(1, 1, 1, 1);
	_Color = XMFLOAT4(1, 1, 1, 1);
	_GlossMapScale = 1;
	_OcclusionStrength = 1;
	_SpecLightOff = 0;
}

TConstBufferDecl& cbUnityMaterial::GetDesc()
{
	static TConstBufferDecl desc = 
		TConstBufferDeclBuilder()
		.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4))
		.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4))
		.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4))
		.Add(CBELEMNT(TINT4, E_CONSTBUF_ELEM_INT4))
		.Build();
	return desc;
}

/********** cbUnityGlobal **********/
cbUnityGlobal::cbUnityGlobal()
{
	_Unity_IndirectSpecColor = XMFLOAT4(0, 0, 0, 0);
	_AmbientOrLightmapUV = XMFLOAT4(0.01, 0.01, 0.01, 1);
	_Unity_SpecCube0_HDR = XMFLOAT4(0.5, 1, 0, 0);
}

TConstBufferDecl& cbUnityGlobal::GetDesc()
{
	static TConstBufferDecl desc =
		TConstBufferDeclBuilder()
		.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4))
		.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4))
		.Add(CBELEMNT(XMFLOAT4, E_CONSTBUF_ELEM_FLOAT4))
		.Build();
	return desc;
}
