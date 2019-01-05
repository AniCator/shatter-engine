// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Game/Game.h>

class CRenderable;

class CDummyLayer : public IGameLayer
{
public:
	CDummyLayer();
	virtual ~CDummyLayer();

	virtual void Initialize() override;
	virtual void Frame() override;
	virtual void Tick() override;
	virtual void Shutdown() override;

	virtual Version GetVersion() const override;
private:
	std::vector<CRenderable*> TestRenderables;
};
