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
	CRenderTexture( const std::string& Name, int TextureWidth, int TextureHeight );
	~CRenderTexture();

	void Initalize();
	void Push();
	void Pop();

private:
	GLuint FramebufferHandle;
	GLuint DepthHandle;
	FName Name;

	int Width;
	int Height;
	int Channels;

	unsigned char* ImageData;
};
