#include "core/rendersys/material_cb.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/const_buffer_decl.h"

namespace mir {

/********** TDirectLight **********/
TDirectLight::TDirectLight()
{
	SetDirection(0, 0, 1);
	SetDiffuseColor(1, 1, 1, 1);
	SetSpecularColor(1, 1, 1, 1);
	SetSpecularPower(32);
}

void TDirectLight::SetDirection(float x, float y, float z)
{
	LightPos = XMFLOAT4(x, y, z, 0);
}

void TDirectLight::SetDiffuseColor(float r, float g, float b, float a)
{
	DiffuseColor = XMFLOAT4(r, g, b, a);
}

void TDirectLight::SetSpecularColor(float r, float g, float b, float a)
{
	SpecularColorPower = XMFLOAT4(r, g, b, SpecularColorPower.w);
}

void TDirectLight::SetSpecularPower(float power)
{
	SpecularColorPower.w = power;
}

TConstBufferDecl& TDirectLight::GetDesc()
{
	static TConstBufferDecl decl;
	TConstBufferDeclBuilder builder(decl);
	TDirectLight cb;
	BUILD_ADD(LightPos);
	BUILD_ADD(DiffuseColor);
	BUILD_ADD(SpecularColorPower);
	return builder.Build();
}

/********** TLight **********/
TPointLight::TPointLight()
{
	SetPosition(0, 0, -10);
	SetAttenuation(1.0, 0.01, 0.0);
}

void TPointLight::SetPosition(float x, float y, float z)
{
	LightPos = XMFLOAT4(x, y, z, 1);
}

void TPointLight::SetAttenuation(float a, float b, float c)
{
	Attenuation = XMFLOAT4(a, b, c, 0);
}

TConstBufferDecl& TPointLight::GetDesc()
{
	static TConstBufferDecl decl;
	decl = TDirectLight::GetDesc();

	TConstBufferDeclBuilder builder(decl);
	TPointLight cb;
	BUILD_ADD(Attenuation);
	return builder.Build();
}

/********** TSpotLight **********/
TSpotLight::TSpotLight()
{
	SetDirection(0, 0, 1);
	SetAngle(3.14 * 30 / 180);
}

void TSpotLight::SetDirection(float x, float y, float z)
{
	DirectionCutOff = XMFLOAT4(x, y, z, DirectionCutOff.w);
}

void TSpotLight::SetCutOff(float cutoff)
{
	DirectionCutOff.w = cutoff;
}

void TSpotLight::SetAngle(float radian)
{
	SetCutOff(cos(radian));
}

TConstBufferDecl& TSpotLight::GetDesc()
{
	static TConstBufferDecl decl;
	decl = TPointLight::GetDesc();

	TConstBufferDeclBuilder builder(decl);
	TSpotLight cb;
	BUILD_ADD(DirectionCutOff);
	return builder.Build();
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
	CBBEGIN(cbGlobalParam);
	BUILD_ADD(World);
	BUILD_ADD(View);
	BUILD_ADD(Projection);

	BUILD_ADD(WorldInv);
	BUILD_ADD(ViewInv);
	BUILD_ADD(ProjectionInv);

	BUILD_ADD(LightView);
	BUILD_ADD(LightProjection);
	BUILD_ADDSUB(Light);
	
	BUILD_ADD(LightType);
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
	CBBEGIN(TFogExp);
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
	CBBEGIN(cbWeightedSkin);
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
	CBBEGIN(cbUnityMaterial);
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
	CBBEGIN(cbUnityGlobal);
	BUILD_ADD(_Unity_IndirectSpecColor);
	BUILD_ADD(_AmbientOrLightmapUV);
	BUILD_ADD(_Unity_SpecCube0_HDR);
	return builder.Build();
}

}