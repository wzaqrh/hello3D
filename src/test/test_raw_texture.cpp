#include <boost/filesystem.hpp>
#include "test/test_case.h"
#include "test/app.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/rendersys/interface_type.h"
#include "core/renderable/sprite.h"
#include "WINGDI.h"

using namespace mir;

class TestRawTexture : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;

	ITexturePtr LoadTexture(std::string filename);
private:
	SpritePtr mSprite;
};

ITexturePtr TestRawTexture::LoadTexture(std::string filename)
{
	std::string path = boost::filesystem::current_path().string() + "\\model\\" + filename;
	
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

	ITexturePtr texture = mContext->RenderSys()->CreateTexture(bmpInfo.biWidth, bmpInfo.biHeight, kFormatR8G8B8A8UNorm, 4);

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
	mContext->RenderSys()->LoadRawTextureData(texture, &imgBuf[0], imgBuf.size(), bmpInfo.biWidth * 4);

	fclose(fd);
	return texture;
}

void TestRawTexture::OnPostInitDevice()
{
	mContext->SceneMng()->RemoveAllCameras();
	mContext->SceneMng()->AddOthogonalCamera(Eigen::Vector3f(0,0,-10), 100);

	mSprite = mContext->RenderableFac()->CreateSprite("smile.bmp", E_MAT_SPRITE);

	mSprite->SetPosition(Eigen::Vector3f(
		-mContext->RenderSys()->WinSize().x() / 2, 
		-mContext->RenderSys()->WinSize().y() / 2, 
		0));
	mSprite->SetSize(Eigen::Vector2f(
		mContext->RenderSys()->WinSize().x(), 
		mContext->RenderSys()->WinSize().y()));
}

void TestRawTexture::OnRender()
{
	mContext->RenderPipe()->Draw(*mSprite, *mContext->SceneMng());
}

#if defined TEST_RAW_TEXTURE && TEST_CASE == TEST_RAW_TEXTURE
auto reg = AppRegister<TestRawTexture>("Sprite: RawTexture");
#endif