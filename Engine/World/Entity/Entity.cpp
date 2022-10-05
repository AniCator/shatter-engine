// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Entity.h"

#include <Game/Game.h>

#include <Engine/Configuration/Configuration.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/World/Level/Level.h>
#include <Engine/World/World.h>

#include <random>
#include <iomanip>

ConVar<bool> DebugEntityIO( "debug.Entity.IO", false );

void CEntityMap::Add( const std::string& Type, EntityFunction Factory )
{
	// Log::Event( "Registering entity \"%s\".\n", Type.c_str() );
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

CEntity::CEntity()
{
	Level = nullptr;
	Parent = nullptr;

	Enabled = false;
	ShouldDebug = false;

	ClassName = "-";
}

CEntity::~CEntity()
{
	
}

void CEntity::SetEntityID( const EntityUID& EntityID )
{
	ID = EntityID;
}

const EntityUID& CEntity::GetEntityID() const
{
	return ID;
}

void CEntity::SetLevelID( const LevelUID& EntityID )
{
	LevelID = EntityID;
}

const LevelUID& CEntity::GetLevelID() const
{
	return LevelID;
}

void CEntity::SetLevel( CLevel* SpawnLevel )
{
	Level = SpawnLevel;
}

CLevel* CEntity::GetLevel() const
{
	return Level;
}

CWorld* CEntity::GetWorld() const
{
	return Level->GetWorld();
}

void CEntity::SetParent( CEntity* Entity )
{
	// Don't do anything if we're already parented to this entity.
	if( Parent == Entity )
		return;

	// Check if we have a former parent.
	if( Parent )
	{
		// Remove this entity from the former parent.
		const auto Iterator = std::find( Parent->Children.begin(), Parent->Children.end(), this );
		if( Iterator != Parent->Children.end() )
		{
			Parent->Children.erase( Iterator );
		}
	}

	Parent = Entity;

	if( Parent )
	{
		// Assign this entity as a child.
		Parent->Children.emplace_back( this );
		ParentName = Parent->Name.String();
	}
	else
	{
		ParentName = std::string();
	}
}

CEntity* CEntity::GetParent() const
{
	return Parent;
}

void CEntity::Construct()
{
	// Register loaded tags.
	if( auto* World = GetWorld() )
	{
		for( auto& Tag : Tags )
		{
			World->Tag( this, Tag );
		}
	}
}

void CEntity::Destroy()
{
	// Unregister any remaining tags.
	if( auto* World = GetWorld() )
	{
		for( auto& Tag : Tags )
		{
			World->Untag( this, Tag );
		}
	}

	if( !Level )
	{
		Log::Event( Log::Error, "Orphaned entity requesting destruction (%s)\n", Name.String().c_str() );
		return;
	}

	// Check which entities can call methods on this entity, and remove those outputs.
	for( const auto& TrackedEntityID : TrackedEntityIDs )
	{
		auto* Entity = Level->Find( TrackedEntityID );
		if( !Entity )
			continue;
		
		Entity->Unlink( GetEntityID() );
	}

	// Delete the entity from the level.
	Level->Remove( this );
}

void CEntity::Traverse()
{
	const auto* World = GetWorld();

	// Initialize the parent if we deserialized a parent name but didn't assign the parent yet.
	if( ParentName.length() > 0 && !Parent && World )
	{
		if( auto* Entity = World->Find( ParentName ) )
		{
			SetParent( Entity );
		}
		else
		{
			// Clear the parent name to avoid retries.
			ParentName = std::string();
		}
	}

	const auto CurrentTime = GameLayersInstance->GetCurrentTime();
	const bool ShouldTick = NextTickTime < CurrentTime;
	if( ShouldTick )
	{
		// OptickEvent();

		if( Math::Equal( LastTickTime, 0.0 ) )
		{
			LastTickTime = CurrentTime;
		}

		if( NextTickTime > 0.0 )
		{
			// Invalidate the next tick time when it has previously been set by the user.
			NextTickTime = FLT_MAX;
		}

		// Update this entity's delta time.
		DeltaTime = CurrentTime - LastTickTime;

		// Call the entity's tick function. (may be overridden)
		Tick();

		LastTickTime = CurrentTime;
	}

	// Display debug information, if requested.
	if( IsDebugEnabled() )
	{
		Debug();
	}

	// Traverse through the children of the entity.
	const auto HasChildren = !Children.empty();
	if( HasChildren )
	{
		for( auto* Child : Children )
		{
			Child->Traverse();
		}
	}
}

void CEntity::Link( const JSON::Vector& Objects )
{
	if( !Level )
		return;

	for( auto Property : Objects )
	{
		if( Property->Key != "outputs" )
			continue;
		
		for( auto Output : Property->Objects )
		{
			std::string OutputName;
			std::string TargetName;
			std::string InputName;

			for( auto OutputProperty : Output->Objects )
			{
				if( OutputProperty->Key == "name" )
				{
					OutputName = OutputProperty->Value;
				}
				else if( OutputProperty->Key == "target" )
				{
					TargetName = OutputProperty->Value;
				}
				else if( OutputProperty->Key == "input" )
				{
					InputName = OutputProperty->Value;
				}
			}

			if( OutputName.length() > 0 && TargetName.length() > 0 && InputName.length() > 0 )
			{
				auto Entity = Level->GetWorld()->Find( TargetName );
				if( Entity )
				{
					FMessage Message;
					Message.TargetID = Entity->GetEntityID();
					Message.TargetName = TargetName;
					Message.Inputs.emplace_back( InputName );

					auto& Messsages = Outputs[OutputName];
					Messsages.emplace_back( Message );
				}
				else
				{
					Log::Event( Log::Warning, "Target entity \"%s\" not found for entity \"%s\".\n", TargetName.c_str(), Name.String().c_str() );
				}
			}
		}
	}
}

void CEntity::Relink()
{
	for( auto& Output : Outputs )
	{
		for( auto& Message : Output.second )
		{
			auto* Entity = Level->Find( Message.TargetName );
			if( Entity )
			{
				Message.TargetID = Entity->GetEntityID();
			}
			else
			{
				Log::Event( Log::Warning, "Target entity \"%s\" not found for entity \"%s\".\n", Message.TargetName.c_str(), Name.String().c_str() );
			}
		}
	}
}

void CEntity::Send( const char* Output, CEntity* Origin )
{
	if( !Level )
		return;

	const auto Iterator = Outputs.find( Output );
	if( Iterator == Outputs.end() )
		return;

	if( DebugEntityIO )
		Log::Event( "Broadcasting output \"%s\".\n", Output );

	for( auto& Message : Outputs[Output] )
	{
		const auto Entity = Level->GetWorld()->Find( Message.TargetID );
		if( !Entity )
			continue;

		for( const auto& Input : Message.Inputs )
		{
			Entity->Receive( Input.c_str(), Origin );
		}
	}
}

bool CEntity::Receive( const char* Input, CEntity* Origin )
{
	const auto Iterator = Inputs.find( Input );
	if( Iterator == Inputs.end() )
		return true; // By default we pretend we were succesful, even if the input wasn't found.

	if( DebugEntityIO )
	{
		if( Origin )
		{
			Log::Event( "Receiving input \"%s\" on entity \"%s\" from \"%s\".\n", Input, Name.String().c_str(), Origin->Name.String().c_str() );
		}
		else
		{
			Log::Event( "Receiving input \"%s\" on entity \"%s\" from unknown source.\n", Input, Name.String().c_str() );
		}
	}

	return ( *Iterator ).second( Origin );
}

void CEntity::Track( const CEntity* Entity )
{
	if( Entity )
	{
		TrackedEntityIDs.emplace_back( Entity->GetEntityID().ID );
	}
}

void CEntity::Unlink( const EntityUID EntityID )
{
	// If this entity ID is associated with any message outputs, remove those messages to prevent them from being called on removed entities.
	for( auto& Output : Outputs )
	{
		for( auto MessageIterator = Output.second.begin(); MessageIterator != Output.second.end();)
		{
			if( MessageIterator->TargetID == EntityID )
			{
				MessageIterator = Output.second.erase( MessageIterator );
			}
			else
			{
				++MessageIterator;
			}			
		}
	}
}

void CEntity::Debug()
{
	if( ShouldDebug )
	{
		// 
	}
}

void CEntity::EnableDebug( const bool Enable )
{
	ShouldDebug = Enable;
}

void CEntity::Tag( const std::string& TagName )
{
	if( auto* World = GetWorld() )
	{
		World->Tag( this, TagName );

		Tags.insert( TagName );
	}
}

void CEntity::Untag( const std::string& TagName )
{
	if( auto* World = GetWorld() )
	{
		World->Untag( this, TagName );

		Tags.erase( std::find( Tags.begin(), Tags.end(), TagName ) );
	}
}

double CEntity::GetCurrentTime()
{
	return GameLayersInstance->GetCurrentTime();
}

CData& operator<<( CData& Data, CEntity* Entity )
{
	DataString::Encode( Data, Entity->ClassName );
	DataString::Encode( Data, Entity->Name.String() );

	Data << Entity->Identifier.ID;

	if( Entity->Parent )
	{
		const auto ParentName = Entity->Parent->Name.String();
		if( ParentName.length() > 0 )
		{
			Serialize::Export( Data, "pt", ParentName );
		}
	}

	if( !Entity->Tags.empty() )
	{
		Serialize::Export( Data, "tg", Entity->Tags );
	}

	const auto OutputCount = Entity->Outputs.size();
	Data << OutputCount;
	if( OutputCount > 0 )
	{
		for( auto& Output : Entity->Outputs )
		{
			DataString::Encode( Data, Output.first.String() );

			const auto MessageCount = Output.second.size();
			Data << MessageCount;
			for( auto& Message : Output.second )
			{
				DataString::Encode( Data, Message.TargetName );

				const auto InputCount = Message.Inputs.size();
				Data << InputCount;
				for( auto& Input : Message.Inputs )
				{
					DataString::Encode( Data, Input );
				}
			}
		}
	}

	Entity->Export( Data );

	return Data;
}

CData& operator>>( CData& Data, CEntity* Entity )
{
	if( !Entity )
		return Data;
	
	Serialize::Import( Data, "pt", Entity->ParentName );
	Serialize::Import( Data, "tg", Entity->Tags );

	size_t OutputCount;
	Data >> OutputCount;

	for( size_t OutputIndex = 0; OutputIndex < OutputCount; OutputIndex++ )
	{
		std::string OutputName;
		DataString::Decode( Data, OutputName );
		size_t MessageCount;
		Data >> MessageCount;
		for( size_t MessageIndex = 0; MessageIndex < MessageCount; MessageIndex++ )
		{
			std::string TargetName;
			DataString::Decode( Data, TargetName );

			size_t InputCount;
			Data >> InputCount;
			for( size_t InputIndex = 0; InputIndex < InputCount; InputIndex++ )
			{
				std::string InputName;
				DataString::Decode( Data, InputName );

				FMessage Message;
				Message.TargetID = EntityUID::None();
				Message.TargetName = TargetName;
				Message.Inputs.emplace_back( InputName );

				auto& Messsages = Entity->Outputs[OutputName];
				Messsages.emplace_back( Message );
			}
		}
	}

	Entity->Import( Data );
	
	return Data;
}

EntityUID EntityUID::Create()
{
	static size_t GlobalID = 0;
	EntityUID ID;
	ID.ID = ++GlobalID;
	return ID;
}

EntityUID EntityUID::None()
{
	static EntityUID NoneID;
	return NoneID;
}

std::string GenerateHex( const int& Length )
{
	// Not that random but random enough.
	static std::random_device RandomDevice;
	static std::mt19937 Generator( RandomDevice() );
	static std::uniform_int_distribution<> Distribution( 0, 15 );
	std::stringstream Stream;
	
	for( int Index = 0; Index < Length; Index++ )
	{
		Stream << std::hex << std::setw( 2 ) << std::setfill( '0' );
		Stream << Distribution( Generator );
	}

	return Stream.str();
}

void UniqueIdentifier::Random()
{
	std::string Hex;
	Hex.reserve( 36 );
	
	Hex += GenerateHex( 4 );
	Hex += "-";
	Hex += GenerateHex( 2 );
	Hex += "-";
	Hex += GenerateHex( 2 );
	Hex += "-";
	Hex += GenerateHex( 2 );
	Hex += "-";
	Hex += GenerateHex( 6 );

	Set( Hex.c_str() );
}

void UniqueIdentifier::Set( const char* Identifier )
{
	const auto Size = std::strlen( Identifier );
	if( Size == 36 )
	{
		strcpy_s( ID, Identifier );
	}
	else
	{
		Log::Event( Log::Warning, "Invalid unique identifier value.\n" );
	}
}

bool UniqueIdentifier::Valid() const
{
	static char Invalid[37] = "00000000-0000-0000-0000-000000000000";
	if( std::strcmp( ID, Invalid ) == 0 )
		return false;

	const auto Size = std::strlen( ID );
	if( Size != 36 )
	{
		return false;
	}

	return true;
}

bool UniqueIdentifier::operator==( const UniqueIdentifier& B )
{
	if( std::strcmp( ID, B.ID ) == 0 )
		return true;

	return false;
}

CData& operator<<( CData& Data, const UniqueIdentifier& Identifier )
{
	DataString::Encode( Data, Identifier.ID );

	return Data;
}

CData& operator>>( CData& Data, UniqueIdentifier& Identifier )
{
	std::string ID;
	DataString::Decode( Data, ID );	
	strcpy_s( Identifier.ID, ID.c_str() );

	return Data;
}
