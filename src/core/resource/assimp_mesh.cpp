#include "core/resource/assimp_mesh.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material.h"
#include "core/base/debug.h"

namespace mir {
namespace res {

AssimpMesh::AssimpMesh()
{}

void AssimpMesh::Build(Launch launchMode, ResourceManager& resMng)
{
	mVao = resMng.CreateVertexArray(launchMode);

	mIndexBuffer = resMng.CreateIndexBuffer(__launchMode__, mVao, kFormatR32UInt, Data::Make(mIndices));
	DEBUG_SET_PRIV_DATA(mIndexBuffer, "assimp_mesh.index");

	mVBOSurface = resMng.CreateVertexBuffer(__launchMode__, mVao, sizeof(vbSurface), 0, Data::Make(mSurfVertexs));
	DEBUG_SET_PRIV_DATA(mVBOSurface, "assimp_mesh.surface");

	mVBOSkeleton = resMng.CreateVertexBuffer(__launchMode__, mVao, sizeof(vbSkeleton), 0, Data::Make(mSkeletonVertexs));
	DEBUG_SET_PRIV_DATA(mVBOSurface, "assimp_mesh.skeleton");
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