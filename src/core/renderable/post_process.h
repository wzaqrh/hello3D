#pragma once
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
class PostProcess : public IRenderable 
{
protected:
	IRenderSystem& mRenderSys;
	IRenderTexturePtr mMainTex;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr mMaterial;
	std::map<std::pair<PassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
public:
	PostProcess(IRenderSystem& RenderSys, IRenderTexturePtr mainTex);
	~PostProcess();
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
};

class Bloom : public PostProcess 
{
public:
	Bloom(IRenderSystem& renderSys, MaterialFactory& matFac, IRenderTexturePtr mainTex);
};

}