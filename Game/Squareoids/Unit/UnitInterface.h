// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glm/glm.hpp"
#include <Engine/Utility/Math.h>

struct FSquareoidUnitData
{
	FTransform Transform;
	glm::vec3 Velocity;
	glm::vec4 Color;

	float Health;
};

class ISquareoidsUnit
{
public:
	virtual ~ISquareoidsUnit() = default;

	virtual void Interaction( ISquareoidsUnit* Unit ) = 0;
	virtual void Tick() = 0;
	virtual FSquareoidUnitData& GetUnitData() = 0;
};
