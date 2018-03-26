// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include "glm/glm.hpp"

class IGameLayer
{
public:
	virtual ~IGameLayer() {};

	virtual void Initialize() = 0;
	virtual void Frame() = 0;
	virtual void Tick() = 0;
	virtual void Shutdown() = 0;
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

	void RegisterGameLayers();

private:
	std::vector<IGameLayer*> GameLayers;

	double PreviousTime;
	double CurrentTime;
	double DeltaTime;

	double TimeScale;
};

extern CGameLayers* GameLayersInstance;
