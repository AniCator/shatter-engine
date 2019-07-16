// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "World.h"

#include <algorithm>

#include <Engine/Physics/Physics.h>
#include <Engine/World/Entity/Entity.h>

static const char WorldIdentifier[5] = "LLWF"; // Lofty Lagoon World Format
static const size_t WorldVersion;

CWorld* CWorld::PrimaryWorld = nullptr;

CWorld::CWorld()
{
	ActiveLevel = nullptr;
	Physics = nullptr;
}

CWorld::~CWorld()
{
	delete Physics;
	Physics = nullptr;
}

void CWorld::Construct()
{
	Camera = nullptr;

	if( Physics )
	{
		delete Physics;
		Physics = nullptr;
	}

	Physics = new CPhysics();

	if( Physics )
	{
		Physics->Construct();
	}

	for( auto Level : Levels )
	{
		Level.Construct();
	}
}

void CWorld::Frame()
{
	for( auto Level : Levels )
	{
		Level.Frame();
	}
}

void CWorld::Tick()
{
	CameraPriority = 0;

	if( Physics )
	{
		Physics->Tick();
	}

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

	Physics->Destroy();
}

CLevel& CWorld::Add()
{
	Levels.push_back( CLevel( this ) );
	ActiveLevel = &Levels[0];

	auto LevelIndex = Levels.size() - 1;
	return Levels[LevelIndex];
}

void CWorld::SetActiveCamera( CCamera* CameraIn, uint32_t Priority )
{
	if( CameraIn == nullptr || CameraPriority <= Priority )
	{
		Camera = CameraIn;
		CameraPriority = Priority;
	}
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

CPhysics* CWorld::GetPhysics()
{
	return Physics;
}

void CWorld::MakePrimary()
{
	PrimaryWorld = this;
}

CWorld* CWorld::GetPrimaryWorld()
{
	return PrimaryWorld;
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
