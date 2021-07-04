// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glad/include/glad/glad.h>

template<typename T>
class ShaderStorageBuffer
{
public:
	~ShaderStorageBuffer()
	{
		if( BufferHandle )
		{
			glDeleteBuffers( 1, &BufferHandle );
		}
	}

	void Initialize( T* Data, const size_t& Count )
	{
		BufferData = Data;
		BufferCount = Count;

		if( BufferData == nullptr || BufferCount < 1 )
		{
			return;
		}

		if( BufferHandle )
		{
			glDeleteBuffers( 1, &BufferHandle );
		}

		glGenBuffers( 1, &BufferHandle );
		glBindBuffer( GL_SHADER_STORAGE_BUFFER, BufferHandle );
		glBufferData( GL_SHADER_STORAGE_BUFFER, BufferCount * sizeof( T ), BufferData, GL_STATIC_DRAW );
	}

	void Bind() const
	{
		glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, BufferHandle );
	}

	GLuint Handle() const
	{
		return BufferHandle;
	}

	size_t Count() const
	{
		return BufferCount;
	}

	size_t Size() const
	{
		return BufferCount * sizeof( T );
	}

protected:
	GLuint BufferHandle = 0;
	size_t BufferCount = 0;
	T* BufferData = nullptr;
};
