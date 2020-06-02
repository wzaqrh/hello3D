#include "TApp.h"
#include "TInterfaceType.h"
#include "TSprite.h"
#include "Utility.h"
#include "WINGDI.h"

class TestRawTexture : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;

	ITexturePtr LoadTexture(std::string filename);
private:
	TSpritePtr mSprite;
};

ITexturePtr TestRawTexture::LoadTexture(std::string filename)
{
	std::string path = GetCurDirectory() + "\\model\\" + filename;
	
	FILE* fd = fopen(path.c_str(), "rb");
	assert(fd);
	fseek(fd, 0, SEEK_END);
	int length = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER bmpInfo;
	if (length < sizeof(bmpHeader) + sizeof(bmpInfo)) return nullptr;

	std::vector<char> buffer;
	buffer.resize(length);
	fread(&buffer[0], 1, length, fd);

	int position = 0;
	memcpy(&bmpHeader, &buffer[position], sizeof(bmpHeader));
	position += sizeof(bmpHeader);

	memcpy(&bmpInfo, &buffer[position], sizeof(bmpInfo));
	position += sizeof(bmpInfo);

	if (bmpInfo.biClrImportant != 0) return nullptr;

	ITexturePtr texture = mRenderSys->CreateTexture(bmpInfo.biWidth, bmpInfo.biHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 4);

	std::vector<char> imgBuf;
	imgBuf.resize(bmpInfo.biWidth * bmpInfo.biHeight * 4);

	int src = position, dst = 0;
	for (int y = 0; y < bmpInfo.biHeight; ++y)
	{
		for (int x = 0; x < bmpInfo.biWidth; ++x)
		{
			imgBuf[dst + 0] = buffer[src + 2];
			imgBuf[dst + 1] = buffer[src + 1];
			imgBuf[dst + 2] = buffer[src + 0];
			imgBuf[dst + 3] = 0xff;
			src += 3;
			dst += 4;
		}
	}
	mRenderSys->LoadRawTextureData(texture, &imgBuf[0], imgBuf.size(), bmpInfo.biWidth * 4);

	fclose(fd);
	return texture;
}

void TestRawTexture::OnPostInitDevice()
{
	mRenderSys->SetOthogonalCamera(100);

	mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_SPRITE);
	mSprite->SetTexture(LoadTexture("smile.bmp"));

	mSprite->SetPosition(-mRenderSys->GetWinSize().x / 2, -mRenderSys->GetWinSize().y / 2, 0);
	mSprite->SetSize(mRenderSys->GetWinSize().x, mRenderSys->GetWinSize().y);
}

void TestRawTexture::OnRender()
{
	if (mRenderSys->BeginScene()) {
		mRenderSys->Draw(mSprite.get());
		mRenderSys->EndScene();
	}
}

//auto reg = AppRegister<TestRawTexture>("Sprite: RawTexture");