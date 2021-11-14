#pragma once
#include <assimp/mesh.h>
#include "core/mir_export.h"
#include "core/renderable/renderable.h"

namespace mir {

//#define MESH_VETREX_POSTEX
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

class MIR_CORE_API AssimpMesh : public IRenderable {
public:
	const aiMesh* Data = nullptr;
	std::vector<AssimpMeshVertex> Vertices;
	std::vector<UINT> Indices;
	TextureBySlotPtr Textures;
	IVertexBufferPtr VertexBuffer;
	IIndexBufferPtr IndexBuffer;
	MaterialPtr Material;
public:
	AssimpMesh(const aiMesh* data, 
		std::vector<AssimpMeshVertex>& vertices, 
		std::vector<UINT>& indices,
		TextureBySlotPtr textures,
		MaterialPtr material,
		IRenderSystem& renderSys);
	bool HasTexture(int slot);
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
private:
	bool setupMesh(IRenderSystem& renderSys);
};
typedef std::shared_ptr<AssimpMesh> AssimpMeshPtr;
typedef std::vector<AssimpMeshPtr> AssimpMeshPtrVector;

}