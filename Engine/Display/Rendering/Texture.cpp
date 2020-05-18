// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <ThirdParty/stb/stb_image_write.h>

#include <Engine/Display/Rendering/TextureEnumeratorsGL.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>

CTexture::CTexture()
{
	Location = "";

	Handle = 0;
	Width = 0;
	Height = 0;
	Channels = 0;

	Format = EImageFormat::Unknown;

	ImageData8 = nullptr;
	ImageData16 = nullptr;
	ImageData32 = nullptr;
	ImageData32F = nullptr;

	FilteringMode = EFilteringMode::Linear;
}

CTexture::CTexture( const char* FileLocation ) : CTexture()
{
	Location = FileLocation;
}

CTexture::~CTexture()
{
	auto ImageData = GetImageData();
	if( ImageData )
	{
		stbi_image_free( ImageData );
	}

	glDeleteTextures( 1, &Handle );
}

bool CTexture::Load( const EFilteringMode Mode, const EImageFormat PreferredFormat )
{
	CFile TextureSource( Location.c_str() );
	const std::string Extension = TextureSource.Extension();

	// The STB header supports more than these types but we want to refrain from loading them since they're generally inefficient to load.
	const bool Supported = Extension == "jpg" || Extension == "png" || Extension == "tga" || Extension == "hdr";

	if( Supported && TextureSource.Load( true ) )
	{
		stbi_set_flip_vertically_on_load( 1 );

		if( PreferredFormat > EImageFormat::RGBA16 )
		{
			ImageData32F = stbi_loadf_from_memory( TextureSource.Fetch<stbi_uc>(), static_cast<int>( TextureSource.Size() ), &Width, &Height, &Channels, 0 );
		}
		else if( PreferredFormat > EImageFormat::RGBA8 )
		{
			ImageData16 = stbi_load_16_from_memory( TextureSource.Fetch<stbi_uc>(), static_cast<int>( TextureSource.Size() ), &Width, &Height, &Channels, 0 );
		}
		else
		{
			ImageData8 = stbi_load_from_memory( TextureSource.Fetch<stbi_uc>(), static_cast<int>( TextureSource.Size() ), &Width, &Height, &Channels, 0 );
		}

		FilteringMode = Mode;

		if( Load( nullptr, Width, Height, Channels, FilteringMode, PreferredFormat ) )
		{
			return true;
		}
		else
		{
			Log::Event( Log::Warning, "Invalid image data (\"%s\") (\"%s\").\n", Location.c_str(), stbi_failure_reason() );
		}
	}
	else
	{
		Log::Event( Log::Warning, "Failed to load texture (\"%s\").\n", Location.c_str() );
	}

	return false;
}

bool CTexture::Load( unsigned char* Data, const int WidthIn, const int HeightIn, const int ChannelsIn, const EFilteringMode ModeIn, const EImageFormat PreferredFormatIn )
{
	bool Supported = false;

	const void* Pixels = Data;

	if( Pixels )
	{
		// Delete old data.
		auto ImageData = GetImageData();
		if( ImageData )
		{
			stbi_image_free( ImageData );
		}

		// Assume input data is 8-bit.
		ImageData8 = Data;
	}

	if( !Pixels )
	{
		Pixels = GetImageData();
	}

	if( !Pixels || WidthIn < 1 || HeightIn < 1 || ChannelsIn < 1 )
		return false;

	Width = WidthIn;
	Height = HeightIn;
	Channels = ChannelsIn;
	FilteringMode = ModeIn;
	Format = PreferredFormatIn;

	if( Handle == 0 )
	{
		glGenTextures( 1, &Handle );
	}

	if( Handle == 0 )
	{
		return false;
	}

	glBindTexture( GL_TEXTURE_2D, Handle );

	// Wrapping parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	// Filtering parameters
	const auto Mode = static_cast<EFilteringModeType>( FilteringMode );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetFilteringMipMapMode( Mode ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetFilteringMode( Mode ) );

	const auto ImageFormat = static_cast<EImageFormatType>( PreferredFormatIn );
	// auto Format = ImageFormatToFormat[ImageFormat];
	auto Type = GetImageFormatType( ImageFormat );
	auto InternalFormat = ::GetImageFormat( ImageFormat );

	const bool PowerOfTwoWidth = ( Width & ( Width - 1 ) ) == 0;
	const bool PowerOfTwoHeight = ( Height & ( Height - 1 ) ) == 0;
	if( PowerOfTwoWidth && PowerOfTwoHeight )
	{
		if( Channels == 1 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, GL_RED, Type, Pixels );
			Supported = true;
		}
		else if( Channels == 2 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, GL_RG, Type, Pixels );
			Supported = true;
		}
		else if( Channels == 3 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, GL_RGB, Type, Pixels );
			Supported = true;
		}
		else if( Channels == 4 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, GL_RGBA, Type, Pixels );
			Supported = true;
		}
	}
	else
	{
		Log::Event( Log::Warning, "Not a power of two texture (\"%s\").\n", Location.c_str() );
	}

	if( Supported )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	return Supported;
}

void CTexture::Save( const char* FileLocation )
{
	auto Data = GetImageData();
	if( Data )
	{
		std::string ExportLocation = FileLocation ? FileLocation : Location;
		ExportLocation += ".tga";

		stbi_write_tga( ExportLocation.c_str(), Width, Height, Channels, Data );
	}
}

void CTexture::Bind( ETextureSlot Slot )
{
	if( Handle )
	{
		const auto Index = static_cast<std::underlying_type<ETextureSlot>::type>( Slot );
		glActiveTexture( GetTextureSlot( Index ) );
		glBindTexture( GL_TEXTURE_2D, Handle );
	}
}

const std::string& CTexture::GetLocation() const
{
	return Location;
}

const GLuint CTexture::GetHandle() const
{
	return Handle;
}

const int CTexture::GetWidth() const
{
	return Width;
}

const int CTexture::GetHeight() const
{
	return Height;
}

const EImageFormat CTexture::GetImageFormat() const
{
	return Format;
}

void* CTexture::GetImageData() const
{
	if( ImageData8 )
		return ImageData8;
	else if( ImageData16 )
		return ImageData16;
	else if( ImageData32 )
		return ImageData32;
	else if( ImageData32F )
		return ImageData32F;

	return nullptr;
}
