// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Game/Game.h>

class CCauseEffect : public IGameLayer
{
public:
	CCauseEffect();
	virtual ~CCauseEffect();

	virtual void Initialize() override;
	virtual void Frame() override;
	virtual void Tick() override;
	virtual void Shutdown() override;

private:
};
