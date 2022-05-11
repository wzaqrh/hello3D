#pragma once
#include <assimp/mesh.h>
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

class MIR_CORE_API AssimpMesh
{
	friend class AiSceneLoader;
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
public:
	AssimpMesh();
	void Build(Launch launchMode, ResourceManager& resourceMng);
public:
	bool IsLoaded() const;
	void SetMaterial(const MaterialInstance& mat) { mMaterial = mat; }
	const MaterialInstance& GetMaterial() const { return mMaterial; }
	const TextureVector& GetTextures() const { return mMaterial->GetTextures(); }
	const aiMesh* GetRawMesh() const { return mAiMesh; }
	const IVertexBufferPtr& GetVBOSurface() const { return mVBOSurface; }
	const IVertexBufferPtr& GetVBOSkeleton() const { return mVBOSkeleton; }
	const IIndexBufferPtr& GetIndexBuffer() const { return mIndexBuffer; }
	const Eigen::AlignedBox3f& GetAABB() const { return mAABB; }
private:
	const aiMesh* mAiMesh;
	vbSurfaceVector mSurfVertexs;
	vbSkeletonVector mSkeletonVertexs;
	std::vector<uint32_t> mIndices;
	res::MaterialInstance mMaterial;
	IVertexBufferPtr mVBOSurface, mVBOSkeleton;
	IIndexBufferPtr mIndexBuffer;
	Eigen::AlignedBox3f mAABB;
};

}
}