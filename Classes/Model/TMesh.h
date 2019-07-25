#pragma once
#include "TBaseTypes.h"
#include "std.h"

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
	std::vector<MeshVertex> vertices;
	std::vector<UINT> indices;
	TTextureBySlot mTextures;
	const aiMesh* data = nullptr;
public:
	TMesh(const aiMesh* __data, 
		std::vector<MeshVertex>& __vertices, 
		std::vector<UINT>& __indices,
		TTextureBySlot& __textures,
		TRenderSystem *renderSys);
	void Close();
	void Draw(TRenderSystem* renderSys);

	bool HasTexture(int slot);
private:
	bool setupMesh(TRenderSystem *renderSys);
public:
	TVertexBufferPtr mVertexBuffer;
	TIndexBufferPtr mIndexBuffer;
};
typedef std::shared_ptr<TMesh> TMeshSharedPtr;
typedef std::vector<TMeshSharedPtr> TMeshSharedPtrVector;
