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
		const size_t Size = sizeof( T );
		Data.write( reinterpret_cast<const char*>( &Object ), Size );

		if( !Data.good() )
		{
			Log::Event( Log::Error, "Could not write data.\n" );
		}
	}

	void operator<<( const char* Object )
	{
		const size_t Size = strlen( Object ) * sizeof( char );
		Data.write( Object, Size + 1 );

		if( !Data.good() )
		{
			Log::Event( Log::Error, "Could not write string data.\n" );
		}
	}

	void operator<<( CData& Object )
	{
		const size_t Size = Object.Size();
		Data.write( reinterpret_cast<const char*>( &Size ), sizeof( size_t ) );

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
	void operator>>( T& Object )
	{
		if( Invalid )
			return;

		static_assert( !std::is_pointer<T>::value, "Pointers can not be deserialized." );
		const size_t Size = sizeof( T );
		Data.read( reinterpret_cast<char*>( &Object ), Size );

		if( !Data.good() )
		{
			Invalidate();
		}
	}

	void operator>>( char* Object )
	{
		if( Invalid )
			return;

		do
		{
			Data.read( Object, sizeof( char ) );
			Object++;
		} while( Object[-1] != '\0' );

		if( !Data.good() )
		{
			Invalidate();
		}
	}

	void operator>>( CData& Object )
	{
		if( Invalid )
			return;

		size_t Size = 0;
		Data.read( reinterpret_cast<char*>( &Size ), sizeof( size_t ) );

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
		ResetCursor();
		Data.read( Buffer, Size );
	}

	void Load( const char* Buffer, const size_t Size )
	{
		Data = std::stringstream();
		Data.write( Buffer, Size );
		ResetCursor();
	}

	void ResetCursor()
	{
		Data.seekg( std::ios::beg );
	}

	void Jump( const int Offset )
	{
		Data.seekg( Offset, std::ios::cur );
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
	}

private:
	std::stringstream Data;

	bool Invalid;
};
