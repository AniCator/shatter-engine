// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Data.h"
#include "DataString.h"

#include <set>
#include <string>

// Serialization helpers that assist in easy data import and export.
struct Serialize
{
	// Adds a marker for verification purposes and exports the given data (pointers will assert).
	template<typename T>
	static void Export( CData& Data, const char* Identifier, const T& Snippet )
	{
		static_assert( !std::is_pointer<T>::value, "Pointers can not be serialized directly." );
		DataMarker::Mark( Data, Identifier );
		Data << Snippet;
	}

	// Checks if the given marker is found and extracts the associated data.
	template<typename T>
	static void Import( CData& Data, const char* Identifier, T& Snippet )
	{
		if( !DataMarker::Check( Data, Identifier ) )
			return;
		
		Data >> Snippet;
	}

	static void Export( CData& Data, const char* Identifier, const std::set<std::string>& Set )
	{
		DataMarker::Mark( Data, Identifier );
		
		uint32_t Size = Set.size();
		Data << Size;

		for( const auto& Item : Set )
		{
			DataString::Encode( Data, Item );
		}
	}

	static void Import( CData& Data, const char* Identifier, std::set<std::string>& Set )
	{
		if( !DataMarker::Check( Data, Identifier ) )
			return;

		Set.clear();
		
		uint32_t Size = 0;
		Data >> Size;
		for( uint32_t Index = 0; Index < Size; Index++ )
		{
			std::string Item;
			DataString::Decode( Data, Item );
			Set.insert( Item );
		}
	}

	static void Export( CData& Data, const char* Identifier, const std::vector<std::string>& Vector )
	{
		DataMarker::Mark( Data, Identifier );

		uint32_t Size = Vector.size();
		Data << Size;

		for( const auto& Item : Vector )
		{
			DataString::Encode( Data, Item );
		}
	}

	static void Import( CData& Data, const char* Identifier, std::vector<std::string>& Vector )
	{
		if( !DataMarker::Check( Data, Identifier ) )
			return;

		Vector.clear();

		uint32_t Size = 0;
		Data >> Size;
		for( uint32_t Index = 0; Index < Size; Index++ )
		{
			std::string Item;
			DataString::Decode( Data, Item );
			Vector.emplace_back( Item );
		}
	}

	static void Export( CData& Data, const char* Identifier, const std::string& Snippet )
	{
		DataMarker::Mark( Data, Identifier );
		DataString::Encode( Data, Snippet );
	}

	static void Import( CData& Data, const char* Identifier, std::string& Snippet )
	{
		if( !DataMarker::Check( Data, Identifier ) )
			return;

		DataString::Decode( Data, Snippet );
	}
};