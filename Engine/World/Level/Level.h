// Copyright � 2017, Christiaan Bakker, All rights reserved.
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
		CEntity* Entity = new T();
		if( Entity )
		{
			Entity->Name = Name;
			Entity->Identifier.Random();
			Entity->SetEntityID( EntityUID::Create() );
			Entity->SetLevelID( LevelUID( Entities.size() ) );
			Entity->SetLevel( this );
			Entities.push_back( Entity );
		}

		return dynamic_cast<T*>( Entity );
	}

	CEntity* Spawn( const std::string& Type, const std::string& Name, const UniqueIdentifier& Identifier = UniqueIdentifier() )
	{
		CEntity* Entity = CEntityMap::Get().Find( Type )( );
		if( Entity )
		{
			Entity->Name = Name;
			
			if( Identifier.Valid() )
			{
				Entity->Identifier = Identifier;
			}
			else
			{
				Entity->Identifier.Random();
			}
			
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
	CEntity* Find( const UniqueIdentifier& Identifier ) const;
	
	template<class T>
	std::vector<T*> Find() const
	{
		static std::vector<T*> FoundEntities;
		FoundEntities.clear();
		
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
	FTransform Transform;

	bool IsTemporary() const
	{
		return Temporary;
	}

	// Used to indicate if the level was loaded as a sub-level in a level script.
	bool Prefab = false;

private:
	std::vector<CEntity*> Entities;
	CWorld* World;
	std::string Name;

	bool Temporary = false;

public:
	friend CData& operator<<( CData& Data, CLevel& Level );
	friend CData& operator>>( CData& Data, CLevel& Level );
};
