// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Shader.h"
#include <Utility/File.h>
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

GLuint CShader::ShaderTypeToGL( EShaderType ShaderType )
{
	if( ShaderType == Vertex )
	{
		return GL_VERTEX_SHADER;
	}

	return GL_FRAGMENT_SHADER;
}

bool CShader::Load( const char* FileLocation )
{
	Location = FileLocation;

	std::stringstream VertexPath;
	std::stringstream FragmentPath;

	VertexPath << FileLocation << ".vs";
	FragmentPath << FileLocation << ".fs";

	const bool LoadedVertexShader = Load( VertexPath.str().c_str(), HandleVS, Vertex );
	const bool LoadedFragmentShader = Load( FragmentPath.str().c_str(), HandleFS, Fragment );

	if( LoadedVertexShader && LoadedFragmentShader )
	{
		Handle = Link();

		return Handle != 0;
	}

	return false;
}

bool CShader::Load( const char* FileLocation, GLuint& HandleIn, EShaderType ShaderType )
{
	CFile ShaderSource;
	const bool Loaded = ShaderSource.Load( FileLocation );

	if( !Loaded )
		return false;

	const char* ShaderData = ShaderSource.Fetch<char>();

	if( ShaderData )
	{
		GLuint ShaderTypeGL = ShaderTypeToGL( ShaderType );

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

void LogShaderCompilationErrors( GLuint v )
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
	}
}

GLuint CShader::Link()
{
	if( HandleVS == 0 || HandleFS == 0 )
	{
		return 0;
	}

	// Compile all shaders
	glCompileShader( HandleVS );
	LogShaderCompilationErrors( HandleVS );
	glCompileShader( HandleFS );
	LogShaderCompilationErrors( HandleFS );

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
