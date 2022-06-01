#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/base/stl.h"
#include "core/base/math.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/material.h"
#include "core/scene/component.h"

namespace mir {

struct RenderOperation 
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	RenderOperation() {}
	void AddVertexBuffer(IVertexBufferPtr vbo) { VertexBuffers.push_back(vbo); }
	res::MaterialInstance& WrMaterial() const { return const_cast<res::MaterialInstance&>(Material); }
public:
	Eigen::Matrix4f WorldTransform = Eigen::Matrix4f::Identity();
	unsigned CameraMask = (unsigned)-1;
	bool CastShadow = true;

	std::vector<IVertexBufferPtr> VertexBuffers;
	IIndexBufferPtr IndexBuffer;
	short IndexPos = 0, IndexCount = 0, IndexBase = 0;
	res::MaterialInstance Material;
};

class RenderOperationQueue 
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void Clear() { mOps.clear(); }
	void AddOP(const RenderOperation& op) { mOps.push_back(op); }

	std::vector<RenderOperation>::const_iterator begin() const { return mOps.begin(); }
	std::vector<RenderOperation>::const_iterator end() const { return mOps.end(); }
	std::vector<RenderOperation>::iterator begin() { return mOps.begin(); }
	std::vector<RenderOperation>::iterator end() { return mOps.end(); }

	bool IsEmpty() const { return mOps.empty(); }
	size_t Count() const { return mOps.size(); }
	
	RenderOperation& At(size_t pos) { return mOps[pos]; }
	const RenderOperation& At(size_t pos) const { return mOps[pos]; }
	RenderOperation& operator[](size_t pos) { return At(pos); }
	const RenderOperation& operator[](size_t pos) const { return At(pos); }

	RenderOperation& Back() { return mOps.back(); }
private:
	std::vector<RenderOperation, mir_allocator<RenderOperation>> mOps;
};

interface MIR_CORE_API Renderable : public Component
{
	virtual CoTask<void> UpdateFrame(float dt) { CoReturn; }
	virtual void GenRenderOperation(RenderOperationQueue& ops) = 0;
	virtual Eigen::AlignedBox3f GetWorldAABB() const = 0;

	void SetCastShadow(bool castShadow) { mCastShadow = castShadow; }
	bool IsCastShadow() const { return mCastShadow; }

	void SetCameraMask(unsigned mask) { mCameraMask = mask; }
	unsigned GetCameraMask() const { return mCameraMask; }
protected:
	bool mCastShadow = true;
	unsigned mCameraMask = -1;
};

class RenderableCollection
{
public:
	void Clear() { mRenderables.clear(); }
	void AddRenderable(const RenderablePtr& renderable) { mRenderables.push_back(renderable); }

	std::vector<RenderablePtr>::const_iterator begin() const { return mRenderables.begin(); }
	std::vector<RenderablePtr>::const_iterator end() const { return mRenderables.end(); }
	std::vector<RenderablePtr>::iterator begin() { return mRenderables.begin(); }
	std::vector<RenderablePtr>::iterator end() { return mRenderables.end(); }

	bool IsEmpty() const { return mRenderables.empty(); }
	size_t Count() const { return mRenderables.size(); }

	RenderablePtr& At(size_t pos) { return mRenderables[pos]; }
	const RenderablePtr& At(size_t pos) const { return mRenderables[pos]; }
	RenderablePtr& operator[](size_t pos) { return At(pos); }
	const RenderablePtr& operator[](size_t pos) const { return At(pos); }

	RenderablePtr& Back() { return mRenderables.back(); }
private:
	std::vector<RenderablePtr> mRenderables;
};

}