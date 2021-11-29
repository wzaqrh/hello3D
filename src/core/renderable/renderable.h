#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/base/stl.h"
#include "core/base/math.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/material.h"

namespace mir {

/********** RenderOperation **********/
struct RenderOperation {
	RenderOperation() 
		: IndexPos(0), IndexCount(0), IndexBase(0), WorldTransform(Eigen::Matrix4f::Identity()), CameraMask(-1) {}
	template<class T> void SetConstantBufferBytes(const std::string& cbName, const T& value) {
		CBDataByName[cbName].assign((char*)&value, (char*)&value + sizeof(value));
	}
public:
	MaterialPtr Material;
	IVertexBufferPtr VertexBuffer;
	std::map<std::pair<PassPtr, int>, IVertexBufferPtr> VertBufferByPass;
	IIndexBufferPtr IndexBuffer;
	short IndexPos, IndexCount, IndexBase;
	TextureBySlot Textures;
	Eigen::Matrix4f WorldTransform;
	typedef std::function<void(RenderSystem& renderSys, const Pass& pass, const RenderOperation& op)> PassCallback;
	PassCallback OnBind, OnUnbind;
	std::map<std::string, std::vector<char>> CBDataByName;
	unsigned CameraMask;
};

struct RenderOperationQueue {
	std::vector<RenderOperation> mOps;
public:
	void Clear() {
		mOps.clear();
	}
	void AddOP(const RenderOperation& op) {
		mOps.push_back(op);
	}
	size_t Count() const { return mOps.size(); }
	bool IsEmpty() const { return mOps.empty(); }

	RenderOperation& At(size_t pos) { return mOps[pos]; }
	RenderOperation& operator[](size_t pos) { return At(pos); }

	const RenderOperation& At(size_t pos) const { return mOps[pos]; }
	const RenderOperation& operator[](size_t pos) const { return At(pos); }
};

interface MIR_CORE_API IRenderable : boost::noncopyable 
{
	virtual ~IRenderable() {}
	virtual int GenRenderOperation(RenderOperationQueue& opList) = 0;

	void SetCameraMask(unsigned mask) { mCameraMask = mask; }
	unsigned GetCameraMask() const { return mCameraMask; }
protected:
	unsigned mCameraMask;
};

}