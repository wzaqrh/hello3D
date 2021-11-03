#pragma once
//INCLUDE_PREDEFINE_H
#include "core/renderable/renderable.h"

//#define MESH_VETREX_POSTEX
struct AssimpMeshVertex
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

class TAssimpMesh : public IRenderable {
public:
	const aiMesh* Data = nullptr;
	std::vector<AssimpMeshVertex> Vertices;
	std::vector<UINT> Indices;
	TTextureBySlotPtr Textures;
	IVertexBufferPtr VertexBuffer;
	IIndexBufferPtr IndexBuffer;
	TMaterialPtr Material;
public:
	TAssimpMesh(const aiMesh* __data, 
		std::vector<AssimpMeshVertex>& __vertices, 
		std::vector<UINT>& __indices,
		TTextureBySlotPtr __textures,
		TMaterialPtr __material,
		IRenderSystem *__renderSys);
	bool HasTexture(int slot);
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
private:
	bool setupMesh(IRenderSystem *renderSys);
};
typedef std::shared_ptr<TAssimpMesh> TMeshSharedPtr;
typedef std::vector<TMeshSharedPtr> TMeshSharedPtrVector;
