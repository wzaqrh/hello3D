#pragma once
#include "core/renderable/renderable.h"
//INCLUDE_PREDEFINE_H

struct POSTPROCESS_VERTEX
{
	XMFLOAT4 Pos;
	XMFLOAT2 Tex;
};
struct POSTPROCESS_VERTEX_QUAD {
#if _MSC_VER <= 1800
	POSTPROCESS_VERTEX lb, lt, rt, rb;
#else
	union {
		POSTPROCESS_VERTEX m[4];
		struct {
			POSTPROCESS_VERTEX lb, lt, rt, rb;
		};
	};
#endif
public:
	POSTPROCESS_VERTEX_QUAD(float x, float y, float w, float h);
	void SetRect(float x, float y, float w, float h);
	void SetFlipY(bool flipY);
	void SetZ(float z);
};
class TPostProcess : public IRenderable 
{
protected:
	IRenderSystem* mRenderSys = nullptr;
	IRenderTexturePtr mMainTex;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	TMaterialPtr mMaterial;
	std::map<std::pair<TPassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
public:
	TPostProcess(IRenderSystem* RenderSys, IRenderTexturePtr mainTex);
	~TPostProcess();
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
	void Draw();
};

struct cbBloom {
	XMFLOAT4 SampleOffsets[16];
	XMFLOAT4 SampleWeights[16];
	static cbBloom CreateDownScale2x2Offsets(int dwWidth, int dwHeight);
	static cbBloom CreateDownScale3x3Offsets(int dwWidth, int dwHeight);
	static cbBloom CreateBloomOffsets(int dwD3DTexSize, float fDeviation, float fMultiplier);
	static TConstBufferDecl& GetDesc();
};
class TBloom : public TPostProcess
{
public:
	TBloom(IRenderSystem* RenderSys, IRenderTexturePtr mainTex);
};