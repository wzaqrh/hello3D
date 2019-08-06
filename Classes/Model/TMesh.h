#pragma once
#include "TPredefine.h"

struct MeshVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT2 Tex;
	XMFLOAT4 BlendWeights;
	unsigned int  BlendIndices[4];
	XMFLOAT3 BiTangent;
};

class TRenderSystem;
class TMesh {
public:
	const aiMesh* data = nullptr;
	std::vector<MeshVertex> vertices;
	std::vector<UINT> indices;

	TTextureBySlotPtr mTextures;
	TVertexBufferPtr mVertexBuffer;
	TIndexBufferPtr mIndexBuffer;
	TMaterialPtr mMaterial;
public:
	TMesh(const aiMesh* __data, 
		std::vector<MeshVertex>& __vertices, 
		std::vector<UINT>& __indices,
		TTextureBySlotPtr __textures,
		TMaterialPtr __material,
		TRenderSystem *__renderSys);
	void Close();
	void Draw(TRenderSystem* renderSys);

	bool HasTexture(int slot);
private:
	bool setupMesh(TRenderSystem *renderSys);
};
typedef std::shared_ptr<TMesh> TMeshSharedPtr;
typedef std::vector<TMeshSharedPtr> TMeshSharedPtrVector;
