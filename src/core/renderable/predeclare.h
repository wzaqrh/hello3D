#pragma once
#include "core/base/std.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr

DECLARE_STRUCT(Transform);
DECLARE_STRUCT(TextureBySlot);

DECLARE_CLASS(SkyBox);
DECLARE_CLASS(PostProcess);
DECLARE_CLASS(Sprite);
DECLARE_CLASS(Mesh);
DECLARE_CLASS(Label);
DECLARE_CLASS(FontCache);

DECLARE_CLASS(RenderableFactory);

}