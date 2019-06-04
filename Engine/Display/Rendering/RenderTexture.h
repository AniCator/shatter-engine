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
	CRenderTexture( const std::string& Name, int TextureWidth, int TextureHeight );
	~CRenderTexture();

	void Initialize();
	void Push();
	void Pop();

	bool Ready() const { return Initialized; };

private:
	GLuint FramebufferHandle;
	GLuint DepthHandle;
	FName Name;

	int Width;
	int Height;
	int Channels;

	unsigned char* ImageData;

	bool Initialized;
};
