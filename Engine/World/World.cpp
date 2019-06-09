// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "World.h"

#include <algorithm>

#include <Engine/World/Entity/Entity.h>

static const char WorldIdentifier[5] = "LLWF"; // Lofty Lagoon World Format
static const size_t WorldVersion;

CWorld::CWorld()
{
	ActiveLevel = nullptr;
}

CWorld::~CWorld()
{

}

void CWorld::Construct()
{
	Camera = nullptr;

	for( auto Level : Levels )
	{
		Level.Construct();
	}
}

void CWorld::Tick()
{
	for( auto Level : Levels )
	{
		Level.Tick();
	}
}

void CWorld::Destroy()
{
	Camera = nullptr;

	for( auto Level : Levels )
	{
		Level.Destroy();
	}
}

CLevel& CWorld::Add()
{
	Levels.push_back( CLevel( this ) );
	ActiveLevel = &Levels[0];

	auto LevelIndex = Levels.size() - 1;
	return Levels[LevelIndex];
}

void CWorld::SetActiveCamera( CCamera* CameraIn )
{
	Camera = CameraIn;
}

CCamera* CWorld::GetActiveCamera() const
{
	return Camera;
}

static const FCameraSetup DummySetup = FCameraSetup();
const FCameraSetup& CWorld::GetActiveCameraSetup() const
{
	if( Camera )
	{
		return Camera->GetCameraSetup();
	}

	return DummySetup;
}

CData& operator<<( CData& Data, CWorld* World )
{
	Data << WorldIdentifier;
	Data << WorldVersion;

	std::string Timestamp( __TIME__ );
	Timestamp.erase( std::remove_if( Timestamp.begin(), Timestamp.end(),
		[] ( const char a_c ) { 
			return !isdigit( a_c ); 
		} ),
		Timestamp.end() );

	const int StampInteger = std::stoi( Timestamp );
	Data << StampInteger;

	const size_t Count = World->Levels.size();
	Data << Count;

	for( size_t Index = 0; Index < Count; Index++ )
	{
		Data << World->Levels[Index];
	}

	return Data;
}

CData& operator>>( CData& Data, CWorld* World )
{
	char Identifier[5];
	Data >> Identifier;

	size_t Version;
	Data >> Version;

	std::string Timestamp( __TIME__ );
	Timestamp.erase( std::remove_if( Timestamp.begin(), Timestamp.end(),
		[] ( const char a_c ) {
			return !isdigit( a_c );
		} ),
		Timestamp.end() );

	const int StampInteger = std::stoi( Timestamp );
	int ExtractedStamp;
	Data >> ExtractedStamp;

	if( strcmp( Identifier, WorldIdentifier ) == 0 && Version >= WorldVersion && StampInteger == ExtractedStamp )
	{
		size_t Count;
		Data >> Count;

		for( size_t Index = 0; Index < Count; Index++ )
		{
			CLevel Level( World );
			Data >> Level;
			World->Levels.push_back( Level );
		}

		if( World->Levels.size() > 0 )
		{
			World->ActiveLevel = &World->Levels[0];
		}

		for( auto& Level : World->Levels )
		{
			for( auto Entity : Level.GetEntities() )
			{
				Entity->SetLevel( &Level );
			}
		}
	}
	else
	{
		Data.Invalidate();
	}

	return Data;
}
