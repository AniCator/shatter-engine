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
		Level->Remove( this );
	}
}

void CEntity::Send( const std::string& Entity, const std::string& Name )
{

}

void CEntity::Receive( const std::string& Name )
{

}

void CEntity::Track( CEntity* Entity )
{
	
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
