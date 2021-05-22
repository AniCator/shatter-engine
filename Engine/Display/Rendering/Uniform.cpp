// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Uniform.h"

#include <ThirdParty/glad/include/glad/glad.h>

void Uniform::Bind( const unsigned int& Program, const std::string& Location ) const
{
	const GLint UniformBufferLocation = glGetUniformLocation( Program, Location.c_str() );
	if( UniformBufferLocation > -1 )
	{
		if( Type == Component4 )
		{
			glUniform4fv( UniformBufferLocation, 1, Uniform4.Base() );
		}
		else if( Type == Component3 )
		{
			glUniform3fv( UniformBufferLocation, 1, Uniform3.Base() );
		}
		else if( Type == Component4x4 )
		{
			glUniformMatrix4fv( UniformBufferLocation, 1, GL_FALSE, &Uniform4x4[0][0] );
		}
		else if( Type == Unsigned )
		{
			glUniform1ui( UniformBufferLocation, UniformUnsigned );
		}
	}
}
