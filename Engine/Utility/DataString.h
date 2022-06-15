// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Data.h"
#include <string>

struct DataMarker
{
	static void Mark( CData& Data, const char* Identifier )
	{
		const size_t Length = std::strlen( Identifier );
		for( size_t Index = 0; Index < Length; Index++ )
		{
			Data << Identifier[Index];
		}
	}

	static bool Check( CData& Data, const char* Identifier )
	{
		const auto ReadPosition = Data.ReadPosition();
		const size_t Length = std::strlen( Identifier );
		for( size_t Index = 0; Index < Length; Index++ )
		{
			char Extracted;
			Data >> Extracted;
			if( Extracted != Identifier[Index] )
			{
				// Reset the read position if we didn't find the marker.
				Data.ReadPosition( static_cast<int32_t>( ReadPosition ) );
				return false;
			}
		}
		
		return true;
	}
};

struct DataString
{
	DataString()
	{
		Size = 0;
		Address = nullptr;
	}

	DataString( const std::string& String )
	{
		Size = String.size();
		Address = new char[Size + 1];
		strcpy_s( Address, ( Size + 1 ) * sizeof( char ), String.c_str() );
	}

	~DataString()
	{
		if( Address )
		{
			delete[] Address;
			Address = nullptr;
		}
	}

	uint32_t Size;
	char* Address;

	friend CData& operator<<( CData& Data, const DataString& String )
	{
		Data << String.Size;
		Data << static_cast<const char*>( String.Address );

		return Data;
	}

	friend CData& operator>>( CData& Data, DataString& String )
	{
		const auto Result = Data >> String.Size;

		// If the returned size is 0 this means we failed to extract the string size.
		if( Result == 0 )
		{
			return Data;
		}

		if( String.Size > 100000 )
		{
			Data.Invalidate();
			return Data;
		}

		String.Address = new char[String.Size + 1];
		Data >> String.Address;

		return Data;
	}

	static void Encode( CData& Data, const std::string& Object )
	{
		Data << DataString( Object );
	}

	static void Decode( CData& Data, std::string& Object )
	{
		DataString DataString;
		Data >> DataString;

		if( Data.Valid() && DataString.Address )
		{
			Object = DataString.Address;
		}
	}
};

struct DataVector
{
	template<typename T>
	static void Encode( CData& Data, const std::vector<T>& Vector )
	{
		const uint32_t Count = Vector.size();
		Data << Count;

		for( uint32_t Index = 0; Index < Count; Index++ )
		{
			Data << Vector[Index];
		}
	}

	template<typename T>
	static void Decode( CData& Data, std::vector<T>& Vector )
	{
		uint32_t ItemCount;
		Data >> ItemCount;

		Vector.reserve( ItemCount );
		for( uint32_t Index = 0; Index < ItemCount; Index++ )
		{
			typedef std::remove_pointer<T>::type U;
			U Item = U();
			Data >> Item;
			Vector.emplace_back( Item );
		}
	}
};
