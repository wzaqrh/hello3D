#include "core/rendersys/material_cb.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/camera.h"

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
	LightPos = Eigen::Vector4f(x, y, z, 0);
}

void cbDirectLight::SetDiffuseColor(float r, float g, float b, float a)
{
	DiffuseColor = Eigen::Vector4f(r, g, b, a);
}

void cbDirectLight::SetSpecularColor(float r, float g, float b, float a)
{
	SpecularColorPower = Eigen::Vector4f(r, g, b, SpecularColorPower.w());
}

void cbDirectLight::SetSpecularPower(float power)
{
	SpecularColorPower.w() = power;
}

#if !defined MATERIAL_FROM_XML
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
#endif

/********** cbDirectLight **********/
void cbDirectLight::CalculateLightingViewProjection(const Camera& camera, XMMATRIX& view, XMMATRIX& proj) {
	XMVECTOR Eye = XMVectorSet(LightPos.x(), LightPos.y(), LightPos.z(), 0.0f);
	XMVECTOR At = XMVectorSet(camera.mLookAtPos.x, camera.mLookAtPos.y, camera.mLookAtPos.z, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	view = XMMatrixLookAtLH(Eye, At, Up);
	proj = XMMatrixOrthographicOffCenterLH(0, camera.mWidth, 0, camera.mHeight, 0.01, camera.mZFar);
}

/********** cbPointLight **********/
cbPointLight::cbPointLight()
{
	SetPosition(0, 0, -10);
	SetAttenuation(1.0, 0.01, 0.0);
}

void cbPointLight::SetPosition(float x, float y, float z)
{
	LightPos = Eigen::Vector4f(x, y, z, 1);
}

void cbPointLight::SetAttenuation(float a, float b, float c)
{
	Attenuation = Eigen::Vector4f(a, b, c, 0);
}

#if !defined MATERIAL_FROM_XML
ConstBufferDecl& cbPointLight::GetDesc()
{
	static ConstBufferDecl decl;
	decl = cbDirectLight::GetDesc();

	ConstBufferDeclBuilder builder(decl);
	cbPointLight cb;
	BUILD_ADD(Attenuation);
	return builder.Build();
}
#endif

/********** cbSpotLight **********/
cbSpotLight::cbSpotLight()
{
	SetDirection(0, 0, 1);
	SetAngle(3.14 * 30 / 180);
}

void cbSpotLight::SetDirection(float x, float y, float z)
{
	DirectionCutOff = Eigen::Vector4f(x, y, z, DirectionCutOff.w());
}

void cbSpotLight::SetCutOff(float cutoff)
{
	DirectionCutOff.w() = cutoff;
}

void cbSpotLight::SetAngle(float radian)
{
	SetCutOff(cos(radian));
}

#if !defined MATERIAL_FROM_XML
ConstBufferDecl& cbSpotLight::GetDesc()
{
	static ConstBufferDecl decl;
	decl = cbPointLight::GetDesc();

	ConstBufferDeclBuilder builder(decl);
	cbSpotLight cb;
	BUILD_ADD(DirectionCutOff);
	return builder.Build();
}
#endif

/********** cbGlobalParam **********/
cbGlobalParam::cbGlobalParam()
{
	auto Ident = XMMatrixIdentity();
	World = Eigen::Matrix4f::Identity();
	View = Eigen::Matrix4f::Identity();
	Projection = Eigen::Matrix4f::Identity();
}

#if !defined MATERIAL_FROM_XML
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
#endif

/********** TFogExp **********/
cbFogExp::cbFogExp()
{
	SetColor(0.5, 0.5, 0.5);
	SetExp(1);
}

void cbFogExp::SetColor(float r, float g, float b)
{
	FogColorExp = Eigen::Vector4f(r, g, b, FogColorExp.w());
}

void cbFogExp::SetExp(float exp)
{
	FogColorExp.w() = exp;
}

#if !defined MATERIAL_FROM_XML
ConstBufferDecl& cbFogExp::GetDesc()
{
	CBBEGIN(cbFogExp);
	BUILD_ADD(FogColorExp);
	return builder.Build();
}
#endif

/********** cbWeightedSkin **********/
#if !defined MATERIAL_FROM_XML
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
#endif

/********** cbUnityMaterial **********/
cbUnityMaterial::cbUnityMaterial()
{
	_SpecColor = Eigen::Vector4f(1, 1, 1, 1);
	_Color = Eigen::Vector4f(1, 1, 1, 1);
	_GlossMapScale = 1;
	_OcclusionStrength = 1;
	_SpecLightOff = 0;
}

#if !defined MATERIAL_FROM_XML
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
#endif

/********** cbUnityGlobal **********/
cbUnityGlobal::cbUnityGlobal()
{
	_Unity_IndirectSpecColor = Eigen::Vector4f(0, 0, 0, 0);
	_AmbientOrLightmapUV = Eigen::Vector4f(0.01, 0.01, 0.01, 1);
	_Unity_SpecCube0_HDR = Eigen::Vector4f(0.5, 1, 0, 0);
}

#if !defined MATERIAL_FROM_XML
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
#endif

/********** cbBloom **********/
cbBloom cbBloom::CreateDownScale2x2Offsets(int dwWidth, int dwHeight)
{
	cbBloom bloom = {};
	float tU = 1.0f / dwWidth;// / 2.0f;
	float tV = 1.0f / dwHeight;// / 2.0f;
	// Sample from 4 surrounding points. 
	int index = 0;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			bloom.SampleOffsets[index].x() = (x - 0.5f) * tU;
			bloom.SampleOffsets[index].y() = (y - 0.5f) * tV;
			index++;
		}
	}
	//return bloom;
	return CreateDownScale3x3Offsets(dwWidth, dwHeight);
}

cbBloom cbBloom::CreateDownScale3x3Offsets(int dwWidth, int dwHeight)
{
	cbBloom bloom = {};
	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;
	// Sample from the 9 surrounding points. 
	int index = 0;
	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			bloom.SampleOffsets[index].x() = x * tU;
			bloom.SampleOffsets[index].y() = y * tV;
			index++;
		}
	}
	return bloom;
}
inline float GaussianDistribution(float x, float y, float rho)
{
	float g = 1.0f / sqrtf(2.0f * 3.141592654f * rho * rho);
	g *= expf(-(x * x + y * y) / (2 * rho * rho));
	return g;
}

cbBloom cbBloom::CreateBloomOffsets(int dwD3DTexSize, float fDeviation, float fMultiplier)
{
	cbBloom bloom = {};
	int i = 0;
	float tu = 1.0f / (float)dwD3DTexSize;

	// Fill the center texel
	float weight = 1.0f * GaussianDistribution(0, 0, fDeviation);
	bloom.SampleOffsets[0] = Eigen::Vector4f(0.0f, 0, 0, 0);
	bloom.SampleWeights[0] = Eigen::Vector4f(weight, weight, weight, 1.0f);

	// Fill the right side
	for (i = 1; i < 8; i++) {
		weight = fMultiplier * GaussianDistribution((float)i, 0, fDeviation);
		bloom.SampleOffsets[i] = Eigen::Vector4f(i * tu, 0, 0, 0);
		bloom.SampleWeights[i] = Eigen::Vector4f(weight, weight, weight, 1.0f);
	}

	// Copy to the left side
	for (i = 8; i < 15; i++) {
		bloom.SampleOffsets[i] = Eigen::Vector4f(-bloom.SampleOffsets[i - 7].x(), 0, 0, 0);
		bloom.SampleWeights[i] = bloom.SampleWeights[i - 7];
	}
	return bloom;
}

#if !defined MATERIAL_FROM_XML
ConstBufferDecl& cbBloom::GetDesc()
{
	CBBEGIN(cbBloom);
	builder.Add(CBELEMNTS(SampleOffsets));
	builder.Add(CBELEMNTS(SampleWeights));
	return builder.Build();
}
#endif
}