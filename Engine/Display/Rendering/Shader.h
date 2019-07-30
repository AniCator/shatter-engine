// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glad/glad.h"
#include <string>

#include <Engine/Utility/File.h>

enum class EShaderType : uint16_t
{
	Vertex = GL_VERTEX_SHADER,
	Fragment = GL_FRAGMENT_SHADER,
	Compute = GL_COMPUTE_SHADER
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

namespace EDepthMask
{
	enum Type
	{
		Ignore = 0,
		Write,
		Maximum
	};
}

namespace EDepthTest
{
	enum Type
	{
		Never = 0,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always,
		Maximum
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

	bool Load( bool ShouldLink = true );
	bool Load( const char* FileLocation, bool ShouldLink = true );
	bool Load( const char* VertexLocation, const char* FragmentLocation, bool ShouldLink = true );
	bool Load( const char* FileLocation, GLuint& HandleIn, EShaderType ShaderType );

	bool Reload();

	GLuint Activate();
	const FProgramHandles& GetHandles() const;
	const EBlendMode::Type& GetBlendMode() const;
	const EDepthMask::Type& GetDepthMask() const;
	const EDepthTest::Type& GetDepthTest() const;

private:
	std::string Process( const CFile& File );
	GLuint Link();

	FProgramHandles Handles;

	std::string VertexLocation;
	std::string FragmentLocation;

	EBlendMode::Type BlendMode;
	EDepthMask::Type DepthMask;
	EDepthTest::Type DepthTest;

	time_t ModificationTime;

};
