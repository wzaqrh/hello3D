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
};

enum enTextureType {
	E_TEXTURE_DIFFUSE,
	E_TEXTURE_SPECULAR,
	E_TEXTURE_NORMAL,
	E_TEXTURE_BUMP
};
struct TextureInfo {
	std::string path;
	ID3D11ShaderResourceView *texture;
	TextureInfo();
	TextureInfo(std::string __path, ID3D11ShaderResourceView* __texture);
};

class TRenderSystem;
class TMesh {
public:
	std::vector<MeshVertex> vertices;
	std::vector<UINT> indices;
	std::vector<TextureInfo> textures;
	const aiMesh* data = nullptr;
public:
	TMesh(const aiMesh* __data, 
		const std::vector<MeshVertex>& vertices, 
		const std::vector<UINT>& indices,
		const std::vector<TextureInfo>& textures,
		TRenderSystem *renderSys);
	void Close();
	void Draw(TRenderSystem* renderSys);
private:
	bool setupMesh(TRenderSystem *renderSys);
public:
	ID3D11Buffer *mVertexBuffer, *mIndexBuffer;
};
typedef std::shared_ptr<TMesh> TMeshSharedPtr;
typedef std::vector<TMeshSharedPtr> TMeshSharedPtrVector;
