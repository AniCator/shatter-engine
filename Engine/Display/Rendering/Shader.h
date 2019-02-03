// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glad/glad.h"
#include <string>

enum class EShaderType : uint16_t
{
	Vertex = GL_VERTEX_SHADER,
	Fragment = GL_FRAGMENT_SHADER
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

	GLuint Handle;

private:
	GLuint Link();

	GLuint HandleVS;
	GLuint HandleFS;

	std::string Location;
};
