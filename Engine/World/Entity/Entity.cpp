// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Entity.h"

#include <Engine/World/Level/Level.h>
#include <Engine/World/World.h>

#include <random>
#include <iomanip>

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
	Parent = Entity;
}

CEntity* CEntity::GetParent()
{
	return Parent;
}

void CEntity::Construct()
{

}

void CEntity::Destroy()
{
	if( Level )
	{
		for( const auto& TrackedEntityID : TrackedEntityIDs )
		{
			auto Entity = Level->Find( TrackedEntityID );
			if( Entity )
			{
				Entity->Unlink( GetEntityID() );
			}
		}

		Level->Remove( this );
	}
}

void CEntity::Link( const JSON::Vector& Objects )
{
	if( Level )
	{
		for( auto Property : Objects )
		{
			if( Property->Key == "outputs" )
			{
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

void CEntity::Send( const char* Output )
{
	if( Level )
	{
		if( Outputs.find( Output ) != Outputs.end() )
		{
			Log::Event( "Broadcasting output \"%s\".\n", Output );
			for( auto& Message : Outputs[Output] )
			{
				auto Entity = Level->GetWorld()->Find( Message.TargetID );
				if( Entity )
				{
					for( const auto& Input : Message.Inputs )
					{
						Entity->Receive( Input.c_str() );
					}
				}
			}
		}
	}
}

void CEntity::Receive( const char* Input )
{
	if( Inputs.find( Input ) != Inputs.end() )
	{
		Log::Event( "Receiving input \"%s\" on entity \"%s\".\n", Input, Name.String().c_str() );
		Inputs[Input]();
	}
}

void CEntity::Track( const CEntity* Entity )
{
	if( Entity )
	{
		TrackedEntityIDs.push_back( Entity->GetEntityID().ID );
	}
}

void CEntity::Unlink( const EntityUID EntityID )
{
	// Find target input and unlink it.
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

CData& operator<<( CData& Data, CEntity* Entity )
{
	if( Entity )
	{
		DataString::Encode( Data, Entity->ClassName );
		DataString::Encode( Data, Entity->Name.String() );

		Data << Entity->Identifier.ID;

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
	}

	return Data;
}

CData& operator>>( CData& Data, CEntity* Entity )
{
	if( Entity )
	{
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
	}
	
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
