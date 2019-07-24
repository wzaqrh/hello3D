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

enum enTextureType {
	E_TEXTURE_DIFFUSE,
	E_TEXTURE_SPECULAR,
	E_TEXTURE_NORMAL,
	E_TEXTURE_BUMP
};
enum enTexturePbrType {
	E_TEXTURE_PBR_ALBEDO,
	E_TEXTURE_PBR_NORMAL,
	E_TEXTURE_PBR_METALNESS,
	E_TEXTURE_PBR_ROUGHNESS,
	E_TEXTURE_PBR_AO
};
struct TextureInfo {
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
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

	bool HasTexture(int slot);
private:
	bool setupMesh(TRenderSystem *renderSys);
public:
	ID3D11Buffer *mVertexBuffer, *mIndexBuffer;
};
typedef std::shared_ptr<TMesh> TMeshSharedPtr;
typedef std::vector<TMeshSharedPtr> TMeshSharedPtrVector;
