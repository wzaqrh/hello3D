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
#if USE_MATERIAL_INSTANCE
	const MaterialInstance& GetMaterial() const { return mMaterial; }
	const TextureVector& GetTextures() const { return mMaterial->GetTextures(); }
#else
	bool HasTangent() const { return mHasTangent; }
	bool HasTexture(int slot) const;
	const Eigen::Vector4f& GetFactor(int slot) const;
	const Eigen::Vector4f& GetUvTransform(int slot) const;
	const TextureVector& GetTextures() const { return mTextures; }
#endif
	const aiMesh* GetRawMesh() const { return mAiMesh; }
	const IVertexBufferPtr& GetVBOSurface() const { return mVBOSurface; }
	const IVertexBufferPtr& GetVBOSkeleton() const { return mVBOSkeleton; }
	const IIndexBufferPtr& GetIndexBuffer() const { return mIndexBuffer; }
private:
	const aiMesh* mAiMesh;
	std::vector<vbSurface, mir_allocator<vbSurface>> mSurfVertexs;
	std::vector<vbSkeleton, mir_allocator<vbSkeleton>> mSkeletonVertexs;
	std::vector<uint32_t> mIndices;
#if USE_MATERIAL_INSTANCE
	res::MaterialInstance mMaterial;
#else
	TextureVector mTextures;
	bool mHasTangent;
	std::vector<Eigen::Vector4f> mFactors;
	std::vector<Eigen::Vector4f> mUvTransform;
#endif
	IVertexBufferPtr mVBOSurface, mVBOSkeleton;
	IIndexBufferPtr mIndexBuffer;
#if defined _DEBUG
	Eigen::Vector3f mMaxPos, mMinPos;
#endif
};

}
}