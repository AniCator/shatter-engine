// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <Engine/Utility/Math.h>

class CEntity;

class CLevel
{
public:
	CLevel();
	~CLevel();

	void Construct();
	void Tick();
	void Destroy();

	template<typename T>
	T* Spawn( FTransform& Transform )
	{
		T* Entity = new T();
		Entities.push_back( Entity );

		return Entity;
	}

private:
	std::vector<CEntity*> Entities;
};
