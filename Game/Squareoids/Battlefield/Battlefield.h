// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

#include "../Unit/UnitInterface.h"

class CSquareoidsBattlefield
{
public:
	CSquareoidsBattlefield();
	~CSquareoidsBattlefield();

	void Update();
private:
	std::vector<ISquareoidsUnit*> SquareoidUnits;
};
