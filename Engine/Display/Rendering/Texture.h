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
	~CTexture();

	bool Load();
	bool Load( unsigned char* Data, const int Width, const int Height, const int Channels, const EFilteringMode Mode = EFilteringMode::Linear );
	void Bind( ETextureSlot Slot );

	const int GetWidth() const;
	const int GetHeight() const;

	EFilteringMode FilteringMode;

protected:
	GLuint Handle;
	std::string Location;

	int Width;
	int Height;
	int Channels;

	unsigned char* ImageData;
};
