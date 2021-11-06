#pragma once

namespace mir {

template<class T> struct CBETypeTrait {};
#define DECLRAE_CBET(E,CLS) template<> struct CBETypeTrait<CLS> { enum {value = E }; };
DECLRAE_CBET(kCBElementInt, unsigned int);
DECLRAE_CBET(kCBElementFloat, float);
DECLRAE_CBET(kCBElementFloat4, XMFLOAT4);
DECLRAE_CBET(kCBElementMatrix, XMMATRIX);
DECLRAE_CBET(kCBElementStruct, cbDirectLight);
DECLRAE_CBET(kCBElementStruct, cbPointLight);
DECLRAE_CBET(kCBElementStruct, cbSpotLight);
template<class T> inline ConstBufferElementType GetCBEType(const T& Value) { return static_cast<ConstBufferElementType>(CBETypeTrait<T>::value); }
template<class T> inline ConstBufferDecl& GetDecl(const T& Value) { return T::GetDesc(); }


#define CBELEMNT(CLS) ConstBufferDeclElement(#CLS, GetCBEType(cb.CLS), sizeof(cb.CLS))
#define BUILD_ADD(CLS) builder.Add(CBELEMNT(CLS));
#define BUILD_ADDSUB(CLS) builder.Add(CBELEMNT(CLS), GetDecl(cb.CLS));

#define CBELEMNTS(CLS, N) ConstBufferDeclElement(#CLS, GetCBEType(cb.CLS[0]), sizeof(cb.CLS), ARRAYSIZE(cb.CLS))
#define BUILD_ADDS(CLS) builder.Add(CBELEMNTS(CLS));

#define CBBEGIN(CLS) static ConstBufferDecl decl; ConstBufferDeclBuilder builder(decl); CLS cb;


struct ConstBufferDeclBuilder
{
	ConstBufferDecl& mDecl;
public:
	//TConstBufferDeclBuilder();
	ConstBufferDeclBuilder(ConstBufferDecl& decl);
	ConstBufferDecl& Build();
	ConstBufferDeclBuilder& Add(const ConstBufferDeclElement& elem);
	ConstBufferDeclBuilder& Add(const ConstBufferDeclElement& elem, const ConstBufferDecl& subDecl);
};
#define MAKE_CBDESC(CB) (CB::GetDesc())

}