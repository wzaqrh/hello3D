#pragma once
//INCLUDE_PREDEFINE_H
#include "core/rendersys/interface_type.h"
#include "ft2build.h"
#include "freetype.h"
#include "freetype2/freetype/ftglyph.h"

namespace mir {

class FontTextureAtlas 
{
public:
	ITexturePtr Texture;
	XMFLOAT2 UV0, UV1;
	XMINT2 Pos0, Pos1;
public:
	FontTextureAtlas(ITexturePtr texture, XMFLOAT2 uv0, XMFLOAT2 uv1, XMINT2 pos0, XMINT2 pos1) 
		: Texture(texture), UV0(uv0), UV1(uv1), Pos0(pos0), Pos1(pos1) {}
};
typedef std::shared_ptr<FontTextureAtlas> FontTextureAtlasPtr;

class FontTexture 
{
	IRenderSystem& mRenderSys;
	int mWidth = 0, mHeight = 0;
	ITexturePtr mTexture;
	std::vector<char> mRawBuffer;
	bool mRawBufferDirty = false;

	std::map<int, FontTextureAtlasPtr> mAtlasByCh;
	int mCol = 0, mRow = 0, mColH = 0;
public:
	FontTexture(IRenderSystem& renderSys, XMINT2 size);

	FontTextureAtlasPtr GetCharactor(int ch);
	bool ContainsCharactor(int ch) const;
	FontTextureAtlasPtr AddCharactor(int ch, int w, int h, unsigned char* buffer);
public:
	ITexturePtr GetTexture();
	int Width() const { return mWidth; }
	int Height() const { return mHeight; }
};
typedef std::shared_ptr<FontTexture> FontTexturePtr;

struct FontCharactor
{
	FT_UInt charIndex;
	FT_Glyph glyph;
	FT_BBox bbox;	//dot space

	XMINT2 Size;	//dot space   
	XMINT2 Bearing;	//dot space
	int Advance;	//dot space
	FontTextureAtlasPtr Atlas;
};
typedef std::shared_ptr<FontCharactor> FontCharactorPtr;

class FontCharactorCache 
{
	IRenderSystem& mRenderSys;
	FT_Face mFtFace;
	std::vector<FontTexturePtr> mFontTextures;
	FontTexturePtr mCurFontTexture;
	std::map<int, FontCharactorPtr> mCharactors;
public:
	FontCharactorCache(IRenderSystem& renderSys, FT_Face ftFace);
	FontCharactorPtr GetCharactor(int ch);
	void FlushChange();
private:
	FontTexturePtr AllocFontTexture();
};
typedef std::shared_ptr<FontCharactorCache> FontCharactorCachePtr;

class Font 
{
	std::string mFontName;
	int mFontSize;
	FontCharactorCachePtr mCharactorCache;
	FT_Face mFtFace;
public:
	Font(IRenderSystem& renderSys, FT_Library ftLib, std::string fontPath, int fontSize, int dpi = 72);
	~Font();
public:
	FT_Face GetFtFace() { return mFtFace; }
	FontCharactorPtr GetCharactor(int ch);
	void Flush();
};
typedef std::shared_ptr<Font> TFontPtr;

class FontCache
{
	IRenderSystem& mRenderSys;
	FT_Library mFtLib;
	int mDPI;
	struct FontKey {
		std::string path;
		int size;
		bool operator<(const FontKey& other) const {
			if (path != other.path) return path < other.path;
			return size < other.size;
		}
	};
	std::map<FontKey, TFontPtr> mFonts;
public:
	FontCache(IRenderSystem& renderSys, int dpi = 72);
	~FontCache();
	TFontPtr GetFont(std::string fontPath, int fontSize);
};
typedef std::shared_ptr<FontCache> FontCachePtr;

}