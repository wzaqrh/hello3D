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
	mVertexBuffer = renderSys->CreateVertexBuffer(sizeof(SimpleVertex) * vertices.size(), &vertices[0]);
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
	UINT stride = sizeof(SimpleVertex);
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

	renderSys->mDeviceContext->DrawIndexed(indices.size(), 0, 0);
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
