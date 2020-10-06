// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <stdint.h>

#include <ThirdParty/glad/include/glad/glad.h>

#include <Engine/Display/Rendering/TextureEnumerators.h>
#include <Engine/Utility/Data.h>

class CTexture
{
public:
	CTexture();
	CTexture( const char* FileLocation );
	virtual ~CTexture();

	virtual bool Load( const EFilteringMode Mode = EFilteringMode::Linear, const EImageFormat PreferredFormat = EImageFormat::RGB8 );
	bool Load( unsigned char* Data, const int Width, const int Height, const int Channels, const EFilteringMode Mode = EFilteringMode::Linear, const EImageFormat PreferredFormat = EImageFormat::RGB8 );
	void Save( const char* FileLocation = nullptr );
	virtual void Bind( ETextureSlot Slot ) const;

	const std::string& GetLocation() const;
	const GLuint GetHandle() const;
	const int GetWidth() const;
	const int GetHeight() const;
	const EImageFormat GetImageFormat() const;

	void* GetImageData() const;

	EFilteringMode FilteringMode;
	GLuint Handle;
protected:
	EImageFormat Format;
	std::string Location;

	int Width;
	int Height;
	int Channels;

	unsigned char* ImageData8;
	unsigned short* ImageData16;
	unsigned int* ImageData32;
	float* ImageData32F;
};
