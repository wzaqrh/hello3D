#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr

DECLARE_STRUCT(Transform);
DECLARE_STRUCT(TextureVector);

namespace renderable {
DECLARE_CLASS(SkyBox);
DECLARE_CLASS(Sprite);
DECLARE_CLASS(Mesh);
DECLARE_CLASS(AssimpModel);
DECLARE_CLASS(Cube);
DECLARE_CLASS(PostProcess);
DECLARE_CLASS(Bloom);
DECLARE_CLASS(Label);
}
DECLARE_CLASS(FontCache);

DECLARE_STRUCT(RenderOperation);
DECLARE_STRUCT(RenderOperationQueue);
DECLARE_STRUCT(IRenderable);
DECLARE_CLASS(RenderableFactory);

}