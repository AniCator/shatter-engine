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

void Uniform::Bind( const unsigned& Program )
{
	// Cache the program location.
	if( Program != LastProgram )
	{
		BufferLocation = glGetUniformLocation( Program, Name.c_str() );
		LastProgram = Program;
	}

	if( BufferLocation > -1 )
	{
		if( Type == Component4 )
		{
			glUniform4fv( BufferLocation, 1, Uniform4.Base() );
		}
		else if( Type == Component3 )
		{
			glUniform3fv( BufferLocation, 1, Uniform3.Base() );
		}
		else if( Type == Component4x4 )
		{
			glUniformMatrix4fv( BufferLocation, 1, GL_FALSE, &Uniform4x4[0][0] );
		}
		else if( Type == Unsigned )
		{
			glUniform1ui( BufferLocation, UniformUnsigned );
		}
		else if( Type == Signed )
		{
			glUniform1i( BufferLocation, UniformSigned );
		}
		else if( Type == Signed4 && UniformSigned4 )
		{
			glUniform4iv( BufferLocation, 1, UniformSigned4 );
		}
		else if( Type == Float )
		{
			glUniform1f( BufferLocation, UniformFloat );
		}
	}
}
