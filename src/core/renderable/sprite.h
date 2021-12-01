#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable.h"

namespace mir {

struct SpriteVertex {
	Eigen::Vector3f Pos;
	unsigned int Color;
	Eigen::Vector2f Tex;
};

struct SpriteVertexQuad {
	SpriteVertexQuad();
	SpriteVertexQuad(const Eigen::Vector2f& origin, const Eigen::Vector2f& size);
	void SetCornerByRect(const Eigen::Vector2f& origin, const Eigen::Vector2f& size, float z = 0);
	void SetCornerByLBRT(const Eigen::Vector2f& pLB, const Eigen::Vector2f& pRT, float z);
	void SetCornerByVector(const Eigen::Vector3f& pLB, const Eigen::Vector3f& right, const Eigen::Vector3f& up);
	void SetZ(float z);
	void SetColor(const Eigen::Vector4f& color);
	void FlipY();
	void SetTexCoord(const Eigen::Vector2f& uv0, const Eigen::Vector2f& uv1);

	SpriteVertex& lb() { return Coners[kCubeConerFrontLeftBottom]; }
	SpriteVertex& lt() { return Coners[kCubeConerFrontLeftTop]; }
	SpriteVertex& rt() { return Coners[kCubeConerFrontRightTop]; }
	SpriteVertex& rb() { return Coners[kCubeConerFrontRightBottom]; }
private:
	void DoSetTexCoords(Eigen::Vector2f plb, Eigen::Vector2f prt);
private:
	SpriteVertex Coners[kCubeConerFrontCount];
};

class MIR_CORE_API Sprite : public IRenderable 
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(Sprite);
	Sprite(Launch launchMode, ResourceManager& resourceMng, const std::string& matName = "");
public:
	void SetPosition(const Eigen::Vector3f& pos);
	void SetSize(const Eigen::Vector2f& size);
	void SetTexture(const ITexturePtr& Texture);
	void SetColor(const Eigen::Vector4f& color);
	void SetFlipY(bool flipY);
public:
	int GenRenderOperation(RenderOperationQueue& opList) override;
	const SpriteVertexQuad* GetVertexData() const { return &mQuad; }
	const MaterialPtr& GetMaterial() const { return mMaterial; }
	const TransformPtr& GetTransform() const { return mTransform; }
private:
	ResourceManager& mResourceMng;
	ITexturePtr mTexture = nullptr;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr mMaterial;
	TransformPtr mTransform;
private:
	SpriteVertexQuad mQuad;
	bool mQuadDirty;
	Eigen::Vector2f mPosition;
	Eigen::Vector2f mSize;
	bool mFlipY;
};

}