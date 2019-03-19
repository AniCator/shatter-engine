// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <sstream>

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
	}

	void operator<<( const char* Object )
	{
		const size_t Size = strlen( Object ) * sizeof( char );
		Data.write( Object, Size + 1 );
	}

	template<typename T>
	void operator>> ( T& Object )
	{
		const size_t Size = sizeof( T );
		Data.read( reinterpret_cast<char*>( &Object ), Size );
	}

	void Store( char* Buffer, const size_t Size )
	{
		Data.seekg( std::ios::beg );
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
