// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <memory>

#include "../Unit/UnitInterface.h"

class CCamera;

class CSquareoidsBattlefield
{
public:
	CSquareoidsBattlefield();
	~CSquareoidsBattlefield();

	void Update();

	void UpdateBruteForce();
	void UpdateSpatialGrid();
private:
	std::vector<ISquareoidsUnit*> SquareoidUnits;

	ISquareoidsUnit* PlayerUnit;

	CCamera* Camera;
};
