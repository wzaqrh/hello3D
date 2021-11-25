#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
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
	PostProcess(Launch launchMode, ResourceManager& resourceMng, IFrameBufferPtr mainTex);
public:
	~PostProcess();
	int GenRenderOperation(RenderOperationQueue& opList) override;
protected:
	ResourceManager& mResourceMng;
	IFrameBufferPtr mMainTex;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr mMaterial;
	std::map<std::pair<PassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
};

struct cbBloom {
	Eigen::Vector4f SampleOffsets[16];
	Eigen::Vector4f SampleWeights[16];
public:
	static cbBloom CreateDownScale2x2Offsets(int dwWidth, int dwHeight);
	static cbBloom CreateDownScale3x3Offsets(int dwWidth, int dwHeight);
	static cbBloom CreateBloomOffsets(int dwD3DTexSize, float fDeviation, float fMultiplier);
};
class MIR_CORE_API Bloom : public PostProcess 
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(Bloom);
	Bloom(Launch launchMode, ResourceManager& resourceMng, IFrameBufferPtr mainTex);
public:
};

}