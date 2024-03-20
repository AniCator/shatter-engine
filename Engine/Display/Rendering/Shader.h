// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glad/glad.h"
#include <string>
#include <array>

#include <Engine/Utility/File.h>
#include <Engine/Display/Rendering/Uniform.h>

enum class EShaderType : uint16_t
{
	Vertex = GL_VERTEX_SHADER,
	Geometry = GL_GEOMETRY_SHADER,
	Fragment = GL_FRAGMENT_SHADER,
	Compute = GL_COMPUTE_SHADER,
	TesselationControl = GL_TESS_CONTROL_SHADER,
	TesselationEvaluation = GL_TESS_EVALUATION_SHADER
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
	GLuint Program = 0;

	GLuint VertexShader = 0;
	GLuint GeometryShader = 0;
	GLuint FragmentShader = 0;
	GLuint ComputeShader = 0;
	GLuint TesselationControlShader = 0;
	GLuint TesselationEvaluationShader = 0;
};

class CShader
{
public:
	CShader();
	~CShader() = default;

	bool Load( const bool& ShouldLink = true );
	bool Load( const char* FileLocation, const bool& ShouldLink = true, const EShaderType& ShaderType = EShaderType::Fragment );
	bool Load( const char* VertexLocation, const char* FragmentLocation, const bool& ShouldLink = true );

	// Used to load a shader from a file.
	bool Load( const std::string& FileLocation, GLuint& HandleIn, const EShaderType& ShaderType );

	// Used to load directly load code stored in strings as shaders.
	bool Load( GLuint& HandleIn, const EShaderType& ShaderType, const std::string& Code );

	bool Reload();

	GLuint Activate();
	const FProgramHandles& GetHandles() const;
	const EBlendMode::Type& GetBlendMode() const;

	const EDepthMask::Type& GetDepthMask() const;
	const EDepthTest::Type& GetDepthTest() const;

	void AutoReload( const bool& Enable );
	bool AutoReload() const
	{
		return ShouldAutoReload;
	}

	// Returns the default values of non-sampler uniforms.
	const std::vector<std::pair<std::string, Uniform>>& GetDefaults() const;

private:
	std::string Process( const CFile& File );
	std::string Process( const std::string& Data );
	std::string Process( std::stringstream& Stream );
	GLuint Link();

	FProgramHandles Handles;

	std::string Location;
	std::string GeometryLocation;
	std::string VertexLocation;
	std::string FragmentLocation;
	std::string ComputeLocation;

	EShaderType ShaderType = EShaderType::Fragment;
	EBlendMode::Type BlendMode = EBlendMode::Opaque;

	// glDepthMask
	EDepthMask::Type DepthMask = EDepthMask::Write;

	// glDepthFunc
	EDepthTest::Type DepthTest = EDepthTest::Less;

	time_t ModificationTime;
	bool ShouldAutoReload = false;

	// Stores the default values of non-sampler uniforms.
	std::vector<std::pair<std::string, Uniform>> Defaults;

	void GatherDefaults();
};
