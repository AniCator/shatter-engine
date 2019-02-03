// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Shader.h"
#include <Engine/Utility/File.h>
#include <Engine/Profiling/Logging.h>

#include <sstream>


CShader::CShader()
{
	Handle = 0;
	HandleVS = 0;
	HandleFS = 0;
}

CShader::~CShader()
{

}

bool CShader::Load( const char* FileLocation )
{
	Location = FileLocation;

	std::stringstream VertexPath;
	std::stringstream FragmentPath;

	VertexPath << FileLocation << ".vs";
	FragmentPath << FileLocation << ".fs";

	const bool LoadedVertexShader = Load( VertexPath.str().c_str(), HandleVS, EShaderType::Vertex );
	const bool LoadedFragmentShader = Load( FragmentPath.str().c_str(), HandleFS, EShaderType::Fragment );

	if( LoadedVertexShader && LoadedFragmentShader )
	{
		Handle = Link();

		return Handle != 0;
	}

	return false;
}

bool CShader::Load( const char* FileLocation, GLuint& HandleIn, EShaderType ShaderType )
{
	CFile ShaderSource( FileLocation );
	const bool Loaded = ShaderSource.Load();

	if( !Loaded )
		return false;

	const char* ShaderData = ShaderSource.Fetch<char>();

	if( ShaderData )
	{
		GLuint ShaderTypeGL = static_cast<GLuint>( ShaderType );

		HandleIn = glCreateShader( ShaderTypeGL );
		glShaderSource( HandleIn, 1, &ShaderData, NULL );

		return true;
	}

	return false;
}

bool CShader::Reload()
{
	return Load( Location.c_str() );
}

GLuint CShader::Activate() const
{
	glUseProgram( Handle );

	return Handle;
}

bool LogShaderCompilationErrors( GLuint v )
{
	GLint ByteLength = 0;
	GLsizei StringLength = 0;

	glGetShaderiv( v, GL_INFO_LOG_LENGTH, &ByteLength );

	if( ByteLength > 1 )
	{
		GLchar* CompileLog = new GLchar[ByteLength];
		glGetShaderInfoLog( v, ByteLength, &StringLength, CompileLog );

		Log::Event( Log::Error, "---Shader Compilation Log---\n%s\n", CompileLog );

		delete CompileLog;

		return true;
	}

	return false;
}

GLuint CShader::Link()
{
	if( HandleVS == 0 || HandleFS == 0 )
	{
		return 0;
	}

	// Compile all shaders
	bool ShaderCompiled = false;
	while( !ShaderCompiled )
	{
		glCompileShader( HandleVS );
		const bool HasErrorsVS = LogShaderCompilationErrors( HandleVS );
		glCompileShader( HandleFS );
		const bool HasErrorsFS = LogShaderCompilationErrors( HandleFS );
		ShaderCompiled = !HasErrorsVS && !HasErrorsFS;

		if( !ShaderCompiled )
		{
			Reload();
		}
	}

	// Create the program
	GLuint ProgramHandle = glCreateProgram();

	// Attach shaders to program
	glAttachShader( ProgramHandle, HandleVS );
	glAttachShader( ProgramHandle, HandleFS );

	// Link and set program to use
	glLinkProgram( ProgramHandle );

	GLint LinkStatus;
	glGetProgramiv( ProgramHandle, GL_LINK_STATUS, &LinkStatus );

	glDeleteShader( HandleVS );
	glDeleteShader( HandleFS );

	if( LinkStatus == GL_FALSE )
	{
		Log::Event( "Shader program compilation failed.\n" );
		return 0;
	}

	return ProgramHandle;
}
