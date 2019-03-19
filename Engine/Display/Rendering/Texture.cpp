// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/stb/stb_image.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>

static const GLenum EnumToSlot[ETextureSlot::Maximum]
{
	GL_TEXTURE0,
	GL_TEXTURE1,
	GL_TEXTURE2,
	GL_TEXTURE3,
	GL_TEXTURE4,
	GL_TEXTURE5,
	GL_TEXTURE6,
	GL_TEXTURE7,
	GL_TEXTURE8,
	GL_TEXTURE9,
	GL_TEXTURE10,
	GL_TEXTURE11,
	GL_TEXTURE12,
	GL_TEXTURE13,
	GL_TEXTURE14,
	GL_TEXTURE15,
	GL_TEXTURE16,
	GL_TEXTURE17,
	GL_TEXTURE18,
	GL_TEXTURE19,
	GL_TEXTURE20,
	GL_TEXTURE21,
	GL_TEXTURE22,
	GL_TEXTURE23,
	GL_TEXTURE24,
	GL_TEXTURE25,
	GL_TEXTURE26,
	GL_TEXTURE27,
	GL_TEXTURE28,
	GL_TEXTURE29,
	GL_TEXTURE30,
	GL_TEXTURE31
};

CTexture::CTexture( const char* FileLocation )
{
	Location = FileLocation;

	Handle = 0;
	Width = 0;
	Height = 0;
	Channels = 0;

	ImageData = nullptr;
}

CTexture::~CTexture()
{
	if( ImageData )
	{
		stbi_image_free( ImageData );
	}
}

bool CTexture::Load()
{
	CFile TextureSource( Location.c_str() );
	const std::string Extension = TextureSource.Extension();

	// The STB header supports more than these types but we want to refrain from loading them since they're generally inefficient to load.
	const bool Supported = Extension == "jpg" || Extension == "png" || Extension == "tga" || Extension == "hdr";

	if( Supported && TextureSource.Load( true ) )
	{
		glGenTextures( 1, &Handle );
		glBindTexture( GL_TEXTURE_2D, Handle );

		// Wrapping parameters
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

		// Filtering parameters
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		stbi_set_flip_vertically_on_load( 1 );
		ImageData = stbi_load_from_memory( TextureSource.Fetch<stbi_uc>(), static_cast<int>( TextureSource.Size() ), &Width, &Height, &Channels, 3 );
		if( ImageData && ImageData[0] != '\0' )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData );
			glGenerateMipmap( GL_TEXTURE_2D );

			return true;
		}
		else
		{
			Log::Event( Log::Warning, "Invalid image data (\"%s\") (\"%s\").\n", Location.c_str(), stbi_failure_reason() );
		}
	}
	else
	{
		Log::Event( Log::Error, "Failed to load texture (\"%s\").\n", Location.c_str() );
	}

	return false;
}

void CTexture::Bind( ETextureSlot Slot )
{
	if( Handle )
	{
		const auto Index = static_cast<std::underlying_type<ETextureSlot>::type>( Slot );
		glActiveTexture( EnumToSlot[Index] );
		glBindTexture( GL_TEXTURE_2D, Handle );
	}
}
