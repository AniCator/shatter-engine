// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <deque>

#include <Engine/World/Level/Level.h>
#include <Engine/Utility/Math.h>

#include <Engine/Display/Rendering/Camera.h>

class CEntity;
class CPhysics;

//struct Event
//{
//	CTimer Timer;
//	float Interval;
//	std::function<void> Callback;
//};

class CWorld
{
public:
	CWorld();
	~CWorld();

	void Construct();
	void Frame();
	void Tick();
	void Destroy();

	void Reload();

	template<typename T>
	T* Spawn()
	{
		if( ActiveLevel )
			return ActiveLevel->Spawn<T>();

		return nullptr;
	}

	CLevel& Add();
	std::deque<CLevel>& GetLevels() { return Levels; };
	CLevel* GetActiveLevel() { return ActiveLevel; };

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

	CEntity* Find( const EntityUID& ID ) const
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

	void SetActiveCamera( CCamera* Camera, uint32_t Priority = 100 );
	CCamera* GetActiveCamera() const;
	const FCameraSetup& GetActiveCameraSetup() const;

	CPhysics* GetPhysics() const;

	void MakePrimary();
	static CWorld* GetPrimaryWorld();

private:
	std::deque<CLevel> Levels;
	CLevel* ActiveLevel;

	Vector3D CameraPosition = Vector3D::Zero;
	CCamera* PreviousCamera = nullptr;
	CCamera* Camera;
	uint32_t CameraPriority;

	CPhysics* Physics;

	static CWorld* PrimaryWorld;

public:
	friend CData& operator<<( CData& Data, CWorld* World );
	friend CData& operator>>( CData& Data, CWorld* World );
};
