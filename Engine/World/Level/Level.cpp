// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Level.h"

static const size_t LevelVersion = 0;

CLevel::CLevel()
{

}

CLevel::~CLevel()
{
	
}

void CLevel::Construct()
{
	for( auto Entity : Entities )
	{
		Entity->Construct();
	}
}

void CLevel::Tick()
{
	for( auto Entity : Entities )
	{
		Entity->Tick();
	}
}

void CLevel::Destroy()
{
	for( auto Entity : Entities )
	{
		Entity->Destroy();
		delete Entity;
		Entity = nullptr;
	}
}

CData& operator<<( CData& Data, CLevel& Level )
{
	Data << LevelVersion;

	const size_t Count = Level.Entities.size();
	Data << Count;

	for( size_t Index = 0; Index < Count; Index++ )
	{
		Data << *Level.Entities[Index];
	}

	return Data;
}

CData& operator>> ( CData& Data, CLevel& Level )
{
	size_t Version;
	Data >> Version;

	if( Version >= LevelVersion )
	{
		size_t Count;
		Data >> Count;

		Level.Entities.reserve( Count );

		for( size_t Index = 0; Index < Count; Index++ )
		{
			/*CEntity* Entity = new CEntity();
			Data >> Entity;
			Level.Entities.push_back( Entity );*/
		}
	}

	return Data;
}
