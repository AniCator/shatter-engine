// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <memory>
#include <Game/Game.h>

namespace Log
{
	class CLog;
};

class CGameLoftyLagoon : public IGameLayer
{
public:
	CGameLoftyLagoon();
	virtual ~CGameLoftyLagoon();

	virtual void Initialize() override;
	virtual void Frame() override;
	virtual void Tick() override;
	virtual void Shutdown() override;

	virtual Version GetVersion() const override;

private:
	static std::unique_ptr<Log::CLog> Report;
};
