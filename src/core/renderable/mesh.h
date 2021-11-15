#pragma once
#include "core/mir_export.h"
#include "core/renderable/renderable.h"

namespace mir {

struct MeshVertex {
	Eigen::Vector3f Position;
	int Color;
	Eigen::Vector2f UV;
};

class MIR_CORE_API Mesh : public IRenderable 
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(Mesh);
	Mesh(IRenderSystem& renderSys, MaterialFactory& matFac, const std::string& matName, 
		int vertCount = 1024, int indexCount = 1024);
public:
	void Clear();
	void SetVertexs(const MeshVertex* vertData, int vertCount);
	void SetVertexs(const MeshVertex* vertData, int vertCount, int vertPos);
	void SetPositions(const Eigen::Vector3f* posData, int count);
	void SetColors(const Eigen::Vector4f* colorData, int count);
	void SetUVs(const Eigen::Vector2f* uvData, int count);
	void SetSubMeshCount(int count);
	void SetIndices(const unsigned int* indiceData, int indicePos, int indiceCount, int indiceBase, int subMeshIndex);
	void SetTexture(int slot, ITexturePtr texture, int subMeshIndex);
public:
	int GenRenderOperation(RenderOperationQueue& opList) override;
private:
	IRenderSystem& mRenderSys;
	MaterialPtr Material;

	int VertPos = 0, VertDirty = false;
	std::vector<MeshVertex> Vertices;
	IVertexBufferPtr VertexBuffer;

	int IndiceDirty = false;
	std::vector<unsigned int> Indices;
	IIndexBufferPtr IndexBuffer;

	struct SubMesh {
		short IndicePos, IndiceCount, IndiceBase;
		TextureBySlot Textures;
	};
	std::vector<SubMesh> SubMeshs;
};

}