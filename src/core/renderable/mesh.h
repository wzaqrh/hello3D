#pragma once
#include "core/renderable/renderable.h"

namespace mir {

struct MeshVertex {
	XMFLOAT3 Position;
	int Color;
	XMFLOAT2 UV;
};

class Mesh : public IRenderable 
{
	IRenderSystem& mRenderSys;
public:
	MaterialPtr Material;

	int VertPos = 0, VertDirty = false;
	std::vector<MeshVertex> Vertices;
	IVertexBufferPtr VertexBuffer;

	int IndiceDirty = false;
	std::vector<UINT> Indices;
	IIndexBufferPtr IndexBuffer;

	struct SubMesh {
		short IndicePos,IndiceCount,IndiceBase;
		TextureBySlot Textures;
	};
	std::vector<SubMesh> SubMeshs;
public:
	Mesh(IRenderSystem& renderSys, MaterialFactory& matFac, const std::string& matName, int vertCount = 1024, int indexCount = 1024);
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;

	void Clear();
	void SetVertexs(const MeshVertex* vertData, int vertCount);
	void SetVertexs(const MeshVertex* vertData, int vertCount, int vertPos);
	void SetPositions(const XMFLOAT3* posData, int count);
	void SetColors(const XMFLOAT4* colorData, int count);
	void SetUVs(const XMFLOAT2* uvData, int count);
	void SetSubMeshCount(int count);
	void SetIndices(const UINT* indiceData, int indicePos, int indiceCount, int indiceBase, int subMeshIndex);
	void SetTexture(int slot, ITexturePtr texture, int subMeshIndex);
};

}