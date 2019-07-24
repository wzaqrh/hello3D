#include "TMesh.h"
#include "TRenderSystem.h"

/********** TextureInfo **********/
TextureInfo::TextureInfo(std::string __path, ID3D11ShaderResourceView* __texture)
{
	path = __path;
	texture = __texture;
}

TextureInfo::TextureInfo()
	:texture(nullptr)
{

}

/********** TMesh **********/
TMesh::TMesh(const aiMesh* __data,
	const std::vector<MeshVertex>& vertices,
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
	mVertexBuffer = renderSys->CreateVertexBuffer(sizeof(MeshVertex) * vertices.size(), &vertices[0]);
	mIndexBuffer = renderSys->CreateIndexBuffer(sizeof(UINT) * indices.size(), &indices[0]);
	return true;
}

void TMesh::Close()
{
	mVertexBuffer->Release();
	mIndexBuffer->Release();
}

void TMesh::Draw(TRenderSystem* renderSys)
{
	UINT stride = sizeof(MeshVertex);
	UINT offset = 0;
	renderSys->mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	renderSys->mDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	if (textures.size() > 0)
	{
		std::vector<ID3D11ShaderResourceView*> texViews(textures.size());
		for (int i = 0; i < textures.size(); ++i)
			texViews[i] = textures[i].texture;
		renderSys->mDeviceContext->PSSetShaderResources(0, texViews.size(), &texViews[0]);
	}
	else {
		ID3D11ShaderResourceView* texViewNull = nullptr;
		renderSys->mDeviceContext->PSSetShaderResources(0, 1, &texViewNull);
	}

	renderSys->mDeviceContext->DrawIndexed(indices.size(), 0, 0);
}

bool TMesh::HasTexture(int slot)
{
	return slot < textures.size() && textures[slot].texture != nullptr;
}