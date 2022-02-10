#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable_base.h"

namespace mir {
namespace rend {

class MIR_CORE_API Cube : public RenderableSingleRenderOp
{
	INHERIT_RENDERABLE_SINGLE_OP(Cube);
	Cube(Launch launchMode, ResourceManager& resourceMng, const res::MaterialInstance& material);
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void SetPosition(const Eigen::Vector3f& pos);
	void SetHalfSize(const Eigen::Vector3f& size);
	void SetColor(const Eigen::Vector4f& color);
	void SetColor(unsigned bgra);
public:
	void GenRenderOperation(RenderOperationQueue& opList) override;
private:
	vbSurfaceCube mVertexData;
	bool mVertexDirty;
	Eigen::Vector3f mPosition;
	Eigen::Vector3f mHalfSize;
	Eigen::Vector4f mColor;
};

}
}