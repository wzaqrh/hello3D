#include "TMesh.h"

TMesh::TMesh(const aiMesh* __data, ID3D11Device *dev, std::vector<SimpleVertex> vertices, std::vector<UINT> indices, std::vector<TextureInfo> textures)
{
	data = __data;
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	this->setupMesh(dev);
}

bool TMesh::setupMesh(ID3D11Device *dev)
{
	HRESULT hr;

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &vertices[0];

	hr = dev->CreateBuffer(&vbd, &initData, &VertexBuffer);
	if (FAILED(hr))
		return false;

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	initData.pSysMem = &indices[0];

	hr = dev->CreateBuffer(&ibd, &initData, &IndexBuffer);
	if (FAILED(hr))
		return false;
}

//void TMesh::Draw(ID3D11Device *dev, ID3D11DeviceContext *devcon)
//{
//	UINT stride = sizeof(SimpleVertex);
//	UINT offset = 0;
//
//	devcon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
//	devcon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
//
//	devcon->PSSetShaderResources(0, 1, &textures[0].texture);
//
//	devcon->DrawIndexed(indices.size(), 0, 0);
//}

void TMesh::Close()
{
	VertexBuffer->Release();
	IndexBuffer->Release();
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
