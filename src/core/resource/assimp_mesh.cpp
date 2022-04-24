#include "core/resource/assimp_mesh.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material.h"
#include "core/base/debug.h"

namespace mir {
namespace res {

AssimpMesh::AssimpMesh()
{}

void AssimpMesh::Build(Launch launchMode, ResourceManager& resourceMng)
{
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt, Data::Make(mIndices));
	DEBUG_SET_PRIV_DATA(mIndexBuffer, "assimp_mesh.index");

	mVBOSurface = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSurface), 0, Data::Make(mSurfVertexs));
	DEBUG_SET_PRIV_DATA(mVBOSurface, "assimp_mesh.surface");

	mVBOSkeleton = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSkeleton), 0, Data::Make(mSkeletonVertexs));
	DEBUG_SET_PRIV_DATA(mVBOSurface, "assimp_mesh.skeleton");

	const aiVector3D &min = mAiMesh->mAABB.mMin, &max = mAiMesh->mAABB.mMax;
	mAABB = Eigen::AlignedBox3f();
	mAABB.extend(Eigen::Vector3f(min.x, min.y, min.z));
	mAABB.extend(Eigen::Vector3f(max.x, max.y, max.z));
}

bool AssimpMesh::IsLoaded() const
{
	return mVBOSurface->IsLoaded()
		&& mVBOSkeleton->IsLoaded()
		&& mIndexBuffer->IsLoaded()
		&& mMaterial->IsLoaded();
}

}
}