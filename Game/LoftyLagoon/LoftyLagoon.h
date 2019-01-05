// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Game/Game.h>

class CRenderable;

class CGameLoftyLagoon : public IGameLayer
{
public:
	CGameLoftyLagoon();
	virtual ~CGameLoftyLagoon();

	virtual void Initialize() override;
	virtual void Frame() override;
	virtual void Tick() override;
	virtual void Shutdown() override;

private:
	std::vector<CRenderable*> TestRenderables;
};
