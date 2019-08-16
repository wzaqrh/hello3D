#include "TMaterialCB.h"

TConstBufferDeclElement::TConstBufferDeclElement(const char* __name, EConstBufferElementType __type, size_t __size, size_t __count, size_t __offset)
	:name(__name)
	,type(__type)
	,size(__size)
	,count(__count)
	,offset(__offset)
{
}

TConstBufferDeclElement& TConstBufferDecl::Add(const TConstBufferDeclElement& elem)
{
	elements.push_back(elem);
	return elements.back();
}

TConstBufferDecl& TConstBufferDeclBuilder::Build()
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
	World = Ident;
	View = Ident;
	Projection = Ident;
}

TConstBufferDecl& cbGlobalParam::GetDesc()
{
	static TConstBufferDecl desc = MKDesc();
	return desc;
}

TConstBufferDecl cbGlobalParam::MKDesc()
{
	TConstBufferDeclBuilder builder;
	cbGlobalParam cb;
	BUILD_ADD(World);
	BUILD_ADD(View);
	BUILD_ADD(Projection);

	BUILD_ADD(WorldInv);
	BUILD_ADD(ViewInv);
	BUILD_ADD(ProjectionInv);

	BUILD_ADD(LightType);
	BUILD_ADD(Light);

	BUILD_ADD(LightView);
	BUILD_ADD(LightProjection);
	BUILD_ADD(HasDepthMap);
	return builder.Build();
}

/********** TFogExp **********/
TFogExp::TFogExp()
{
	SetColor(0.5, 0.5, 0.5);
	SetExp(1);
}

void TFogExp::SetColor(float r, float g, float b)
{
	FogColorExp = XMFLOAT4(r, g, b, FogColorExp.w);
}

void TFogExp::SetExp(float exp)
{
	FogColorExp.w = exp;
}

TConstBufferDecl& TFogExp::GetDesc()
{
	TConstBufferDeclBuilder builder;
	TFogExp cb;
	BUILD_ADD(FogColorExp);
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
	cbWeightedSkin cb;
	BUILD_ADD(Model);
	BUILD_ADDS(Models);
	BUILD_ADD(hasNormal);
	BUILD_ADD(hasMetalness);
	BUILD_ADD(hasRoughness);
	BUILD_ADD(hasAO);
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
	static TConstBufferDecl desc = MKDesc();
	return desc;
}

TConstBufferDecl cbUnityMaterial::MKDesc()
{
	TConstBufferDeclBuilder builder;
	cbUnityMaterial cb;
	BUILD_ADD(_SpecColor);
	BUILD_ADD(_Color);
	BUILD_ADD(_GlossMapScale);
	BUILD_ADD(_OcclusionStrength);
	BUILD_ADD(_SpecLightOff);
	return builder.Build();
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
	static TConstBufferDecl desc = MKDesc();
	return desc;
}

TConstBufferDecl cbUnityGlobal::MKDesc()
{
	TConstBufferDeclBuilder builder;
	cbUnityGlobal cb;
	BUILD_ADD(_Unity_IndirectSpecColor);
	BUILD_ADD(_AmbientOrLightmapUV);
	BUILD_ADD(_Unity_SpecCube0_HDR);
	return builder.Build();
}