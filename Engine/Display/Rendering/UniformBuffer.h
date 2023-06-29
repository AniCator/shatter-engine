// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <cstdint>
#include <ThirdParty/glad/include/glad/glad.h>

template<typename T>
class UniformBufferObject
{
public:
	~UniformBufferObject()
	{
		if( BufferHandle )
		{
			glDeleteBuffers( 1, &BufferHandle );
		}
	}

	void Upload()
	{
		const bool ValidBuffer = BufferHandle != 0;
		if( !ValidBuffer )
		{
			glGenBuffers( 1, &BufferHandle );
		}

		glBindBuffer( GL_UNIFORM_BUFFER, BufferHandle );

		if( ValidBuffer )
		{
			glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( T ), &Data );
		}
		else
		{
			glBufferData( GL_UNIFORM_BUFFER, sizeof( T ), &Data, GL_DYNAMIC_DRAW );
		}
	}

	void Bind( const uint32_t& Binding = 0 ) const
	{
		glBindBufferBase( GL_UNIFORM_BUFFER, Binding, BufferHandle );
	}

	GLuint Handle() const
	{
		return BufferHandle;
	}

	size_t Size() const
	{
		return sizeof( T );
	}

	T Data;
protected:
	GLuint BufferHandle = 0;
};
