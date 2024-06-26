// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "World.h"

#include <algorithm>

#include <Engine/Audio/SoLoudSound.h>
#include <Engine/Configuration/Configuration.h>
#include <Engine/Physics/Physics.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Sequencer/Sequencer.h>
#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/LightEntity/LightEntity.h>
#include <Engine/World/Entity/Node/Node.h>
#include <Engine/Utility/Chunk.h>
#include <Engine/Utility/ThreadPool.h>

#include <Game/Game.h>

ConfigurationVariable<bool> DisplayNetworks( "debug.Navigation.Show", false );

static const char WorldIdentifier[5] = "LLWF"; // Lofty Lagoon World Format
static const size_t WorldVersion = 0;

// Deletes the previous instance if it exists, and creates a new one.
template<typename T>
void Create( T*& X )
{
	if( X )
	{
		delete X;
	}

	X = new T();
}

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
	delete Navigation;
}

void CWorld::Construct()
{
	OptickEvent();

	Camera = nullptr;
	// Tags.clear();

	Create( Physics );
	Create( Navigation );

	LightEntity::Initialize();

	for( auto& Level : Levels )
	{
		Level.Construct();
	}

	LightEntity::UploadToGPU();

	// Warm up the physics simulation.
	if( Physics )
	{
		Physics->Warm( GameLayersInstance->GetCurrentTime(), 10 );
		WaitingForPhysics = true;
	}
}

void CWorld::Frame()
{
	OptickEvent();

	for( auto& Level : Levels )
	{
		if( !Level.IsVisible() )
			continue;

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

	WaitingForPhysics = false;

	if( DisplayNetworks )
	{
		Navigation->Debug();
	}

	EventQueue.Poll();

	for( auto& Level : Levels )
	{
		Level.Tick();
	}

	{
		OptickEvent( "PostTick" );
		for( auto& Level : Levels )
		{
			Level.PostTick();
		}
	}

	LightEntity::UploadToGPU();

	if( Camera )
	{
		auto& Configuration = CConfiguration::Get();
		auto& Setup = Camera->GetCameraSetup();
		auto Width = Configuration.GetInteger( "window.Width" );
		auto Height = Configuration.GetInteger( "window.Height" );
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

	if( Physics && TickPhysics )
	{
		Physics->Tick( GameLayersInstance->GetCurrentTime() );
		WaitingForPhysics = true;
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

	if( Physics )
		Physics->Destroy();

	for( auto& Level : Levels )
	{
		Level.Destroy();
	}

	Tags.clear();
}

void CWorld::Reload()
{
	LightEntity::Initialize();

	for( auto& Level : Levels )
	{
		Level.Reload();
	}

	LightEntity::UploadToGPU();
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

CEntity* CWorld::Find( const NameSymbol& Name ) const
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

	static FCameraSetup NullSetup;
	return NullSetup;
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

Navigation* CWorld::GetNavigation() const
{
	return Navigation;
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

void CWorld::Rebase( const FTransform& Transform )
{
	const auto Offset = Transform.GetPosition();
	for( auto& Level : Levels )
	{
		Level.Transform.SetPosition( Level.Transform.GetPosition() - Offset );
		Level.Transform.Update();
	}
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
			World->Levels.push_back( Level );
			Chunk.Data >> World->Levels.back();
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

		// TODO: Figure out why this checks serialization.
		if( !World->ActiveLevel->CanSerialize() )
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
