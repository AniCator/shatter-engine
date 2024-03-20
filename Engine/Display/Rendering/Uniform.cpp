// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Uniform.h"

#include <ThirdParty/glad/include/glad/glad.h>

void Uniform::Bind( const unsigned int& Program, const std::string& Location )
{
	if( Type == Undefined )
		return;

	Name = Location;
	Bind( Program );
}

void Uniform::Bind( const unsigned int& Program )
{
	// Cache the program location.
	if( Program != LastProgram )
	{
		BufferLocation = glGetUniformLocation( Program, Name.c_str() );
		LastProgram = Program;
	}

	if( BufferLocation > -1 )
	{
		switch( Type )
		{
		case Component4:
			glUniform4fv( BufferLocation, 1, Uniform4.Base() );
			break;
		case Component3:
			glUniform3fv( BufferLocation, 1, Uniform3.Base() );
			break;
		case Component2:
			glUniform2fv( BufferLocation, 1, Uniform2.Base() );
			break;
		case Component4x4:
			glUniformMatrix4fv( BufferLocation, 1, GL_FALSE, &Uniform4x4[0][0] );
			break;
		case Unsigned:
			glUniform1ui( BufferLocation, UniformUnsigned );
			break;
		case Signed:
			glUniform1i( BufferLocation, UniformSigned4[0] );
			break;
		case Signed4:
			glUniform4iv( BufferLocation, 1, UniformSigned4 );
			break;
		case Float:
			glUniform1f( BufferLocation, UniformFloat );
			break;
		default:
			break;
		}
	}
}

void Uniform::Reset()
{
	switch( Type )
	{
	case Component4:
		Uniform4 = { 0.0f, 0.0f, 0.0f, 0.0f };
		break;
	case Component3:
		Uniform3 = { 0.0f, 0.0f, 0.0f };
		break;
	case Component2:
		Uniform2 = { 0.0f, 0.0f };
		break;
	case Component4x4:
		Uniform4x4 = {};
		break;
	case Unsigned:
		UniformUnsigned = 0;
		break;
	case Signed:
	case Signed4:
		UniformSigned4[0] = 0;
		UniformSigned4[1] = 0;
		UniformSigned4[2] = 0;
		UniformSigned4[3] = 0;
		break;
	case Float:
		UniformFloat = 0.0f;
		break;
	default:
		break;
	}
}
