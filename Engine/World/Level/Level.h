// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

#include <Engine/World/Entity/Entity.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>

class CLevel
{
public:
	CLevel();
	~CLevel();

	void Construct();
	void Tick();
	void Destroy();

	template<typename T>
	T* Spawn()
	{
		Entities.push_back( new T() );

		return dynamic_cast<T*>(Entities.back());
	}

	CEntity* Spawn( const std::string& Type, const std::string& Name )
	{
		CEntity* Entity = CEntityMap::Get().Find( Type )( );
		if( Entity )
		{
			Entity->Name = Name;
			Entity->SetID( Entities.size() );
			Entity->SetLevel( this );
			Entities.push_back( Entity );
		}
		else
		{
			Log::Event( Log::Warning, "Unknown entity type requested \"%s\".\n", Type.c_str() );
		}

		return Entity;
	}

	CEntity* Spawn( const std::string& Type )
	{
		return Spawn( Type, std::to_string( Entities.size() ) );
	}

	void Load( const CFile& File );
	const std::vector<CEntity*>& GetEntities() const { return Entities; };
	void Remove( CEntity* Entity );

	CEntity* Find( const std::string& Name ) const;
	CEntity* Find( const size_t ID ) const;

private:
	std::vector<CEntity*> Entities;

public:
	friend CData& operator<<( CData& Data, CLevel& Level );
	friend CData& operator>>( CData& Data, CLevel& Level );
};
