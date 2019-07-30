#include "TMesh.h"
#include "TRenderSystem.h"

/********** TMesh **********/
TMesh::TMesh(const aiMesh* __data,
	std::vector<MeshVertex>& __vertices,
	std::vector<UINT>& __indices,
	TTextureBySlot& __textures,
	TMaterialPtr __material,
	TRenderSystem *__renderSys)
{
	data = __data;
	vertices.swap(__vertices); __vertices.clear();
	indices.swap(__indices); __indices.clear();
	mTextures.swap(__textures); __textures.clear();
	mMaterial = __material;

	setupMesh(__renderSys);
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
	if (mTextures.size() > 0)
	{
		std::vector<ID3D11ShaderResourceView*> texViews(mTextures.size());
		for (int i = 0; i < mTextures.size(); ++i)
			texViews[i] = mTextures[i].texture;
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
	return slot < mTextures.size() && mTextures[slot].texture != nullptr;
}