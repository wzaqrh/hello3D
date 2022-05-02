#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;

DECLARE_STRUCT(IResource);
DECLARE_CLASS(ResourceManager);
namespace res {
DECLARE_CLASS(UniformParameters);
DECLARE_CLASS(GpuParameters);
DECLARE_CLASS(PassProperty);
DECLARE_CLASS(Pass);
DECLARE_CLASS(Technique);
DECLARE_CLASS(Shader);
DECLARE_CLASS(Material);
DECLARE_CLASS(MaterialFactory);

DECLARE_CLASS(AssimpMesh);
DECLARE_CLASS(AiNode);
DECLARE_CLASS(AiScene);
DECLARE_CLASS(AiResourceFactory);
}

}