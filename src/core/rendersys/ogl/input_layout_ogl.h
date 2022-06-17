#pragma once
#include <windows.h>
#include "core/rendersys/input_layout.h"

namespace mir {

class InputLayoutOGL : public ImplementResource<IInputLayout>
{
public:
	InputLayoutOGL() {}
	void Init(const std::vector<LayoutInputElement>& layoutElems) {
		mLayoutElements = layoutElems;
	}
	const std::vector<LayoutInputElement>& GetLayoutElements() const { return mLayoutElements; }
public:
	std::vector<LayoutInputElement> mLayoutElements;
};

}