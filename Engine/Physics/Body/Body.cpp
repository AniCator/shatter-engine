// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Body.h"

static struct BodyConverter
{
	BodyConverter()
	{
		FromString = {
			{ "triangle", BodyType::TriangleMesh },
			{ "plane", BodyType::Plane }
		};

		for( auto& Pair : FromString )
		{
			ToString.insert_or_assign( Pair.second, Pair.first );
		}
	}

	std::map<std::string, BodyType>  FromString;
	std::map<BodyType, std::string> ToString;
} Convert;

BodyType ToBodyType( const std::string& Type )
{
	auto Iterator = Convert.FromString.find( Type );
	if( Iterator != Convert.FromString.end() )
	{
		return ( *Iterator ).second;
	}
	else
	{
		return BodyType::TriangleMesh;
	}
}

const std::string& FromBodyType( const BodyType& Type )
{
	auto Iterator = Convert.ToString.find( Type );
	if( Iterator != Convert.ToString.end() )
	{
		return ( *Iterator ).second;
	}
	else
	{
		return "triangle";
	}
}