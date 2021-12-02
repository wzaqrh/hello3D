#pragma once
#include <assimp/mesh.h>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/math.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/base/declare_macros.h"

namespace mir {

class MIR_CORE_API AssimpMesh
{
public:
	DECLARE_STATIC_CREATE_CONSTRUCTOR(AssimpMesh);
	AssimpMesh(Launch launchMode, ResourceManager& resourceMng,
		const aiMesh* aiMeshData,
		std::vector<vbSurface>&& surfVertexs,
		std::vector<vbSkeleton>&& skeletonVertexs,
		std::vector<uint32_t>&& indices,
		TextureBySlotPtr textures);
	bool IsLoaded() const;
	bool HasTexture(int slot) const;
	const aiMesh* GetRawMesh() const { return mAiMesh; }
	const TextureBySlotPtr& GetTextures() const { return mTextures; }
	const IVertexBufferPtr& GetVBOSurface() const { return mVBOSurface; }
	const IVertexBufferPtr& GetVBOSkeleton() const { return mVBOSkeleton; }
	const IIndexBufferPtr& GetIndexBuffer() const { return mIndexBuffer; }
private:
	const aiMesh* mAiMesh;
	std::vector<vbSurface> mSurfVertexs;
	std::vector<vbSkeleton> mSkeletonVertexs;
	std::vector<uint32_t> mIndices;
	TextureBySlotPtr mTextures;
	IVertexBufferPtr mVBOSurface, mVBOSkeleton;
	IIndexBufferPtr mIndexBuffer;
};
typedef std::shared_ptr<AssimpMesh> AssimpMeshPtr;

}