#pragma once
#include "TApp.h"
#include "AssimpModel.h"
#include "TSprite.h"

class LessonD3D9 : public TApp
{
public:
	LessonD3D9();
	~LessonD3D9();
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual IRenderSystem* OnCreateRenderSys() override;
};

