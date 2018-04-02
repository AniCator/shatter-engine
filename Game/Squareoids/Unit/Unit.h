// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "UnitInterface.h"

class CSquareoidsUnit : public ISquareoidsUnit
{
public:
	CSquareoidsUnit();
	~CSquareoidsUnit();

	virtual void Interaction( ISquareoidsUnit* Unit ) override;
	virtual void Tick() override;
	virtual FSquareoidUnitData& GetUnitData() override;

private:
	FSquareoidUnitData UnitData;
};
