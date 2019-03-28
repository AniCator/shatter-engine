// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Entity.h"

void CEntityMap::Add( const std::string& Type, EntityFunction Factory )
{
	Log::Event( "Registering entity \"%s\".\n", Type.c_str() );
	Map[Type] = Factory;
}

EntityFunction CEntityMap::Find( const std::string& Type )
{
	if( Map.find( Type ) != Map.end() )
	{
		return Map[Type];
	}

	return [] () {return nullptr; };
}
