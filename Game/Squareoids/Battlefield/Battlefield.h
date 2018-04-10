// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <memory>

#include "../Unit/UnitInterface.h"
#include <Engine/Utility/Structures/SpatialGrid.h>

class CCamera;

class CSquareoidsBattlefield
{
public:
	CSquareoidsBattlefield();
	~CSquareoidsBattlefield();

	void Update();
private:
	std::vector<ISquareoidsUnit*> SquareoidUnits;
	CSpatialGrid<ISquareoidsUnit> Units;

	ISquareoidsUnit* PlayerUnit;

	CCamera* Camera;
};
