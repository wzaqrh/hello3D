#include "core/renderable/font.h"
#include "core/rendersys/render_system.h"

namespace mir {

/********** FontTexture **********/
FontTexture::FontTexture(IRenderSystem& renderSys, Eigen::Vector2i size)
	:mRenderSys(renderSys)
{
	mWidth = size.x();
	mHeight = size.y();
	mTexture = mRenderSys.CreateTexture(mWidth, mHeight, DXGI_FORMAT_R8_UNORM, 1);
	mRawBuffer.resize(mWidth * mHeight);
}

FontTextureAtlasPtr FontTexture::GetCharactor(int ch)
{
	FontTextureAtlasPtr atlas = nullptr;
	auto iter = mAtlasByCh.find(ch);
	if (iter != mAtlasByCh.end())
		atlas = iter->second;
	return atlas;
}

bool FontTexture::ContainsCharactor(int ch) const
{
	return mAtlasByCh.find(ch) != mAtlasByCh.end();
}

FontTextureAtlasPtr FontTexture::AddCharactor(int ch, int w, int h, unsigned char* buffer)
{
	if (mCol + w > mWidth) {
		mRow += mColH;
		mCol = 0;
		mColH = 0;

		if (mCol + w > mWidth) return nullptr;
	}
	if (h >= mColH) {
		if (mRow + h > mHeight) return nullptr;
		mColH = h;
	}

	Eigen::Vector2i pos0(mCol, mRow), pos1(mCol + w, mRow + h);
	Eigen::Vector2f uv0(pos0.x() * 1.0f / mWidth, pos0.y() * 1.0f / mHeight);
	Eigen::Vector2f uv1(pos1.x() * 1.0f / mWidth, pos1.y() * 1.0f / mHeight);
	FontTextureAtlasPtr atlas = std::make_shared<FontTextureAtlas>(mTexture, uv0, uv1, pos0, pos1);

	for (int y = 0; y < h; ++y)
		memcpy(&mRawBuffer[(pos0.y() + y) * mWidth + pos0.x()], buffer + y * w, w);
	mRawBufferDirty = true;

	mCol += w;
	mAtlasByCh.insert(std::make_pair(ch, atlas));

	return atlas;
}

ITexturePtr FontTexture::GetTexture()
{
	if (mRawBufferDirty) {
		mRawBufferDirty = false;
		mRenderSys.LoadRawTextureData(mTexture, &mRawBuffer[0], mRawBuffer.size(), mWidth);
	}
	return mTexture;
}

/********** FontCharactorCache **********/
FontCharactorCache::FontCharactorCache(IRenderSystem& renderSys, FT_Face ftFace)
	:mRenderSys(renderSys)
{
	mFtFace = ftFace;
	AllocFontTexture();
}

FontTexturePtr FontCharactorCache::AllocFontTexture()
{
	Eigen::Vector2i size = { 512,512 };
	mCurFontTexture = std::make_shared<FontTexture>(mRenderSys, size);
	mFontTextures.push_back(mCurFontTexture);
	return mCurFontTexture;
}

bool CheckError(int error) {
	assert(error == 0);
	return error == 0;
}

FontCharactorPtr FontCharactorCache::GetCharactor(int ch)
{
	FontCharactorPtr charactor = nullptr;
	auto iter = mCharactors.find(ch);
	if (iter != mCharactors.end()) {
		charactor = iter->second;
	}
	else { 
		do {
			FT_UInt charIndex = FT_Get_Char_Index(mFtFace, ch); if (charIndex == 0) break;
			if (!CheckError(FT_Load_Glyph(mFtFace, charIndex, FT_LOAD_DEFAULT))) break;
			if (!CheckError(FT_Render_Glyph(mFtFace->glyph, FT_RENDER_MODE_NORMAL))) break;
			FT_Glyph glyph; if (!CheckError(FT_Get_Glyph(mFtFace->glyph, &glyph))) break;

			charactor = std::make_shared<FontCharactor>();
			charactor->CharIndex = charIndex;
			charactor->Glyph = glyph;
			
			FT_Glyph_Get_CBox(charactor->Glyph, ft_glyph_bbox_pixels, &charactor->BBox);
			charactor->Size = { (int)mFtFace->glyph->bitmap.width, (int)mFtFace->glyph->bitmap.rows };
			charactor->Bearing = { mFtFace->glyph->bitmap_left, mFtFace->glyph->bitmap_top };
			charactor->Advance = mFtFace->glyph->advance.x >> 6;

			charactor->Atlas = mCurFontTexture->AddCharactor(ch, charactor->Size.x(), charactor->Size.y(), mFtFace->glyph->bitmap.buffer);
		} while (0);
		mCharactors.insert(std::make_pair(ch, charactor));
	}
	return charactor;
}

void FontCharactorCache::FlushChange()
{
	mCurFontTexture->GetTexture();
}

/********** Font **********/
Font::Font(IRenderSystem& renderSys, FT_Library ftLib, std::string fontPath, int fontSize, int dpi)
{
	mFontName = fontPath;
	mFontSize = fontSize;
	if (!CheckError(FT_New_Face(ftLib, fontPath.c_str(), 0, &mFtFace))) return;
	if (!CheckError(FT_Set_Char_Size(mFtFace, fontSize * 64, 0, dpi, 0))) return;
	if (!CheckError(FT_Select_Charmap(mFtFace, FT_ENCODING_UNICODE))) return;

	mCharactorCache = std::make_shared<FontCharactorCache>(renderSys, mFtFace);
}
Font::~Font()
{
	FT_Done_Face(mFtFace);
}

void Font::Flush()
{
	mCharactorCache->FlushChange();
}
FontCharactorPtr Font::GetCharactor(int ch) 
{
	return mCharactorCache->GetCharactor(ch);
}

/********** FontCache **********/
FontCache::FontCache(IRenderSystem& renderSys, int dpi)
	:mRenderSys(renderSys)
{
	mDPI = dpi;
	if (FT_Init_FreeType(&mFtLib)) {
		//std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		assert(false);
		return;
	}
}

FontCache::~FontCache()
{
	FT_Done_FreeType(mFtLib);
}

TFontPtr FontCache::GetFont(std::string fontPath, int fontSize)
{
	TFontPtr font = nullptr;
	FontKey key = { fontPath, fontSize };
	auto iter = mFonts.find(key);
	if (iter != mFonts.end()) {
		font = iter->second;
	}
	else {
		font = std::make_shared<Font>(mRenderSys, mFtLib, fontPath, fontSize, mDPI);
		mFonts.insert(std::make_pair(key, font));
	}
	return font;
}

}