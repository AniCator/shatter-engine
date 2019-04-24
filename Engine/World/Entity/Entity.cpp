// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Entity.h"

#include <Engine/World/Level/Level.h>

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

CEntity::CEntity()
{
	Level = nullptr;
}

void CEntity::SetID( const size_t EntityID )
{
	ID = EntityID;
}

const size_t CEntity::GetID() const
{
	return ID;
}

void CEntity::SetLevel( CLevel* SpawnLevel )
{
	Level = SpawnLevel;
}

CLevel* CEntity::GetLevel() const
{
	return Level;
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
				Entity->Unlink( GetID() );
			}
		}

		Level->Remove( this );
	}
}

void CEntity::Load( const JSON::Vector & Objects )
{
	
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
						auto Entity = Level->Find( TargetName );
						if( Entity )
						{
							FMessage Message;
							Message.TargetID = Entity->GetID();
							Message.Inputs.push_back( InputName );

							auto& Messsages = Outputs[OutputName];
							Messsages.push_back( Message );
						}
					}
				}
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
				auto Entity = Level->Find( Message.TargetID );
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
		Log::Event( "Receiving input \"%s\" on entity %i.\n", Input, GetID() );
		Inputs[Input]();
	}
}

void CEntity::Track( const CEntity* Entity )
{
	if( Entity )
	{
		TrackedEntityIDs.push_back( Entity->GetID() );
	}
}

void CEntity::Unlink( const size_t EntityID )
{
	// Find target input and unlink it.
}

/*
const std::vector<std::string>& CEntity::GetInputs() const
{
	return InputNames;
}

const std::vector<std::string>& CEntity::GetOutputs() const
{
	return OutputNames;
}
*/
