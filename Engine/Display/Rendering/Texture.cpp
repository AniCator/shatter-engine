// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/stb/stb_image.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>

static const GLenum SlotToEnum[static_cast<ETextureSlotType>( ETextureSlot::Maximum )]
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

static const GLenum FilteringModeToEnum[static_cast<EFilteringModeType>( EFilteringMode::Maximum )]
{
	GL_NEAREST,
	GL_LINEAR
};

static const GLenum ImageFormatToType[static_cast<EImageFormatType>( EImageFormat::Maximum )]
{
	GL_UNSIGNED_BYTE,

	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,

	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,

	GL_UNSIGNED_INT,
	GL_UNSIGNED_INT,
	GL_UNSIGNED_INT,
	GL_UNSIGNED_INT,

	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT
};

static const GLenum ImageFormatToFormat[static_cast<EImageFormatType>( EImageFormat::Maximum )]
{
	GL_RGB,

	GL_RED,
	GL_RG,
	GL_RGB,
	GL_RGBA,

	GL_RED,
	GL_RG,
	GL_RGB,
	GL_RGBA,

	GL_RED,
	GL_RG,
	GL_RGB,
	GL_RGBA,

	GL_RED,
	GL_RG,
	GL_RGB,
	GL_RGBA,
};

CTexture::CTexture()
{
	Location = "";

	Handle = 0;
	Width = 0;
	Height = 0;
	Channels = 0;

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

	if( !Pixels )
	{
		Pixels = GetImageData();
	}

	if( !Pixels || WidthIn < 1 || HeightIn < 1 || ChannelsIn < 1 )
		return false;

	glGenTextures( 1, &Handle );
	glBindTexture( GL_TEXTURE_2D, Handle );

	// Wrapping parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	// Filtering parameters
	FilteringMode = ModeIn;
	const auto Mode = static_cast<EFilteringModeType>( FilteringMode );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilteringModeToEnum[Mode] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilteringModeToEnum[Mode] );

	Width = WidthIn;
	Height = HeightIn;
	Channels = ChannelsIn;

	const auto ImageFormat = static_cast<EImageFormatType>( PreferredFormatIn );
	// auto Format = ImageFormatToFormat[ImageFormat];
	auto Type = ImageFormatToType[ImageFormat];

	const bool PowerOfTwoWidth = ( Width & ( Width - 1 ) ) == 0;
	const bool PowerOfTwoHeight = ( Height & ( Height - 1 ) ) == 0;
	if( PowerOfTwoWidth && PowerOfTwoHeight )
	{
		if( Channels == 1 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, Width, Height, 0, GL_RED, Type, Pixels );
			Supported = true;
		}
		else if( Channels == 2 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RG, Width, Height, 0, GL_RG, Type, Pixels );
			Supported = true;
		}
		else if( Channels == 3 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, Type, Pixels );
			Supported = true;
		}
		else if( Channels == 4 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, Type, Pixels );
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

void CTexture::Bind( ETextureSlot Slot )
{
	if( Handle )
	{
		const auto Index = static_cast<std::underlying_type<ETextureSlot>::type>( Slot );
		glActiveTexture( SlotToEnum[Index] );
		glBindTexture( GL_TEXTURE_2D, Handle );
	}
}

const int CTexture::GetWidth() const
{
	return Width;
}

const int CTexture::GetHeight() const
{
	return Height;
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
