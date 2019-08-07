#include "LessonD3D9.h"
#include "TRenderSystem9.h"

LessonD3D9::LessonD3D9()
{
}

LessonD3D9::~LessonD3D9()
{
}

IRenderSystem* LessonD3D9::OnCreateRenderSys()
{
	return new TRenderSystem9;
}

void LessonD3D9::OnRender()
{

}

void LessonD3D9::OnPostInitDevice()
{

}

auto reg = AppRegister<LessonD3D9>("LessonD3D9: D3D9");