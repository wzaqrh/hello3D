#include "TMesh.h"
#include "TRenderSystem.h"

/********** TMesh **********/
TMesh::TMesh(const aiMesh* __data,
	const std::vector<MeshVertex>& vertices,
	const std::vector<UINT>& indices,
	const std::vector<TTexture>& textures,
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
	mVertexBuffer = renderSys->CreateVertexBuffer(sizeof(MeshVertex) * vertices.size(), sizeof(MeshVertex), 0, &vertices[0]);
	mIndexBuffer = renderSys->CreateIndexBuffer(sizeof(UINT) * indices.size(), DXGI_FORMAT_R32_UINT, &indices[0]);
	return true;
}

void TMesh::Close()
{
}

void TMesh::Draw(TRenderSystem* renderSys)
{
	renderSys->SetVertexBuffer(mVertexBuffer);
	renderSys->SetIndexBuffer(mIndexBuffer);
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

	renderSys->DrawIndexed(mIndexBuffer);
}

bool TMesh::HasTexture(int slot)
{
	return slot < textures.size() && textures[slot].texture != nullptr;
}