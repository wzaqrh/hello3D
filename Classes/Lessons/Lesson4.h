#pragma once
#include "TApp.h"
#include "AssimpModel.h"

struct TFogExp {
	XMFLOAT4 mFogColorExp;
public:
	TFogExp();
	void SetColor(float r, float g, float b);
	void SetExp(float exp);
	static TConstBufferDecl& GetDesc();
};

class Lesson4 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
};

