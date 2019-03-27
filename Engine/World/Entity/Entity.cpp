// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Entity.h"

void CEntityMap::Add( const std::string& Type, EntityFunction Function )
{
	Log::Event( "Registering entity \"%s\".\n", Type.c_str() );
	Map[Type] = Function;
}

EntityFunction CEntityMap::Find( const std::string& Type )
{
	if( Map.find( Type ) != Map.end() )
	{
		return Map[Type];
	}

	return [] () {return nullptr; };
}
