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

#include <Game/Game.h>

static const char WorldIdentifier[5] = "LLWF"; // Lofty Lagoon World Format
static const size_t WorldVersion = 0;

CWorld* CWorld::PrimaryWorld = nullptr;

CWorld::CWorld()
{
	ActiveLevel = nullptr;
	Physics = nullptr;
	Camera = nullptr;

	Tags = std::unordered_map<std::string, std::vector<CEntity*>>();
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
	OptickEvent();

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
	OptickEvent();

	for( auto Level : Levels )
	{
		Level.Frame();
	}

	LightEntity::Bind();
}

void CWorld::Tick()
{
	OptickEvent();

	// Make sure the physics scene has finished its tasks.
	if( Physics )
	{
		ProfileAlways( "Waiting for Physics" );
		OptickEvent( "Waiting for Physics" );
		Physics->Guard();
	}

	EventQueue.Poll();

	for( auto Level : Levels )
	{
		Level.Tick();
	}

	LightEntity::UploadToGPU();

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
		
		SoLoudSound::SetListenerPosition( Setup.CameraPosition );
		SoLoudSound::SetListenerDirection( Setup.CameraDirection );
		SoLoudSound::SetListenerUpDirection( Setup.CameraUpVector );
		SoLoudSound::SetListenerVelocity( Velocity );
	}

	PreviousCamera = Camera;

	if( Physics )
	{
		Physics->Tick( GameLayersInstance->GetCurrentTime() );
	}

	// Increment the tick counter.
	Ticks++;
}

void CWorld::Destroy()
{
	if( PrimaryWorld == this )
	{
		PrimaryWorld = nullptr;
		
		SoLoudSound::StopSounds();

		// Prevent sequences from playing when the primary world is destroyed.
		for( auto* Sequence : CAssets::Get().Sequences.GetAssets() )
		{
			if( Sequence && Sequence->Playing() )
			{
				Sequence->Stop();
			}
		}
	}
	
	Camera = nullptr;

	Physics->Destroy();

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

bool CWorld::Transfer( CEntity* Entity )
{
	if( !ActiveLevel )
	{
		Log::Event( Log::Error, "This world doesn't have an active level.\n" );
		return false;
	}

	if( !ActiveLevel->Transfer( Entity ) )
	{
		Log::Event(
			Log::Warning,
			"Failed to transfer entity %u \"%s\".\n",
			Entity->GetEntityID().ID,
			Entity->Name.String().c_str()
		);

		return false; // Failed to transfer the entity.
	}

	return true;
}

void CWorld::Transfer( const std::vector<CEntity*>& Entities )
{
	if( !ActiveLevel )
	{
		Log::Event( Log::Error, "This world doesn't have an active level.\n" );
		return;
	}

	for( auto* Entity : Entities )
	{
		if( !ActiveLevel->Transfer( Entity ) )
		{
			Log::Event( 
				Log::Warning, 
				"Failed to transfer entity %u \"%s\".\n", 
				Entity->GetEntityID().ID, 
				Entity->Name.String().c_str() 
			);
		}
	}
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

const FCameraSetup& CWorld::GetActiveCameraSetup() const
{
	if( Camera )
	{
		return Camera->GetCameraSetup();
	}

	return {};
}

uint32_t CWorld::GetCameraPriority() const
{
	return CameraPriority;
}

CCamera* CWorld::GetPreviousCamera() const
{
	return PreviousCamera;
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

void CWorld::Tag( CEntity* Entity, const std::string& TagName )
{
	auto Iterator = Tags.find( TagName );
	if( Iterator == Tags.end() )
	{
		// Create a new tag entry if it doesn't exist yet.
		Tags.insert_or_assign( TagName, std::vector<CEntity*>() );
		Iterator = Tags.find( TagName );
	}

	// Create just the tag category if a nullptr is given.
	if( Entity == nullptr )
		return;

	// Assign the entity to this tag category.
	Iterator->second.emplace_back( Entity );
}

void CWorld::Untag( CEntity* Entity, const std::string& TagName )
{
	for( auto TagIterator = Tags.begin(); TagIterator != Tags.end();)
	{
		if( TagIterator->first == TagName )
		{
			auto& Entities = TagIterator->second;
			auto Iterator = Entities.begin();
			while( Iterator != Entities.end() )
			{
				if( ( *Iterator ) == Entity )
				{
					Iterator = Entities.erase( Iterator );
				}
				else
				{
					++Iterator;
				}
			}
		}

		++TagIterator;
	}
}

const std::vector<CEntity*>* CWorld::GetTagged( const std::string& TagName ) const
{
	const auto Iterator = Tags.find( TagName );
	if( Iterator != Tags.end() )
	{
		return &Iterator->second;
	}

	return nullptr;
}

const std::unordered_map<std::string, std::vector<CEntity*>>& CWorld::GetTags() const
{
	return Tags;
}

CEntity* CWorld::Find( const std::string& TagName, const std::string& EntityName ) const
{
	const auto Iterator = Tags.find( TagName );
	if( Iterator != Tags.end() )
	{
		for( auto* Entity : Iterator->second )
		{
			if( Entity->Name == EntityName )
			{
				return Entity;
			}
		}
	}

	return nullptr;
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
		[] ( const char Character ) {
			return !isdigit( Character );
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
