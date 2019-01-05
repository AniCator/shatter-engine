// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include "glm/glm.hpp"

struct Version
{
	int Major;
	int Minor;
	int Hot;
};

class IGameLayer
{
public:
	virtual ~IGameLayer() {};

	virtual void Initialize() = 0;
	virtual void Frame() = 0;
	virtual void Tick() = 0;
	virtual void Shutdown() = 0;

	virtual Version GetVersion() const = 0;
};

class CGameLayers
{
public:
	CGameLayers();
	~CGameLayers();

	void Add( IGameLayer* GameLayer );

	void Initialize();
	void Frame();
	void Tick();
	void Shutdown();

	void Time( double& Time );

	double GetPreviousTime() const;
	double GetCurrentTime() const;
	double GetDeltaTime() const;

	double GetTimeScale() const;
	void SetTimeScale( double TimeScale );

	std::vector<IGameLayer*> GetGameLayers() const;

private:
	std::vector<IGameLayer*> GameLayers;

	double PreviousTime;
	double CurrentTime;
	double DeltaTime;

	double TimeScale;
};

extern CGameLayers* GameLayersInstance;
