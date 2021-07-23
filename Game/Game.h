// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <string>

struct Version
{
	int Major = 0;
	int Minor = 0;
	int Hot = 0;

	std::string String() const
	{
		return std::to_string( Major ) + "." + std::to_string( Minor ) + "." + std::to_string( Hot );
	}
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

	void Time( const double& Time );
	void FrameTime( const double& FrameTime );

	double GetPreviousTime() const;
	double GetCurrentTime() const;
	double GetRealTime() const;
	double GetDeltaTime() const;

	double GetTimeScale() const;
	void SetTimeScale( const double& TimeScale );

	float GetFrameTime() const;

	std::vector<IGameLayer*> GetGameLayers() const;

private:
	std::vector<IGameLayer*> GameLayers;

	double PreviousTime;
	double CurrentTime;
	double DeltaTime;

	double TimeScale;

	double PreviousFrameTime;
	double CurrentFrameTime;
	double DeltaFrameTime;
};

extern CGameLayers* GameLayersInstance;
