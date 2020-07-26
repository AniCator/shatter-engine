// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <stdint.h>

#include <ThirdParty/glad/include/glad/glad.h>

#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/Structures/Name.h>

class CRenderTexture : public CTexture
{
public:
	CRenderTexture();
	CRenderTexture( const std::string& Name, int TextureWidth, int TextureHeight, const bool DepthOnly = false );
	~CRenderTexture();

	void BindDepth( ETextureSlot Slot ) const;

	void Initialize();
	void Push();
	void Pop();

	void Invalidate() { Initialized = false; };
	bool Ready() const { return Initialized; };

	FName Name;
private:
	GLuint FramebufferHandle;
	GLuint DepthHandle;

	bool DepthOnly;
	bool Initialized;
};
