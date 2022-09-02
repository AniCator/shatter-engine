// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <stdint.h>

#include <ThirdParty/glad/include/glad/glad.h>

#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/Structures/Name.h>

struct ScreenBuffer
{
	// Framebuffer.
	GLuint Buffer = 0;

	// Color texture.
	GLuint Color = 0;

	// Depth texture.
	GLuint Depth = 0;

	bool Multisampled = false;
};

struct RenderTextureConfiguration
{
	bool EnableColor = true;
	bool EnableDepth = true;
	int Samples = 0;

	int Width = 0; // Only used on construction. Use the texture's internal width and height members outside of it.
	int Height = 0; // Only used on construction. Use the texture's internal width and height members outside of it.

	EImageFormat Format = EImageFormat::RGB16F;
};

class CRenderTexture : public CTexture
{
public:
	CRenderTexture();
	CRenderTexture( const std::string& Name, int TextureWidth, int TextureHeight, const bool DepthOnly = false );
	CRenderTexture( const std::string& Name, const RenderTextureConfiguration Configuration );
	~CRenderTexture() override;

	void BindDepth( ETextureSlot Slot ) const;

	void Initialize();
	void Push();
	void Pop();

	// Required for multi-sampling.
	void Prepare();
	// Copies multi-sampled buffers to the render texture framebuffer.
	void Resolve();
	void Delete();

	void Invalidate() { Initialized = false; };
	bool Ready() const { return Initialized; };

	void SetSampleCount( const int Samples );
	int GetSampleCount() const;

	RenderTextureConfiguration GetConfiguration() const;

	NameSymbol Name = NameSymbol::Invalid;
protected:
	GLuint GetFramebuffer() const;

	ScreenBuffer FrameBuffer;
	bool Multisampled = false;

	ScreenBuffer MultisampledBuffer;
	bool HasBeenResolved = false;

	bool Initialized = false;
	RenderTextureConfiguration Configuration;
};
