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

class MIR_CORE_API AssimpMesh : public IRenderable 
{
	friend class AssimpModel;
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(AssimpMesh);
	AssimpMesh(Launch launchMode, ResourceManager& resourceMng,
		const aiMesh* data, 
		std::vector<AssimpMeshVertex>& vertices, 
		std::vector<UINT>& indices,
		TextureBySlotPtr textures,
		MaterialPtr material);
public:
	int GenRenderOperation(RenderOperationQueue& opList) override;
	bool HasTexture(int slot);
	const aiMesh* GetAiMesh() const { return mData; }
	const MaterialPtr& GetMaterial() const { return mMaterial; }
private:
	const aiMesh* mData = nullptr;
	std::vector<AssimpMeshVertex> mVertices;
	std::vector<UINT> mIndices;
	TextureBySlotPtr mTextures;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr mMaterial;
};
typedef std::shared_ptr<AssimpMesh> AssimpMeshPtr;
typedef std::vector<AssimpMeshPtr> AssimpMeshPtrVector;

}