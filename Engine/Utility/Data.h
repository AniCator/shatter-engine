// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <sstream>
#include <Engine/Profiling/Logging.h>

class CData
{
public:
	CData()
	{
		Invalid = false;
	}

	template<typename T>
	void operator<<( T& Object )
	{
		static_assert( !std::is_pointer<T>::value, "Pointers can not be serialized." );
		const uint32_t Size = sizeof( T );
		Data.write( reinterpret_cast<const char*>( &Object ), Size );

		if( !Data.good() )
		{
			Log::Event( Log::Error, "Could not write data.\n" );
		}
	}

	void operator<<( const char* Object )
	{
		const uint32_t Size = strlen( Object ) * sizeof( char );
		Data.write( Object, Size + 1 );

		static const uint32_t MaximumSize = 1073741824;
		if( Size > MaximumSize )
		{
			Log::Event( Log::Warning, "Stored object is rather large.\n" );
		}

		if( !Data.good() )
		{
			Log::Event( Log::Error, "Could not write string data.\n" );
		}
	}

	void operator<<( CData& Object )
	{
		const uint32_t Size = Object.Size();
		Data.write( reinterpret_cast<const char*>( &Size ), sizeof( uint32_t ) );

		static const uint32_t MaximumSize = 1073741824;
		if( Size > MaximumSize )
		{
			Log::Event( Log::Warning, "Stored object is rather large.\n" );
		}

		if( !Data.good() )
		{
			Log::Event( Log::Error, "Could not write CData size.\n" );
		}

		Data.write( Object.Data.str().c_str(), Object.Size() );

		if( !Data.good() )
		{
			Log::Event( Log::Error, "Could not write CData data.\n" );
		}
	}

	template<typename T>
	uint32_t operator>>( T& Object )
	{
		if( Invalid )
			return 0;

		static_assert( !std::is_pointer<T>::value, "Pointers can not be deserialized." );
		const uint32_t Size = sizeof( T );

		const auto Position = Data.tellg();
		Data.read( reinterpret_cast<char*>( &Object ), Size );
		
		if( !Data.good() )
		{
			// Reset the position.
			Data.clear();
			Data.seekg( Position );

			Log::Event( Log::Error, "Could not extract data stream to object.\n" );

			return 0;
		}

		return Size;
	}

	void operator>>( char* Object )
	{
		if( Invalid )
			return;

		const auto Position = Data.tellg();		
		do
		{
			Data.read( Object, sizeof( char ) );
			Object++;
		} while( Object[-1] != '\0' );

		if( !Data.good() )
		{
			// Reset the position.
			Data.clear();
			Data.seekg( Position );

			Log::Event( Log::Error, "Could not extract data stream to character array.\n" );
		}
	}

	void operator>>( CData& Object )
	{
		if( Invalid )
			return;

		uint32_t Size = 0;
		Data.read( reinterpret_cast<char*>( &Size ), sizeof( uint32_t ) );
		// Data >> Size;

		// Warn when the size is rather large.
		static const uint32_t MaximumSize = 1073741824;
		if( Size > MaximumSize )
		{
			Log::Event( Log::Warning, "Extracted object is rather large.\n" );
		}

		char* Stream = new char[Size];
		Data.read( Stream, Size );
		Object.Data.write( Stream, Size );

		if( !Data.good() )
		{
			Object.Invalidate();
			Invalidate();
		}
	}

	void Store( char* Buffer, const size_t Size )
	{
		ReadToStart();
		Data.read( Buffer, Size );
	}

	void Load( const char* Buffer, const size_t Size )
	{
		Data = std::stringstream();
		Data.write( Buffer, Size );
		ReadToStart();
	}

	// Moves the reading cursor to the start of the data.
	void ReadToStart()
	{
		Data.seekg( std::ios::beg );
	}

	// Moves the reading cursor to the end of the data.
	void ReadToEnd()
	{
		Data.seekg( Size() );
	}

	// Moves the writing cursor to the start of the data.
	void WriteToStart()
	{
		Data.seekp( std::ios::beg );
	}

	// Moves the writing cursor to the end of the data.
	void WriteToEnd()
	{
		Data.seekp( Size() );
	}

	void Jump( const int32_t Offset )
	{
		Data.seekg( Offset, std::ios::cur );
	}

	std::basic_stringstream<char>::pos_type WritePosition()
	{
		return Data.tellp();
	}

	void WritePosition( const int32_t Position )
	{
		Data.clear();
		Data.seekp( Position );
	}

	std::basic_stringstream<char>::pos_type ReadPosition()
	{
		return Data.tellg();
	}

	void ReadPosition( const int32_t Position )
	{
		Data.clear();
		Data.seekg( Position );
	}

	const size_t Size()
	{
		return Data.tellp();
	}

	const bool Valid() const
	{
		return !Invalid;
	}

	void Invalidate()
	{
		Invalid = true;
		__debugbreak();
	}

private:
	std::stringstream Data;

	bool Invalid;
};
