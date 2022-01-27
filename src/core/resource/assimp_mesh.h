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
public:
	friend class AiSceneLoader;
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(AssimpMesh);
	AssimpMesh();
	void Build(Launch launchMode, ResourceManager& resourceMng);
public:
	bool IsLoaded() const;
	const MaterialInstance& GetMaterial() const { return mMaterial; }
	const TextureVector& GetTextures() const { return mMaterial->GetTextures(); }
	const aiMesh* GetRawMesh() const { return mAiMesh; }
	const IVertexBufferPtr& GetVBOSurface() const { return mVBOSurface; }
	const IVertexBufferPtr& GetVBOSkeleton() const { return mVBOSkeleton; }
	const IIndexBufferPtr& GetIndexBuffer() const { return mIndexBuffer; }
private:
	const aiMesh* mAiMesh;
	vbSurfaceVector mSurfVertexs;
	vbSkeletonVector mSkeletonVertexs;
	std::vector<uint32_t> mIndices;
	res::MaterialInstance mMaterial;
	IVertexBufferPtr mVBOSurface, mVBOSkeleton;
	IIndexBufferPtr mIndexBuffer;
#if defined _DEBUG
	Eigen::Vector3f mMaxPos, mMinPos;
#endif
};

}
}