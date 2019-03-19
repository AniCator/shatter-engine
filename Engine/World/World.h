// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

#include <Engine/World/Level/Level.h>
#include <Engine/Utility/Math.h>

class CEntity;

struct FLevel
{
	CLevel Level;
	FTransform Transform;

	friend CData& operator<<( CData& Data, FLevel& Level )
	{
		Data << Level.Level;
		Data << Level.Transform;

		return Data;
	};

	friend CData& operator>>( CData& Data, FLevel& Level )
	{
		Data >> Level.Level;
		Data >> Level.Transform;

		return Data;
	};
};

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

	void Add( FLevel& Level );
	std::vector<FLevel>& GetLevels() { return Levels; };

private:
	std::vector<FLevel> Levels;
	CLevel* ActiveLevel;

public:
	friend CData& operator<<( CData& Data, CWorld& World );
	friend CData& operator>>( CData& Data, CWorld& World );
};
