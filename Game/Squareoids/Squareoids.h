// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Game/Game.h>

#include "GameState/SquareoidsGameState.h"

class CGameSquareoids : public IGameLayer
{
public:
	CGameSquareoids();
	virtual ~CGameSquareoids();

	virtual void Initialize() override;
	virtual void Frame() override;
	virtual void Tick() override;
	virtual void Shutdown() override;

private:
	ESquareoidGameState GameState;
};
