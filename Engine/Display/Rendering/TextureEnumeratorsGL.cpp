// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "TextureEnumeratorsGL.h"

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
	GL_LINEAR,
	GL_LINEAR
};

constexpr GLenum Nearest = GL_NEAREST_MIPMAP_LINEAR;
constexpr GLenum Bilinear = GL_LINEAR_MIPMAP_NEAREST;
constexpr GLenum Trilinear = GL_LINEAR_MIPMAP_LINEAR;

static const GLenum FilteringModeToMipMapEnum[static_cast<EFilteringModeType>( EFilteringMode::Maximum )]
{
	Nearest,
	Bilinear,
	Trilinear
};

static const GLenum ImageFormatToType[static_cast<EImageFormatType>( EImageFormat::Maximum )]
{
	GL_UNSIGNED_BYTE,

	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,

	GL_UNSIGNED_BYTE, // sRGB
	GL_UNSIGNED_BYTE, // sRGB

	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_SHORT,

	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT,

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

	GL_RGB, // sRGB
	GL_RGBA, // sRGB

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

static const GLenum ImageFormatToInternalFormat[static_cast<EImageFormatType>( EImageFormat::Maximum )]
{
	GL_RGB8,

	GL_R8,
	GL_RG8,
	GL_RGB8,
	GL_RGBA8,

	GL_SRGB8, // sRGB
	GL_SRGB8_ALPHA8, // sRGB

	GL_R16,
	GL_RG16,
	GL_RGB16,
	GL_RGBA16,

	GL_R16F,
	GL_RG16F,
	GL_RGB16F,
	GL_RGBA16F,

	GL_R32F,
	GL_RG32F,
	GL_RGB32F,
	GL_RGBA32F,
};

GLenum GetTextureSlot( const ETextureSlotType& Slot )
{
	return SlotToEnum[Slot];
}

GLenum GetFilteringMode( const EFilteringModeType& FilteringMode )
{
	return FilteringModeToEnum[FilteringMode];
}

GLenum GetFilteringMipMapMode( const EFilteringModeType& FilteringMode )
{
	return FilteringModeToMipMapEnum[FilteringMode];
}

GLenum GetImageFormatType( const EImageFormatType& ImageFormat )
{
	return ImageFormatToType[ImageFormat];
}

GLenum GetImageFormatChannels( const EImageFormatType& ImageFormat )
{
	return ImageFormatToFormat[ImageFormat];
}

GLenum GetImageFormat( const EImageFormatType& ImageFormat )
{
	return ImageFormatToInternalFormat[ImageFormat];
}
