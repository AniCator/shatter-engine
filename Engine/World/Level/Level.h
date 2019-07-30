// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

#include <Engine/World/Entity/Entity.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/DataString.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>

class CWorld;

class CLevel
{
public:
	CLevel();
	CLevel(CWorld* NewWorld);
	~CLevel();

	void Construct();
	void Frame();
	void Tick();
	void Destroy();

	void Reload();

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
			Entity->ClassName = Type;
			Entity->SetEntityID( EntityUID::Create() );
			Entity->SetLevelID( LevelUID( Entities.size() ) );
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

	void Load( const CFile& File, const bool AssetsOnly = false );
	const std::vector<CEntity*>& GetEntities() const
	{ 
		return Entities;
	};

	void Remove( CEntity* Entity );

	CEntity* Find( const std::string& Name ) const;
	CEntity* Find( const size_t ID ) const;
	CEntity* Find( const EntityUID& ID ) const;
	
	template<class T>
	std::vector<T*> Find() const
	{
		std::vector<T*> FoundEntities;
		for( auto Entity : Entities )
		{
			if( T* CastEntity = dynamic_cast<T*>( Entity ) )
			{
				FoundEntities.emplace_back( CastEntity );
			}
		}

		return FoundEntities;
	}

	CWorld* GetWorld();
	void SetWorld( CWorld* NewWorld );

	const std::string& GetName() const;
	void SetName( const std::string& Name );

	FTransform GetTransform() const
	{
		return Transform;
	}

protected:
	FTransform Transform;

private:
	std::vector<CEntity*> Entities;
	CWorld* World;
	std::string Name;

public:
	friend CData& operator<<( CData& Data, CLevel& Level );
	friend CData& operator>>( CData& Data, CLevel& Level );
};
