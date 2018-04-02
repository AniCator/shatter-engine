// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Unit.h"

class CSquareoidsPlayerUnit : public CSquareoidsUnit
{
public:
	CSquareoidsPlayerUnit();
	~CSquareoidsPlayerUnit();

	virtual void Interaction( ISquareoidsUnit* Unit ) override;
	virtual void Tick() override;
	virtual FSquareoidUnitData& GetUnitData() override;

private:
	FSquareoidUnitData UnitData;
};
