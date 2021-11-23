#pragma once
#include <assimp/mesh.h>
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/renderable/renderable.h"

namespace mir {

struct AssimpMeshVertex
{
	Eigen::Vector3f Pos;
	Eigen::Vector3f Normal;
	Eigen::Vector3f Tangent;
	Eigen::Vector2f Tex;
	Eigen::Vector4f BlendWeights;
	unsigned int BlendIndices[4];
	Eigen::Vector3f BiTangent;
};

class MIR_CORE_API AssimpMesh
{
public:
	DECLARE_STATIC_CREATE_CONSTRUCTOR(AssimpMesh);
	AssimpMesh(Launch launchMode, ResourceManager& resourceMng,
		const aiMesh* aiMeshData,
		std::vector<AssimpMeshVertex>&& vertices,
		std::vector<uint32_t>&& indices,
		TextureBySlotPtr textures);
	bool IsLoaded() const;
	bool HasTexture(int slot) const;
	const aiMesh* GetAiMesh() const { return mAiMesh; }
	const TextureBySlotPtr& GetTextures() const { return mTextures; }
	const IVertexBufferPtr& GetVertexBuffer() const { return mVertexBuffer; }
	const IIndexBufferPtr& GetIndexBuffer() const { return mIndexBuffer; }
private:
	const aiMesh* mAiMesh;
	std::vector<AssimpMeshVertex> mVertices;
	std::vector<uint32_t> mIndices;
	TextureBySlotPtr mTextures;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
};
typedef std::shared_ptr<AssimpMesh> AssimpMeshPtr;

}