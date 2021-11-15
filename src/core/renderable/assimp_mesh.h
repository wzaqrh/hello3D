#pragma once
#include <assimp/mesh.h>
#include "core/mir_export.h"
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
	AssimpMesh(const aiMesh* data, 
		std::vector<AssimpMeshVertex>& vertices, 
		std::vector<UINT>& indices,
		TextureBySlotPtr textures,
		MaterialPtr material,
		IRenderSystem& renderSys);
public:
	int GenRenderOperation(RenderOperationQueue& opList) override;
	bool HasTexture(int slot);
	const aiMesh* GetAiMesh() const { return Data; }
	const MaterialPtr& GetMaterial() const { return Material; }
private:
	bool setupMesh(IRenderSystem& renderSys);
private:
	const aiMesh* Data = nullptr;
	std::vector<AssimpMeshVertex> Vertices;
	std::vector<UINT> Indices;
	TextureBySlotPtr Textures;
	IVertexBufferPtr VertexBuffer;
	IIndexBufferPtr IndexBuffer;
	MaterialPtr Material;
};
typedef std::shared_ptr<AssimpMesh> AssimpMeshPtr;
typedef std::vector<AssimpMeshPtr> AssimpMeshPtrVector;

}