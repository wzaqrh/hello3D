#pragma once
#include "core/renderable/renderable.h"

namespace mir {

struct PostProcessVertex {
	XMFLOAT4 Pos;
	XMFLOAT2 Tex;
};
struct PostProcessVertexQuad {
#if _MSC_VER <= 1800
	PostProcessVertex lb, lt, rt, rb;
#else
	union {
		PostProcessVertex m[4];
		struct { PostProcessVertex lb, lt, rt, rb; };
	};
#endif
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