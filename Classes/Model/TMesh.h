#pragma once
#include <assimp/mesh.h>
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

#include <memory>
#include <string>
#include <vector>

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 BlendWeights;
	unsigned int  BlendIndices[4];
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
	std::vector<SimpleVertex> vertices;
	std::vector<UINT> indices;
	std::vector<TextureInfo> textures;
	const aiMesh* data = nullptr;
public:
	TMesh(const aiMesh* __data, 
		const std::vector<SimpleVertex>& vertices, 
		const std::vector<UINT>& indices,
		const std::vector<TextureInfo>& textures,
		TRenderSystem *renderSys);
	void Close();
private:
	bool setupMesh(TRenderSystem *renderSys);
public:
	ID3D11Buffer *mVertexBuffer, *mIndexBuffer;
};
typedef std::shared_ptr<TMesh> TMeshSharedPtr;
typedef std::vector<TMeshSharedPtr> TMeshSharedPtrVector;
