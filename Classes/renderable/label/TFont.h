#pragma once
//INCLUDE_PREDEFINE_H
#include "TInterfaceType.h"
#include "ft2build.h"
#include "freetype.h"

class TFontTextureAtlas {
public:
	ITexturePtr Texture;
	XMFLOAT2 UV0, UV1;
	XMINT2 Pos0, Pos1;
public:
	TFontTextureAtlas(ITexturePtr texture, XMFLOAT2 uv0, XMFLOAT2 uv1, XMINT2 pos0, XMINT2 pos1) 
		:Texture(texture), UV0(uv0), UV1(uv1), Pos0(pos0), Pos1(pos1) {}
};
typedef std::shared_ptr<TFontTextureAtlas> TFontTextureAtlasPtr;

class TFontTexture {
	IRenderSystem* mRenderSys = nullptr;
	int mWidth = 0, mHeight = 0;
	ITexturePtr mTexture;
	std::vector<char> mRawBuffer;
	bool mRawBufferDirty = false;

	std::map<int, TFontTextureAtlasPtr> mAtlasByCh;
	int mCol = 0, mRow = 0, mColH = 0;
public:
	TFontTexture(IRenderSystem* renderSys, XMINT2 size);

	TFontTextureAtlasPtr GetCharactor(int ch);
	bool ContainsCharactor(int ch) const;
	TFontTextureAtlasPtr AddCharactor(int ch, int w, int h, unsigned char* buffer);
public:
	ITexturePtr GetTexture();
	int Width() const { return mWidth; }
	int Height() const { return mHeight; }
};
typedef std::shared_ptr<TFontTexture> TFontTexturePtr;

struct TFontCharactor
{
	XMINT2 Size;    
	XMINT2 Bearing;
	int Advance;
	TFontTextureAtlasPtr Atlas;
};
typedef std::shared_ptr<TFontCharactor> TFontCharactorPtr;

class TFontCharactorCache
{
	FT_Face mFtFace;
	IRenderSystem* mRenderSys = nullptr;
	std::vector<TFontTexturePtr> mFontTextures;
	TFontTexturePtr mCurFontTexture;
	std::map<int, TFontCharactorPtr> mCharactors;
public:
	TFontCharactorCache(IRenderSystem* renderSys, FT_Face ftFace);
	TFontCharactorPtr GetCharactor(int ch);
private:
	TFontTexturePtr AllocFontTexture();
};
typedef std::shared_ptr<TFontCharactorCache> TFontCharactorCachePtr;

class TFont
{
	std::string mFontName;
	int mFontSize;
	TFontCharactorCachePtr mCharactorCache;
	FT_Face mFtFace;
public:
	TFont(IRenderSystem* renderSys, FT_Library ftLib, std::string fontPath, int fontSize, int dpi = 72);
	~TFont();
	TFontCharactorPtr Get(int ch);
};
typedef std::shared_ptr<TFont> TFontPtr;

class TFontCache
{
	IRenderSystem* mRenderSys = nullptr;
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
	TFontCache(IRenderSystem* renderSys, int dpi = 72);
	~TFontCache();
	TFontPtr GetFont(std::string fontPath, int fontSize);
};
typedef std::shared_ptr<TFontCache> TFontCachePtr;