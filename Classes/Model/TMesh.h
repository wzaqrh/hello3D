#pragma once
#include "TPredefine.h"

//#define MESH_VETREX_POSTEX
struct MeshVertex
{
	XMFLOAT3 Pos;
#ifndef MESH_VETREX_POSTEX
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
#endif
	XMFLOAT2 Tex;
#ifndef MESH_VETREX_POSTEX
	XMFLOAT4 BlendWeights;
	unsigned int  BlendIndices[4];
	XMFLOAT3 BiTangent;
#endif
};

class IRenderSystem;
class TMesh {
public:
	const aiMesh* data = nullptr;
	std::vector<MeshVertex> vertices;
	std::vector<UINT> indices;

	TTextureBySlotPtr mTextures;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	TMaterialPtr mMaterial;
public:
	TMesh(const aiMesh* __data, 
		std::vector<MeshVertex>& __vertices, 
		std::vector<UINT>& __indices,
		TTextureBySlotPtr __textures,
		TMaterialPtr __material,
		IRenderSystem *__renderSys);
	void Close();
	void Draw(IRenderSystem* renderSys);

	bool HasTexture(int slot);
private:
	bool setupMesh(IRenderSystem *renderSys);
};
typedef std::shared_ptr<TMesh> TMeshSharedPtr;
typedef std::vector<TMeshSharedPtr> TMeshSharedPtrVector;
