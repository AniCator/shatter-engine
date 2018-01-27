// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glad/glad.h"
#include <string>

enum EShaderType
{
	Vertex,
	Fragment
};

class CShader
{
public:
	CShader();
	~CShader();

	inline static GLuint ShaderTypeToGL( EShaderType ShaderType );

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
