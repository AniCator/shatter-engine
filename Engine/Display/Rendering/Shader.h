// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glad/glad.h"
#include <string>

enum class EShaderType : uint16_t
{
	Vertex = GL_VERTEX_SHADER,
	Fragment = GL_FRAGMENT_SHADER
};

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

	bool Load( const char* FileLocation );
	bool Load( const char* FileLocation, GLuint& HandleIn, EShaderType ShaderType );

	bool Reload();

	GLuint Activate() const;
	const FProgramHandles& GetHandles() const;

private:
	GLuint Link();

	FProgramHandles Handles;

	std::string Location;
};
