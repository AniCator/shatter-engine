// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "World.h"

#include <algorithm>

#include <Engine/Audio/SoLoudSound.h>
#include <Engine/Configuration/Configuration.h>
#include <Engine/Physics/Physics.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Sequencer/Sequencer.h>
#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/LightEntity/LightEntity.h>
#include <Engine/Utility/Chunk.h>

static const char WorldIdentifier[5] = "LLWF"; // Lofty Lagoon World Format
static const size_t WorldVersion = 0;

CWorld* CWorld::PrimaryWorld = nullptr;

CWorld::CWorld()
{
	ActiveLevel = nullptr;
	Physics = nullptr;
	Camera = nullptr;
}

CWorld::~CWorld()
{
	if( Physics )
		Physics->Destroy();

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

	LightEntity::Initialize();

	for( auto Level : Levels )
	{
		Level.Construct();
	}

	LightEntity::UploadToGPU();
}

void CWorld::Frame()
{
	for( auto Level : Levels )
	{
		Level.Frame();
	}

	LightEntity::Bind();
}

void CWorld::Tick()
{
	for( auto Level : Levels )
	{
		Level.Tick();
	}

	if( Camera )
	{
		auto& Configuration = CConfiguration::Get();
		auto& Setup = Camera->GetCameraSetup();
		auto Width = Configuration.GetInteger( "width" );
		auto Height = Configuration.GetInteger( "height" );
		Setup.AspectRatio = static_cast<float>( Width ) / static_cast<float>( Height );

		Vector3D Velocity = Vector3D::Zero;
		if( Camera == PreviousCamera )
		{
			Velocity = Setup.CameraPosition - CameraPosition;
		}

		CameraPosition = Setup.CameraPosition;
		
		CSoLoudSound::SetListenerPosition( Setup.CameraPosition );
		CSoLoudSound::SetListenerDirection( Setup.CameraDirection );
		CSoLoudSound::SetListenerUpDirection( Setup.CameraUpVector );
		CSoLoudSound::SetListenerVelocity( Velocity );
	}

	PreviousCamera = Camera;

	if( Physics )
	{
		Physics->Tick();
	}
}

void CWorld::Destroy()
{
	Physics->Destroy();

	if( PrimaryWorld == this )
	{
		PrimaryWorld = nullptr;
		
		CSoLoudSound::StopSounds();

		// Prevent sequences from playing when the primary world is destroyed.
		const auto& Sequences = CAssets::Get().GetSequences();
		for( const auto& Sequence : Sequences )
		{
			if( Sequence.second && Sequence.second->Playing() )
			{
				Sequence.second->Stop();
			}
		}
	}
	
	Camera = nullptr;

	for( auto Level : Levels )
	{
		Level.Destroy();
	}
}

void CWorld::Reload()
{
	for( auto Level : Levels )
	{
		Level.Reload();
	}
}

CLevel& CWorld::Add()
{
	Levels.emplace_back( CLevel( this ) );
	ActiveLevel = &Levels[0];

	const auto LevelIndex = Levels.size() - 1;
	return Levels[LevelIndex];
}

CEntity* CWorld::Find( const std::string& Name ) const
{
	for( const auto& Level : Levels )
	{
		CEntity* Entity = Level.Find( Name );
		if( Entity )
		{
			return Entity;
		}
	}

	return nullptr;
}

CEntity* CWorld::Find( const size_t& ID ) const
{
	for( const auto& Level : Levels )
	{
		CEntity* Entity = Level.Find( ID );
		if( Entity )
		{
			return Entity;
		}
	}

	return nullptr;
}

CEntity* CWorld::Find( const EntityUID& ID ) const
{
	for( const auto& Level : Levels )
	{
		CEntity* Entity = Level.Find( ID );
		if( Entity )
		{
			return Entity;
		}
	}

	return nullptr;
}

CEntity* CWorld::Find( const UniqueIdentifier& Identifier ) const
{
	for( const auto& Level : Levels )
	{
		CEntity* Entity = Level.Find( Identifier );
		if( Entity )
		{
			return Entity;
		}
	}

	return nullptr;
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

CPhysics* CWorld::GetPhysics() const
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
	Chunk Chunk( "WORLD" );
	Chunk.Data << WorldIdentifier;
	Chunk.Data << WorldVersion;

	std::string Timestamp( __TIME__ );
	Timestamp.erase( std::remove_if( Timestamp.begin(), Timestamp.end(),
		[] ( const char a_c ) { 
			return !isdigit( a_c ); 
		} ),
		Timestamp.end() );

	const int StampInteger = std::stoi( Timestamp );
	Chunk.Data << StampInteger;

	const size_t Count = World->Levels.size();
	Chunk.Data << Count;

	for( size_t Index = 0; Index < Count; Index++ )
	{
		Chunk.Data << World->Levels[Index];
	}

	Data << Chunk;

	return Data;
}

CData& operator>>( CData& Data, CWorld* World )
{
	Chunk Chunk( "WORLD" );
	Data >> Chunk;

	char Identifier[5];
	Chunk.Data >> Identifier;

	size_t Version;
	Chunk.Data >> Version;

	std::string Timestamp( __TIME__ );
	Timestamp.erase( std::remove_if( Timestamp.begin(), Timestamp.end(),
		[] ( const char a_c ) {
			return !isdigit( a_c );
		} ),
		Timestamp.end() );

	const int StampInteger = std::stoi( Timestamp );
	int ExtractedStamp;
	Chunk.Data >> ExtractedStamp;

	const bool FromCurrentBuild = true; // StampInteger == ExtractedStamp;

	if( strcmp( Identifier, WorldIdentifier ) == 0 && Version >= WorldVersion && FromCurrentBuild )
	{
		size_t Count;
		Chunk.Data >> Count;

		for( size_t Index = 0; Index < Count; Index++ )
		{
			CLevel Level( World );
			Chunk.Data >> Level;
			World->Levels.push_back( Level );
		}

		if( !World->Levels.empty() )
		{
			World->ActiveLevel = &World->Levels[0];
		}

		for( auto& Level : World->Levels )
		{
			for( auto* Entity : Level.GetEntities() )
			{
				Entity->SetLevel( &Level );
			}
		}

		if( World->ActiveLevel->IsTemporary() )
		{
			World->ActiveLevel->Reload();
		}
	}
	else
	{
		Data.Invalidate();
	}

	return Data;
}
