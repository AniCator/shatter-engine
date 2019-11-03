// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Data.h"
#include <string>

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

	size_t Size;
	char* Address;

	friend CData& operator<<( CData& Data, DataString& String )
	{
		Data << String.Size;
		Data << static_cast<const char*>( String.Address );

		return Data;
	}

	friend CData& operator>>( CData& Data, DataString& String )
	{
		Data >> String.Size;

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

struct FDataVector
{
	template<typename T>
	static void Encode( CData& Data, std::vector<T>& Vector )
	{
		const size_t Count = Vector.size();
		Data << Count;

		for( size_t Index = 0; Index < Count; Index++ )
		{
			Data << Vector[Index];
		}
	}

	template<typename T>
	static void Decode( CData& Data, std::vector<T>& Vector )
	{
		size_t ItemCount;
		Data >> ItemCount;

		Vector.reserve( ItemCount );
		for( size_t Index = 0; Index < ItemCount; Index++ )
		{
			typedef std::remove_pointer<T>::type U;
			U Item = U();
			Data >> Item;
			Vector.emplace_back( Item );
		}
	}
};
