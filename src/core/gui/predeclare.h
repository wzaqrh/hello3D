#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; using TYPE##Ptr = std::shared_ptr<TYPE>; using TYPE##RawPtr = TYPE*; using TYPE##WeakPtr = std::weak_ptr<TYPE>;  
#define DECLARE_CLASS(TYPE)   class TYPE; using TYPE##Ptr = std::shared_ptr<TYPE>; using TYPE##RawPtr = TYPE*; using TYPE##WeakPtr = std::weak_ptr<TYPE>;  

namespace gui {
DECLARE_CLASS(GuiCanvas);
}
DECLARE_CLASS(GuiManager);

}