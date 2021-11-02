// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <cstdint>
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

	void Initialize( T* Data, const size_t& Count, const bool& DeleteExisting = true )
	{
		BufferData = Data;
		BufferCount = Count;

		if( BufferData == nullptr || BufferCount < 1 )
		{
			return;
		}

		if( BufferHandle && DeleteExisting )
		{
			glDeleteBuffers( 1, &BufferHandle );
			BufferHandle = 0;
		}

		if( !BufferHandle )
		{
			glGenBuffers( 1, &BufferHandle );
		}

		glBindBuffer( GL_SHADER_STORAGE_BUFFER, BufferHandle );
		glBufferData( GL_SHADER_STORAGE_BUFFER, BufferCount * sizeof( T ), BufferData, GL_DYNAMIC_DRAW );
	}

	void Bind( const uint32_t& Binding = 0 ) const
	{
		glBindBufferBase( GL_SHADER_STORAGE_BUFFER, Binding, BufferHandle );
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
