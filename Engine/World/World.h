// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

#include <Engine/World/Level/Level.h>
#include <Engine/Utility/Math.h>

#include <Engine/Display/Rendering/Camera.h>

class CEntity;

class CWorld
{
public:
	CWorld();
	~CWorld();

	void Construct();
	void Tick();
	void Destroy();

	template<typename T>
	T* Spawn()
	{
		if( ActiveLevel )
			return ActiveLevel->Spawn<T>();
		else
			DebugBreak();

		return nullptr;
	}

	CLevel& Add();
	std::vector<CLevel>& GetLevels() { return Levels; };

	CEntity* Find( const std::string& Name ) const
	{
		for( auto& Level : Levels )
		{
			CEntity* Entity = Level.Find( Name );
			if( Entity )
			{
				return Entity;
			}
		}

		return nullptr;
	}

	CEntity* Find( const size_t ID ) const
	{
		for( auto& Level : Levels )
		{
			CEntity* Entity = Level.Find( ID );
			if( Entity )
			{
				return Entity;
			}
		}

		return nullptr;
	}

	template<class T>
	std::vector<T*> Find() const
	{
		std::vector<T*> FoundEntities;
		for( auto& Level : Levels )
		{
			auto LevelEntities = Level.Find<T>();
			FoundEntities.insert( FoundEntities.end(), LevelEntities.begin(), LevelEntities.end() );
		}

		return FoundEntities;
	}

	void SetActiveCamera(CCamera* Camera);
	const FCameraSetup& GetActiveCameraSetup() const;

private:
	std::vector<CLevel> Levels;
	CLevel* ActiveLevel;

	CCamera* Camera;

public:
	friend CData& operator<<( CData& Data, CWorld& World );
	friend CData& operator>>( CData& Data, CWorld& World );
};
