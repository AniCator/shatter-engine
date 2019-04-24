// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "World.h"

#include <Engine/World/Entity/Entity.h>

static const char WorldIdentifier[5] = "LLWF"; // Lofty Lagoon World Format
static const size_t WorldVersion = 1;

CWorld::CWorld()
{
	ActiveLevel = nullptr;
}

CWorld::~CWorld()
{

}

void CWorld::Construct()
{
	for( auto Level : Levels )
	{
		Level.Level.Construct();
	}
}

void CWorld::Tick()
{
	for( auto Level : Levels )
	{
		Level.Level.Tick();
	}
}

void CWorld::Destroy()
{
	for( auto Level : Levels )
	{
		Level.Level.Destroy();
	}
}

void CWorld::Add( FLevel& Level )
{
	Levels.push_back( Level );
	ActiveLevel = &Levels[0].Level;

	auto& Back = Levels.back();
	for( auto Entity : Back.Level.GetEntities() )
	{
		Entity->SetLevel( &Back.Level );
	}
}

CData& operator<<( CData& Data, CWorld& World )
{
	Data << WorldIdentifier;
	Data << WorldVersion;

	const size_t Count = World.Levels.size();
	Data << Count;

	for( size_t Index = 0; Index < Count; Index++ )
	{
		Data << World.Levels[Index];
	}

	return Data;
}

CData& operator>>( CData& Data, CWorld& World )
{
	char Identifier[4];
	Data >> Identifier;

	size_t Version;
	Data >> Version;

	if( strcmp( Identifier, WorldIdentifier ) == 0 && Version >= WorldVersion )
	{
		size_t Count;
		Data >> Count;

		World.Levels.reserve( Count );

		for( size_t Index = 0; Index < Count; Index++ )
		{
			FLevel Level;
			Data >> Level;
			World.Levels.push_back( Level );
		}

		if( World.Levels.size() > 0 )
		{
			World.ActiveLevel = &World.Levels[0].Level;
		}
	}
	else
	{
		Data.Invalidate();
	}

	return Data;
}
