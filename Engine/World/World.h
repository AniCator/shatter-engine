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

	CEntity* Find( const std::string& Name ) const;
	CEntity* Find( const size_t& ID ) const;
	CEntity* Find( const EntityUID& ID ) const;
	CEntity* Find( const UniqueIdentifier& Identifier ) const;

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

	CCamera* GetPreviousCamera() const;

	CPhysics* GetPhysics() const;

	void MakePrimary();
	static CWorld* GetPrimaryWorld();

	// Associates an entity with a tag name.
	void Tag( CEntity* Entity, const std::string& TagName );

	// Unregisters an entity from a tag name.
	void Untag( CEntity* Entity, const std::string& TagName );

	// Returns a vector of entities matching the given tag name.
	// Returns null if the tag couldn't be found.
	const std::vector<CEntity*>* GetTagged( const std::string& TagName ) const;

	// Returns the entire tag container. (read-only)
	const std::unordered_map<std::string, std::vector<CEntity*>>& GetTags() const;

	// Find an entity in a specific tag category.
	CEntity* Find( const std::string& TagName, const std::string& EntityName ) const;

private:
	std::deque<CLevel> Levels;
	CLevel* ActiveLevel;

	Vector3D CameraPosition = Vector3D::Zero;
	CCamera* PreviousCamera = nullptr;
	CCamera* Camera = nullptr;
	uint32_t CameraPriority;

	CPhysics* Physics;

	// Used to store tagged/registered entities.
	std::unordered_map<std::string, std::vector<CEntity*>> Tags;

	static CWorld* PrimaryWorld;

public:
	friend CData& operator<<( CData& Data, CWorld* World );
	friend CData& operator>>( CData& Data, CWorld* World );
};
