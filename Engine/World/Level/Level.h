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

	void Load( const CFile& File );
	std::vector<CEntity*>& GetEntities() { return Entities; };

private:
	std::vector<CEntity*> Entities;

public:
	friend CData& operator<<( CData& Data, CLevel& Level );
	friend CData& operator>>( CData& Data, CLevel& Level );
};
