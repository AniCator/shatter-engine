// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>

template<typename Key, typename Value>
struct Translate
{
	Translate( const std::unordered_map<Key, Value>& Table )
	{
		FromKey = Table;

		for( auto& Pair : FromKey )
		{
			ToKey.insert_or_assign( Pair.second, Pair.first );
		}
	}

	Key From( const Value& Value )
	{
		auto Iterator = ToKey.find( Value );
		if( Iterator != ToKey.end() )
		{
			return ( *Iterator ).second;
		}

		return Key();
	}

	Value To( const Key& Key )
	{
		auto Iterator = FromKey.find( Key );
		if( Iterator != FromKey.end() )
		{
			return ( *Iterator ).second;
		}

		return Value();
	}

	std::unordered_map<Key, Value> FromKey;
	std::unordered_map<Value, Key> ToKey;
};
