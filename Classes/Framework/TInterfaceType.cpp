#include "TInterfaceType.h"

/********** TRenderTexture **********/
TRenderTexture::TRenderTexture(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format)
{
	mFormat = format;
	InitRenderTexture(pDevice, width, height);
	InitRenderTargetView(pDevice);
	InitRenderTextureView(pDevice);

	InitDepthStencilTexture(pDevice, width, height);
	InitDepthStencilView(pDevice);
}

//const DXGI_FORMAT CTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
bool TRenderTexture::InitRenderTexture(ID3D11Device* pDevice, int width, int height)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = mFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT result = pDevice->CreateTexture2D(&textureDesc, NULL, &mRenderTargetTexture);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool TRenderTexture::InitRenderTargetView(ID3D11Device* pDevice)
{
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
	renderTargetViewDesc.Format = mFormat;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	HRESULT result = pDevice->CreateRenderTargetView(mRenderTargetTexture, &renderTargetViewDesc, &mRenderTargetView);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool TRenderTexture::InitRenderTextureView(ID3D11Device* pDevice)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = mFormat;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	HRESULT result = pDevice->CreateShaderResourceView(mRenderTargetTexture, &shaderResourceViewDesc, &mRenderTargetSRV);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool TRenderTexture::InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	HRESULT result = pDevice->CreateTexture2D(&depthBufferDesc, NULL, &mDepthStencilTexture);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool TRenderTexture::InitDepthStencilView(ID3D11Device* pDevice)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	HRESULT result = pDevice->CreateDepthStencilView(mDepthStencilTexture, &depthStencilViewDesc, &mDepthStencilView);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

TTexture TRenderTexture::GetRenderTargetSRV()
{
	return TTexture("", mRenderTargetSRV);
}

/********** TTexture **********/
TTexture::TTexture(std::string __path, ID3D11ShaderResourceView* __texture)
{
	path = __path;
	texture = __texture;
}

TTexture::TTexture()
	:texture(nullptr)
{

}

D3D11_TEXTURE2D_DESC TTexture::GetDesc()
{
	ID3D11Texture2D* pTexture;
	texture->GetResource((ID3D11Resource **)&pTexture);
	
	D3D11_TEXTURE2D_DESC desc;
	pTexture->GetDesc(&desc);

	return desc;
}

int TTexture::GetWidth()
{
	return GetDesc().Width;
}

int TTexture::GetHeight()
{
	return GetDesc().Height;
}

DXGI_FORMAT TTexture::GetFormat()
{
	return GetDesc().Format;
}

/********** TTextureBySlot **********/
void TTextureBySlot::clear()
{
	textures.clear();
}

void TTextureBySlot::push_back(const TTexture& texture)
{
	textures.push_back(texture);
}

bool TTextureBySlot::empty() const
{
	return textures.empty();
}

size_t TTextureBySlot::size() const
{
	return textures.size();
}

void TTextureBySlot::swap(TTextureBySlot& other)
{
	textures.swap(other.textures);
}

void TTextureBySlot::resize(size_t size)
{
	textures.resize(size);
}

const TTexture& TTextureBySlot::At(size_t pos)  const {
	return textures[pos];
}
TTexture& TTextureBySlot::At(size_t pos) {
	if (pos >= textures.size()) textures.resize(pos + 1);
	return textures[pos];
}
const TTexture& TTextureBySlot::operator[](size_t pos)  const {
	return At(pos);
}
TTexture& TTextureBySlot::operator[](size_t pos) {
	return At(pos);
}

std::vector<ID3D11ShaderResourceView*> TTextureBySlot::GetTextureViews() const
{
	std::vector<ID3D11ShaderResourceView*> views(textures.size());
	for (int i = 0; i < views.size(); ++i)
		views[i] = textures[i].texture;
	return views;
}

void TTextureBySlot::Merge(const TTextureBySlot& other)
{
	if (textures.size() < other.textures.size())
		textures.resize(other.textures.size());

	for (size_t i = 0; i < other.textures.size(); ++i) {
		if (other.textures[i].texture) {
			textures[i] = other.textures[i];
		}
	}
}

/********** TIndexBuffer **********/
int TIndexBuffer::GetWidth()
{
	int width = 4;
	switch (format)
	{
	case DXGI_FORMAT_R32_UINT:
		width = 4;
		break;
	case DXGI_FORMAT_R32_SINT:
		width = 4;
		break;
	case DXGI_FORMAT_R16_UINT:
		width = 2;
		break;
	case DXGI_FORMAT_R16_SINT:
		width = 2;
		break;
	case DXGI_FORMAT_R8_UINT:
		width = 1;
		break;
	case DXGI_FORMAT_R8_SINT:
		width = 1;
		break;
	default:
		break;
	}
	return width;
}

/********** TVertexBuffer **********/
int TVertexBuffer::GetCount()
{
	return bufferSize / stride;
}

/********** TRenderOperation **********/
TRenderOperation::TRenderOperation()
{
	mWorldTransform = XMMatrixIdentity();
}

/********** TRenderOperationQueue **********/
void TRenderOperationQueue::clear()
{
	mOps.clear();
}

void TRenderOperationQueue::push_back(const TRenderOperation& op)
{
	mOps.push_back(op);
}

size_t TRenderOperationQueue::size() const
{
	return mOps.size();
}

const TRenderOperation& TRenderOperationQueue::At(size_t pos) const
{
	return mOps[pos];
}

TRenderOperation& TRenderOperationQueue::At(size_t pos)
{
	return mOps[pos];
}

const TRenderOperation& TRenderOperationQueue::operator[](size_t pos) const
{
	return At(pos);
}

TRenderOperation& TRenderOperationQueue::operator[](size_t pos)
{
	return At(pos);
}
