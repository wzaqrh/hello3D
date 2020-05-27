#pragma once

template<class T> struct CBETypeTrait {};
#define DECLRAE_CBET(E,CLS) template<> struct CBETypeTrait<CLS> { enum {value = E }; };
DECLRAE_CBET(E_CONSTBUF_ELEM_INT, unsigned int);
DECLRAE_CBET(E_CONSTBUF_ELEM_FLOAT, float);
DECLRAE_CBET(E_CONSTBUF_ELEM_FLOAT4, XMFLOAT4);
DECLRAE_CBET(E_CONSTBUF_ELEM_MATRIX, XMMATRIX);
DECLRAE_CBET(E_CONSTBUF_ELEM_STRUCT, TDirectLight);
DECLRAE_CBET(E_CONSTBUF_ELEM_STRUCT, TPointLight);
DECLRAE_CBET(E_CONSTBUF_ELEM_STRUCT, TSpotLight);
template<class T> inline EConstBufferElementType GetCBEType(const T& Value) { return static_cast<EConstBufferElementType>(CBETypeTrait<T>::value); }
template<class T> inline TConstBufferDecl& GetDecl(const T& Value) { return T::GetDesc(); }



#define CBELEMNT(CLS) TConstBufferDeclElement(#CLS, GetCBEType(cb.CLS), sizeof(cb.CLS))
#define BUILD_ADD(CLS) builder.Add(CBELEMNT(CLS));

#define BUILD_ADDSUB(CLS) builder.Add(CBELEMNT(CLS), GetDecl(cb.CLS));

#define CBELEMNTS(CLS, N) TConstBufferDeclElement(#CLS, GetCBEType(cb.CLS[0]), sizeof(cb.CLS), ARRAYSIZE(cb.CLS))
#define BUILD_ADDS(CLS) builder.Add(CBELEMNTS(CLS));

#define CBBEGIN(CLS) static TConstBufferDecl decl; TConstBufferDeclBuilder builder(decl); CLS cb;



struct TConstBufferDeclBuilder
{
	TConstBufferDecl& mDecl;
public:
	//TConstBufferDeclBuilder();
	TConstBufferDeclBuilder(TConstBufferDecl& decl);
	TConstBufferDecl& Build();
	TConstBufferDeclBuilder& Add(const TConstBufferDeclElement& elem);
	TConstBufferDeclBuilder& Add(const TConstBufferDeclElement& elem, const TConstBufferDecl& subDecl);
};
#define MAKE_CBDESC(CB) (CB::GetDesc())