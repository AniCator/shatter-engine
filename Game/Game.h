// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

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

	void Time( float& Time );

	float GetPreviousTime() const;
	float GetCurrentTime() const;
	float GetDeltaTime() const;

	float GetTimeScale() const;
	void SetTimeScale( float TimeScale );

	std::vector<IGameLayer*> GetGameLayers() const;

private:
	std::vector<IGameLayer*> GameLayers;

	float PreviousTime;
	float CurrentTime;
	float DeltaTime;

	float TimeScale;
};

extern CGameLayers* GameLayersInstance;
