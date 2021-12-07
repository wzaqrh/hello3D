#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable.h"

namespace mir {

class MIR_CORE_API Cube : public IRenderable
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(Cube);
	Cube(Launch launchMode, ResourceManager& resourceMng, const std::string& matName = "");
public:
	void SetPosition(const Eigen::Vector3f& pos);
	void SetHalfSize(const Eigen::Vector3f& size);
	void SetColor(const Eigen::Vector4f& color);
	void SetColor(unsigned bgra);
public:
	int GenRenderOperation(RenderOperationQueue& opList) override;
	const MaterialPtr& GetMaterial() const { return mMaterial; }
	const TransformPtr& GetTransform() const { return mTransform; }
private:
	ResourceManager& mResourceMng;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr mMaterial;
	TransformPtr mTransform;
private:
	vbSurfaceCube mVertexData;
	bool mVertexDirty;
	Eigen::Vector3f mPosition;
	Eigen::Vector3f mHalfSize;
	Eigen::Vector4f mColor;
};

}