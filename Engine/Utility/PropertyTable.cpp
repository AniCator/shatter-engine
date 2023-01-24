// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PropertyTable.h"

#include <Engine/Utility/Data.h>
#include <Engine/Utility/DataString.h>

static const Property VoidProperty;
PropertyTable::Query PropertyTable::Get( const std::string& Key, const PropertyType& Type ) const
{
	const auto& Iterator = Properties.find( Key );
	if( Iterator != Properties.end() )
	{
		Query Query;
		Query.Value = Iterator->second;
		Query.Valid = Type == PropertyType::Unknown || Type == Query.Value.GetType();

		return Query;
	}

	Query Query;
	Query.Value = VoidProperty;
	return Query;
}

void PropertyTable::Set( const std::string& Key, const Property& Value )
{
	Properties.insert_or_assign( Key, Value );
}

bool PropertyTable::Has( const std::string& Key, const PropertyType& Type ) const
{
	const auto Iterator = Properties.find( Key );
	const auto Exists = Iterator != Properties.end();
	const auto TypeMatch = Type == PropertyType::Unknown || Iterator->second.GetType() == Type;
	return Exists && TypeMatch;
}

CData& operator<<( CData& Data, const PropertyTable& Block )
{
	const auto StateEntries = static_cast<uint32_t>( Block.Properties.size() );
	Data << StateEntries;

	for( auto& State : Block.Properties )
	{
		DataString::Encode( Data, State.first );
		Data << State.second;
	}

	return Data;
}

CData& operator>>( CData& Data, PropertyTable& Block )
{
	uint32_t StateEntries = 0;
	Data >> StateEntries;

	for( uint32_t Index = 0; Index < StateEntries; Index++ )
	{
		std::string State;
		DataString::Decode( Data, State );

		Property Value;
		Data >> Value;

		Block.Set( State, Value );
	}

	return Data;
}