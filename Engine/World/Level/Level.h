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
		static size_t EntityID = 0;
		return Spawn( Type, std::to_string( EntityID++ ) );
	}

	void Load( const CFile& File );
	const std::vector<CEntity*>& GetEntities() const { return Entities; };

	CEntity* Find( std::string Name ) const;

private:
	std::vector<CEntity*> Entities;

public:
	friend CData& operator<<( CData& Data, CLevel& Level );
	friend CData& operator>>( CData& Data, CLevel& Level );
};
