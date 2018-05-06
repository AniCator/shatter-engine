// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Unit.h"

namespace ESquareoidsInputFlag
{
	typedef uint32_t ESquareoidsInputFlagType;
	enum Type
	{
		None		= 1 << 0,
		Forward		= 1 << 1,
		Backward	= 1 << 2,
		Left		= 1 << 3,
		Right		= 1 << 4,
	};
}

class CSquareoidsPlayerUnit : public CSquareoidsUnit
{
public:
	CSquareoidsPlayerUnit();
	~CSquareoidsPlayerUnit();

	virtual void Interaction( ISquareoidsUnit* Unit ) override;
	virtual void Tick() override;
	virtual FSquareoidUnitData& GetUnitData() override;

	void InputForwardPress();
	void InputForwardRelease();
	void InputBackwardPress();
	void InputBackwardRelease();
	void InputLeftPress();
	void InputLeftRelease();
	void InputRightPress();
	void InputRightRelease();

private:
	FSquareoidUnitData UnitData;
	bool Absorbing;

	ESquareoidsInputFlag::ESquareoidsInputFlagType Inputs;
};
