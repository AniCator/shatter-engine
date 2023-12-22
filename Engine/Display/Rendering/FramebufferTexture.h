// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Rendering/Texture.h>

// Texture that doesn't store any data but instead just binds the main framebuffer's textures.
class FramebufferTexture : public CTexture
{
public:
	FramebufferTexture() = default;

	// Always returns true for main framebuffer texture lookups.
	bool Load( const EFilteringMode Mode = EFilteringMode::Trilinear, const EImageFormat PreferredFormat = EImageFormat::RGB8, const bool GenerateMipMaps = true ) override
	{
		return true; 
	};

	void Bind( ETextureSlot Slot ) const override;

	bool Depth = false;
};
