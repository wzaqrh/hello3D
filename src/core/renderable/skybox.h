#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable_base.h"

namespace mir {

struct SkyboxVertex {
	Eigen::Vector4f pos;
};

class MIR_CORE_API SkyBox : public RenderableSingleRenderOp 
{
	typedef RenderableSingleRenderOp Super;
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(SkyBox);
	SkyBox(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName, const std::string& imgName);
public:
	void GenRenderOperation(RenderOperationQueue& opList) override;
};

}