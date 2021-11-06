#include "core/rendersys/material_cb.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/const_buffer_decl.h"

namespace mir {

/********** TDirectLight **********/
cbDirectLight::cbDirectLight()
{
	SetDirection(0, 0, 1);
	SetDiffuseColor(1, 1, 1, 1);
	SetSpecularColor(1, 1, 1, 1);
	SetSpecularPower(32);
}

void cbDirectLight::SetDirection(float x, float y, float z)
{
	LightPos = XMFLOAT4(x, y, z, 0);
}

void cbDirectLight::SetDiffuseColor(float r, float g, float b, float a)
{
	DiffuseColor = XMFLOAT4(r, g, b, a);
}

void cbDirectLight::SetSpecularColor(float r, float g, float b, float a)
{
	SpecularColorPower = XMFLOAT4(r, g, b, SpecularColorPower.w);
}

void cbDirectLight::SetSpecularPower(float power)
{
	SpecularColorPower.w = power;
}

ConstBufferDecl& cbDirectLight::GetDesc()
{
	static ConstBufferDecl decl;
	ConstBufferDeclBuilder builder(decl);
	cbDirectLight cb;
	BUILD_ADD(LightPos);
	BUILD_ADD(DiffuseColor);
	BUILD_ADD(SpecularColorPower);
	return builder.Build();
}

/********** TLight **********/
cbPointLight::cbPointLight()
{
	SetPosition(0, 0, -10);
	SetAttenuation(1.0, 0.01, 0.0);
}

void cbPointLight::SetPosition(float x, float y, float z)
{
	LightPos = XMFLOAT4(x, y, z, 1);
}

void cbPointLight::SetAttenuation(float a, float b, float c)
{
	Attenuation = XMFLOAT4(a, b, c, 0);
}

ConstBufferDecl& cbPointLight::GetDesc()
{
	static ConstBufferDecl decl;
	decl = cbDirectLight::GetDesc();

	ConstBufferDeclBuilder builder(decl);
	cbPointLight cb;
	BUILD_ADD(Attenuation);
	return builder.Build();
}

/********** TSpotLight **********/
cbSpotLight::cbSpotLight()
{
	SetDirection(0, 0, 1);
	SetAngle(3.14 * 30 / 180);
}

void cbSpotLight::SetDirection(float x, float y, float z)
{
	DirectionCutOff = XMFLOAT4(x, y, z, DirectionCutOff.w);
}

void cbSpotLight::SetCutOff(float cutoff)
{
	DirectionCutOff.w = cutoff;
}

void cbSpotLight::SetAngle(float radian)
{
	SetCutOff(cos(radian));
}

ConstBufferDecl& cbSpotLight::GetDesc()
{
	static ConstBufferDecl decl;
	decl = cbPointLight::GetDesc();

	ConstBufferDeclBuilder builder(decl);
	cbSpotLight cb;
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

ConstBufferDecl& cbGlobalParam::GetDesc()
{
	static ConstBufferDecl desc = MKDesc();
	return desc;
}

ConstBufferDecl cbGlobalParam::MKDesc()
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
cbFogExp::cbFogExp()
{
	SetColor(0.5, 0.5, 0.5);
	SetExp(1);
}

void cbFogExp::SetColor(float r, float g, float b)
{
	FogColorExp = XMFLOAT4(r, g, b, FogColorExp.w);
}

void cbFogExp::SetExp(float exp)
{
	FogColorExp.w = exp;
}

ConstBufferDecl& cbFogExp::GetDesc()
{
	CBBEGIN(cbFogExp);
	BUILD_ADD(FogColorExp);
	return builder.Build();
}

/********** cbWeightedSkin **********/
ConstBufferDecl& cbWeightedSkin::GetDesc()
{
	static ConstBufferDecl desc = MKDesc();
	return desc;
}

ConstBufferDecl cbWeightedSkin::MKDesc()
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

ConstBufferDecl& cbUnityMaterial::GetDesc()
{
	static ConstBufferDecl desc = MKDesc();
	return desc;
}

ConstBufferDecl cbUnityMaterial::MKDesc()
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

ConstBufferDecl& cbUnityGlobal::GetDesc()
{
	static ConstBufferDecl desc = MKDesc();
	return desc;
}

ConstBufferDecl cbUnityGlobal::MKDesc()
{
	CBBEGIN(cbUnityGlobal);
	BUILD_ADD(_Unity_IndirectSpecColor);
	BUILD_ADD(_AmbientOrLightmapUV);
	BUILD_ADD(_Unity_SpecCube0_HDR);
	return builder.Build();
}

}