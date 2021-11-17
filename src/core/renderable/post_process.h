#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/renderable/renderable.h"

namespace mir {

struct PostProcessVertex {
	Eigen::Vector4f Pos;
	Eigen::Vector2f Tex;
};
struct PostProcessVertexQuad {
	PostProcessVertex lb, lt, rt, rb;
public:
	PostProcessVertexQuad(float x, float y, float w, float h);
	void SetRect(float x, float y, float w, float h);
	void SetFlipY(bool flipY);
	void SetZ(float z);
};
class MIR_CORE_API PostProcess : public IRenderable 
{
	friend class RenderableFactory;
protected:
	PostProcess(ResourceManager& resourceMng, IRenderTexturePtr mainTex);
public:
	~PostProcess();
	int GenRenderOperation(RenderOperationQueue& opList) override;
protected:
	ResourceManager& mResourceMng;
	IRenderTexturePtr mMainTex;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr mMaterial;
	std::map<std::pair<PassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
};

class MIR_CORE_API Bloom : public PostProcess 
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(Bloom);
	Bloom(ResourceManager& resourceMng, MaterialFactory& matFac, IRenderTexturePtr mainTex);
public:
};

}