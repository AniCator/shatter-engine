// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glad/glad.h"
#include <string>

#include <Engine/Utility/File.h>

enum class EShaderType : uint16_t
{
	Vertex = GL_VERTEX_SHADER,
	Fragment = GL_FRAGMENT_SHADER
};

namespace EBlendMode
{
	enum Type
	{
		Opaque = 0,
		Alpha,
		Additive
	};
}

struct FProgramHandles
{
	FProgramHandles()
	{
		Program = 0;
		VertexShader = 0;
		FragmentShader = 0;
	}

	GLuint Program;
	GLuint VertexShader;
	GLuint FragmentShader;
};

class CShader
{
public:
	CShader();
	~CShader();

	bool Load( const char* FileLocation, bool ShouldLink = true );
	bool Load( const char* VertexLocation, const char* FragmentLocation, bool ShouldLink = true );
	bool Load( const char* FileLocation, GLuint& HandleIn, EShaderType ShaderType );

	bool Reload();

	GLuint Activate() const;
	const FProgramHandles& GetHandles() const;
	const EBlendMode::Type& GetBlendMode() const;

private:
	std::string Process( const CFile& File );
	GLuint Link();

	FProgramHandles Handles;

	std::string VertexLocation;
	std::string FragmentLocation;

	EBlendMode::Type BlendMode;
};
