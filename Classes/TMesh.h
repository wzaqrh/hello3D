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

class TMesh {
public:
	std::vector<SimpleVertex> vertices;
	std::vector<UINT> indices;
	std::vector<TextureInfo> textures;
	const aiMesh* data = nullptr;
public:
	TMesh(const aiMesh* __data, ID3D11Device *dev, std::vector<SimpleVertex> vertices, std::vector<UINT> indices, std::vector<TextureInfo> textures);
	void Close();
private:
	bool setupMesh(ID3D11Device *dev);
public:
	ID3D11Buffer *VertexBuffer, *IndexBuffer;
};
typedef std::shared_ptr<TMesh> TMeshSharedPtr;
typedef std::vector<TMeshSharedPtr> TMeshSharedPtrVector;
