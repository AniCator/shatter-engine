// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <stdint.h>

#include <ThirdParty/glad/include/glad/glad.h>

#include <Engine/Display/Rendering/TextureEnumerators.h>
#include <Engine/Utility/Data.h>

using TextureHandle = GLuint;

class CTexture
{
public:
	CTexture();
	CTexture( const char* FileLocation );
	virtual ~CTexture();

	virtual bool Load( const EFilteringMode Mode = EFilteringMode::Trilinear, const EImageFormat PreferredFormat = EImageFormat::RGB8, const bool& GenerateMipMaps = true );
	
	// Load a 2D texture.
	bool Load( 
		unsigned char* Data, 
		const int Width, 
		const int Height, 
		const int Channels, 
		const EFilteringMode Mode = EFilteringMode::Trilinear, 
		const EImageFormat PreferredFormat = EImageFormat::RGB8, 
		const bool& GenerateMipMaps = true 
	);

	// Load a 3D texture.
	bool Load(
		unsigned char* Data,
		const int Width,
		const int Height,
		const int Depth,
		const int Channels,
		const EFilteringMode Mode = EFilteringMode::Trilinear,
		const EImageFormat PreferredFormat = EImageFormat::RGB8,
		const bool& GenerateMipMaps = true
	);

	void Save( const char* FileLocation = nullptr );
	virtual void Bind( ETextureSlot Slot ) const;

	const std::string& GetLocation() const;
	TextureHandle GetHandle() const;
	int GetWidth() const;
	int GetHeight() const;
	EImageFormat GetImageFormat() const;

	void* GetImageData() const;

	uint8_t GetAnisotropicSamples() const;
	void SetAnisotropicSamples( const uint8_t Samples );

	EFilteringMode FilteringMode;
	TextureHandle Handle;
protected:
	EImageFormat Format;

	// Used to determine how many samples should be used for anisotropic filtering.
	uint8_t AnisotropicSamples = 1;

	std::string Location;

	int Width;
	int Height;
	int Depth = 0;
	int Channels;

	unsigned char* ImageData8;
	unsigned short* ImageData16;
	unsigned int* ImageData32;
	float* ImageData32F;
};
