#pragma once
#include "TApp.h"
#include "AssimpModel.h"
#include "TSprite.h"

struct cbUnityMaterial 
{
	XMFLOAT4 _SpecColor;
	XMFLOAT4 _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	cbUnityMaterial();
};

struct cbUnityGlobal
{
	XMFLOAT4 _Unity_IndirectSpecColor;
	XMFLOAT4 _AmbientOrLightmapUV;
	cbUnityGlobal();
};

class Lesson6 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
};

