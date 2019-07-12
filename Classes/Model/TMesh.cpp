#include "TMesh.h"
#include "TRenderSystem.h"

TMesh::TMesh(const aiMesh* __data,
	const std::vector<SimpleVertex>& vertices,
	const std::vector<UINT>& indices,
	const std::vector<TextureInfo>& textures,
	TRenderSystem *renderSys)
{
	data = __data;
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	this->setupMesh(renderSys);
}

bool TMesh::setupMesh(TRenderSystem *renderSys)
{
#if 0
	HRESULT hr;

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &vertices[0];

	hr = renderSys->mDevice->CreateBuffer(&vbd, &initData, &mVertexBuffer);
	if (FAILED(hr))
		return false;

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	initData.pSysMem = &indices[0];

	hr = renderSys->mDevice->CreateBuffer(&ibd, &initData, &mIndexBuffer);
	if (FAILED(hr))
		return false;
#else
	mVertexBuffer = renderSys->CreateVertexBuffer(sizeof(SimpleVertex) * vertices.size(), &vertices[0]);
	mIndexBuffer = renderSys->CreateIndexBuffer(sizeof(UINT) * indices.size(), &indices[0]);
#endif
	return true;
}

void TMesh::Close()
{
	mVertexBuffer->Release();
	mIndexBuffer->Release();
}

TextureInfo::TextureInfo(std::string __path, ID3D11ShaderResourceView* __texture)
{
	path = __path;
	texture = __texture;
}

TextureInfo::TextureInfo()
	:texture(nullptr)
{

}
