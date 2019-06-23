// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Data.h"
#include <string>

struct FDataString
{
	FDataString()
	{
		Size = 0;
		Address = nullptr;
	}

	FDataString( const std::string& String )
	{
		Size = String.size();
		Address = new char[Size + 1];
		strcpy_s( Address, ( Size + 1 ) * sizeof( char ), String.c_str() );
	}

	~FDataString()
	{
		if( Address )
		{
			delete[] Address;
			Address = nullptr;
		}
	}

	size_t Size;
	char* Address;

	friend CData& operator<<( CData& Data, FDataString& String )
	{
		Data << String.Size;
		Data << static_cast<const char*>( String.Address );

		return Data;
	}

	friend CData& operator>>( CData& Data, FDataString& String )
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

	static void Encode( CData& Data, std::string& Object )
	{
		Data << FDataString( Object );
	}

	static void Decode( CData& Data, std::string& Object )
	{
		FDataString DataString;
		Data >> DataString;

		if( DataString.Address )
		{
			Object = DataString.Address;
		}
	}
};
