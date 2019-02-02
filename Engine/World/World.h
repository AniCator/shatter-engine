// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include "glm/glm.hpp"

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
	T* Spawn( glm::vec3 Position, glm::vec3 Orientation )
	{
		T* Entity = new T();
		Entities.push_back( Entity );

		return Entity;
	}

private:
	std::vector<CEntity*> Entities;
};
