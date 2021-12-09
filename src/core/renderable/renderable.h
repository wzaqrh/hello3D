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
		: IndexPos(0), IndexCount(0), IndexBase(0), WorldTransform(Eigen::Matrix4f::Identity()), CameraMask(-1) 
	{}
	template<class T> void SetUBOBytes(const std::string& cbName, const T& value) {
		UBOBytesByName[cbName].assign((char*)&value, (char*)&value + sizeof(value));
	}
	void AddVertexBuffer(IVertexBufferPtr vbo) {
		VertexBuffers.push_back(vbo);
	}
public:
	MaterialPtr Material;
	
	std::vector<IVertexBufferPtr> VertexBuffers;
	std::map<std::pair<PassPtr, int>, IVertexBufferPtr> VertBufferByPass;
	
	IIndexBufferPtr IndexBuffer;
	short IndexPos, IndexCount, IndexBase;
	
	TextureBySlot Textures;
	
	Eigen::Matrix4f WorldTransform;
	std::map<std::string, std::vector<char>> UBOBytesByName;
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
	virtual void GenRenderOperation(RenderOperationQueue& opList) = 0;
	virtual void SetCameraMask(unsigned mask) = 0;
	virtual unsigned GetCameraMask() const = 0;
};

}