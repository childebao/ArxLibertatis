#include "graphics/font/FontCache.h"

#include "io/Logger.h"
#include "graphics/texture/PackedTexture.h"

#include <ft2build.h>
#include FT_FREETYPE_H


FT_Library g_FTLibrary = NULL;
FontCache* FontCache::m_Instance = NULL;

void FontCache::Initialize()
{
	m_Instance = new FontCache();
}

void FontCache::Shutdown()
{
	delete m_Instance;
	m_Instance = NULL;
}

FontCache::FontCache()
{
	FT_Init_FreeType( &g_FTLibrary );
}

FontCache::~FontCache()
{
	arx_assert_msg(m_LoadedFonts.size() == 0, "Someone is probably leaking fonts!");

	FT_Done_FreeType( g_FTLibrary );
	g_FTLibrary = NULL;
}

Font* FontCache::GetFont( const std::string& fontFile, unsigned int fontSize )
{
	Font* pFont = 0;
	FontMap::iterator it = m_Instance->m_LoadedFonts.find(Font::Info(fontFile, fontSize));
	if(it == m_Instance->m_LoadedFonts.end())
	{
		pFont = m_Instance->Create(fontFile, fontSize);
		
		if(pFont)
			m_Instance->m_LoadedFonts[Font::Info(fontFile, fontSize)] = pFont;
	}
	else
	{
		pFont = (*it).second;
	}

	if(pFont)
		pFont->m_RefCount++;

	return pFont;
}

Font* FontCache::Create( const std::string& fontFile, const unsigned int fontSize )
{
	FT_Face		face;
	FT_Error    error;
	
    error = FT_New_Face( g_FTLibrary, fontFile.c_str(), 0, &face );
    if( error == FT_Err_Unknown_File_Format )
    {
        // ... the font file could be opened and read, but it appears
        // ... that its font format is unsupported
		LogError << "Font creation error: FT_Err_Unknown_File_Format";
        return NULL;
    }
    else if( error )
    {
        // ... another error code means that the font file could not
        // ... be opened or read, or simply that it is broken...
        return NULL;
    }
	
    // Windows default is 96dpi
    // Freetype default is 72dpi
	error = FT_Set_Char_Size( face, 0, fontSize*64, 64, 64 );
	if( error )
	{
		return NULL;
	}

	return new Font(fontFile, fontSize, face);
}

void FontCache::ReleaseFont( Font* pFont )
{
	if(!pFont)
		return;

	pFont->m_RefCount--;

	if(pFont->m_RefCount == 0)
	{
		FontMap::iterator it = m_Instance->m_LoadedFonts.find(pFont->GetInfo());
		m_Instance->m_LoadedFonts.erase(it);
		delete pFont;
	}
}