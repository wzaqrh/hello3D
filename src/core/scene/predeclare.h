#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr; typedef std::weak_ptr<TYPE> TYPE##WeakPtr;  
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr; typedef std::weak_ptr<TYPE> TYPE##WeakPtr;  

namespace scene {
DECLARE_CLASS(Light);
DECLARE_CLASS(DirectLight);
DECLARE_CLASS(PointLight);
DECLARE_CLASS(SpotLight);
DECLARE_CLASS(Camera);
}
DECLARE_CLASS(Transform);
DECLARE_CLASS(SceneNode);
DECLARE_CLASS(SceneManager);

}