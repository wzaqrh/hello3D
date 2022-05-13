#pragma once
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/math.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/base/uniform_struct.h"
#include "core/base/declare_macros.h"
#include "core/rendersys/texture.h"
#include "core/resource/material.h"

namespace mir {
namespace res {

struct AiNode;
struct AiBone {
	struct VertexWeight {
		unsigned int mVertexId;
		float mWeight;
	};
	std::string mName;
	std::vector<VertexWeight> mWeights;
	Eigen::Matrix4f mOffsetMatrix;
	std::weak_ptr<AiNode> mRelateNode;
};

class MIR_CORE_API AssimpMesh
{
	friend class AiSceneLoader;
	friend class AiSceneObjLoader;
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
public:
	AssimpMesh();
	void Build(Launch launchMode, ResourceManager& resourceMng);
public:
	bool HasBones() const { return mHasBones; }
	int GetMeshIndex() const { return mSceneMeshIndex; }
	const std::vector<AiBone>& GetBones() const { return mBones; }

	bool IsLoaded() const;
	void SetMaterial(const MaterialInstance& mat) { mMaterial = mat; }
	const MaterialInstance& GetMaterial() const { return mMaterial; }
	const TextureVector& GetTextures() const { return mMaterial->GetTextures(); }
	const IVertexBufferPtr& GetVBOSurface() const { return mVBOSurface; }
	const IVertexBufferPtr& GetVBOSkeleton() const { return mVBOSkeleton; }
	const IIndexBufferPtr& GetIndexBuffer() const { return mIndexBuffer; }
	const Eigen::AlignedBox3f& GetAABB() const { return mAABB; }
private:
	int mSceneMeshIndex = -1;
	bool mHasBones = false;
	Eigen::AlignedBox3f mAABB;
	std::vector<AiBone> mBones;

	vbSurfaceVector mSurfVertexs;
	vbSkeletonVector mSkeletonVertexs;
	std::vector<uint32_t> mIndices;
	
	IVertexBufferPtr mVBOSurface, mVBOSkeleton;
	IIndexBufferPtr mIndexBuffer;
	res::MaterialInstance mMaterial;
};

}
}