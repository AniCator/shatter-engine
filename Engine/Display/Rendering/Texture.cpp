// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <ThirdParty/stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <ThirdParty/stb/stb_image_write.h>
#include <glad/glad.h>

#include <Engine/Display/Rendering/TextureEnumeratorsGL.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>

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

	FilteringMode = EFilteringMode::Trilinear;
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

bool CTexture::Load( const EFilteringMode Mode, const EImageFormat PreferredFormat, const bool& GenerateMipMaps )
{
	CFile TextureSource( Location );
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
		else if( PreferredFormat > EImageFormat::SRGBA8 )
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

bool CreateTexture2D( 
	GLuint& Handle, 
	const int Width,
	const int Height,
	const int Channels,
	const EImageFormat Format,
	const EFilteringMode FilteringMode,
	const int AnisotropicSamples,
	const bool GenerateMipMaps,
	const void* Pixels

)
{
	glBindTexture( GL_TEXTURE_2D, Handle );

	// Wrapping parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	// Filtering parameters
	const auto Mode = static_cast<EFilteringModeType>( FilteringMode );
	const auto MinFilter = GenerateMipMaps ? GetFilteringMipMapMode( Mode ) : GetFilteringMode( Mode );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetFilteringMode( Mode ) );

#ifdef GL_ARB_texture_filter_anisotropic
	if( !!GLAD_GL_ARB_texture_filter_anisotropic )
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, AnisotropicSamples );
	}
#endif

	const auto ImageFormat = static_cast<EImageFormatType>( Format );
	auto Type = GetImageFormatType( ImageFormat );
	auto InternalFormat = ::GetImageFormat( ImageFormat );

	const bool PowerOfTwoWidth = ( Width & ( Width - 1 ) ) == 0;
	const bool PowerOfTwoHeight = ( Height & ( Height - 1 ) ) == 0;
	if( PowerOfTwoWidth && PowerOfTwoHeight )
	{
		if( Channels == 1 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, GL_RED, Type, Pixels );
		}
		else if( Channels == 2 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, GL_RG, Type, Pixels );
		}
		else if( Channels == 3 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, GL_RGB, Type, Pixels );
		}
		else if( Channels == 4 )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, GL_RGBA, Type, Pixels );
		}
		else
		{
			// Unsupported.
			Log::Event( Log::Warning, "Unsupported 2D texture format.\n" );
			return false;
		}
	}
	else
	{
		// Not a power of two.
		Log::Event( Log::Warning, "Not a power of two texture.\n" );
		return false;
	}

	if( GenerateMipMaps )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	return true;
}

bool CreateTexture3D(
	GLuint& Handle,
	const int Width,
	const int Height,
	const int Depth,
	const int Channels,
	const EImageFormat Format,
	const EFilteringMode FilteringMode,
	const int AnisotropicSamples,
	const bool GenerateMipMaps,
	const void* Pixels

)
{
	glBindTexture( GL_TEXTURE_3D, Handle );

	// Wrapping parameters
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	// Filtering parameters
	const auto Mode = static_cast<EFilteringModeType>( FilteringMode );
	const auto MinFilter = GenerateMipMaps ? GetFilteringMipMapMode( Mode ) : GetFilteringMode( Mode );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, MinFilter );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GetFilteringMode( Mode ) );

#ifdef GL_ARB_texture_filter_anisotropic
	if( !!GLAD_GL_ARB_texture_filter_anisotropic )
	{
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY, AnisotropicSamples );
	}
#endif

	const auto ImageFormat = static_cast<EImageFormatType>( Format );
	auto Type = GetImageFormatType( ImageFormat );
	auto InternalFormat = ::GetImageFormat( ImageFormat );

	const bool PowerOfTwoWidth = ( Width & ( Width - 1 ) ) == 0;
	const bool PowerOfTwoHeight = ( Height & ( Height - 1 ) ) == 0;
	if( PowerOfTwoWidth && PowerOfTwoHeight )
	{
		if( Channels == 1 )
		{
			glTexImage3D( GL_TEXTURE_3D, 0, InternalFormat, Width, Height, Depth, 0, GL_RED, Type, Pixels );
		}
		else if( Channels == 2 )
		{
			glTexImage3D( GL_TEXTURE_3D, 0, InternalFormat, Width, Height, Depth, 0, GL_RG, Type, Pixels );
		}
		else if( Channels == 3 )
		{
			glTexImage3D( GL_TEXTURE_3D, 0, InternalFormat, Width, Height, Depth, 0, GL_RGB, Type, Pixels );
		}
		else if( Channels == 4 )
		{
			glTexImage3D( GL_TEXTURE_3D, 0, InternalFormat, Width, Height, Depth, 0, GL_RGBA, Type, Pixels );
		}
		else
		{
			// Unsupported.
			Log::Event( Log::Warning, "Unsupported 3D texture format.\n" );
			return false;
		}
	}
	else
	{
		// Not a power of two.
		Log::Event( Log::Warning, "Not a power of two texture.\n" );
		return false;
	}

	if( GenerateMipMaps )
	{
		glGenerateMipmap( GL_TEXTURE_3D );
	}

	return true;
}

bool CTexture::Load( unsigned char* Data, const int WidthIn, const int HeightIn, const int ChannelsIn, const EFilteringMode ModeIn, const EImageFormat PreferredFormatIn, const bool& GenerateMipMaps )
{
	bool Supported = false;

	const void* Pixels = Data;

	if( Pixels )
	{
		// Delete old data.
		auto* ImageData = GetImageData();
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

	if( Depth <= 1 )
	{
		Supported = CreateTexture2D(
			Handle,
			Width,
			Height,
			Channels,
			Format,
			FilteringMode,
			AnisotropicSamples,
			GenerateMipMaps,
			Pixels
		);
	}
	else
	{
		Supported = CreateTexture3D(
			Handle,
			Width,
			Height,
			Depth,
			Channels,
			Format,
			FilteringMode,
			AnisotropicSamples,
			GenerateMipMaps,
			Pixels
		);
	}

	if( !Supported )
	{
		Log::Event( Log::Warning, "Failed to initialize texture (\"%s\").\n", Location.c_str() );
		return false;
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

void CTexture::Bind( ETextureSlot Slot ) const
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

GLuint CTexture::GetHandle() const
{
	return Handle;
}

int CTexture::GetWidth() const
{
	return Width;
}

int CTexture::GetHeight() const
{
	return Height;
}

EImageFormat CTexture::GetImageFormat() const
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

uint8_t CTexture::GetAnisotropicSamples() const
{
	return AnisotropicSamples;
}

void CTexture::SetAnisotropicSamples( const uint8_t Samples )
{
	constexpr uint8_t Minimum = 1;
	constexpr uint8_t Maximum = 16;

	AnisotropicSamples = Math::Clamp( Samples, Minimum, Maximum );

#ifdef GL_ARB_texture_filter_anisotropic
	if( !Handle )
		return;

	glBindTexture( GL_TEXTURE_2D, Handle );

	if( !!GLAD_GL_ARB_texture_filter_anisotropic )
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, AnisotropicSamples );
	}

	glBindTexture( GL_TEXTURE_2D, 0 );
#endif
}
