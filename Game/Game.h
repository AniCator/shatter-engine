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
	virtual ~IGameLayer() = default;

	/// <summary>
	/// Called at the start of the program.
	/// </summary>
	virtual void Initialize() = 0;

	virtual void Frame() = 0;

	/// <summary>
	/// Executed before any accumulated ticks are run. (optional)
	/// </summary>
	virtual void PreTick() = 0;

	/// <summary>
	/// The main tick function that is called whenever ticks accumulate over time.
	/// </summary>
	virtual void Tick() = 0;

	/// <summary>
	/// Executed after all accumulated ticks have run. (optional)
	/// </summary>
	virtual void PostTick() = 0;

	/// <summary>
	/// Called on program exit and application restart.
	/// </summary>
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
	void RealTime( const double& RealTime );

	double GetPreviousTime() const;
	double GetCurrentTime() const;
	double GetRealTime() const;
	double GetDeltaTime() const;

	double GetTimeScale() const;
	void SetTimeScale( const double& TimeScale );

	double GetFrameTime() const;

	std::vector<IGameLayer*> GetGameLayers() const;

private:
	std::vector<IGameLayer*> GameLayers;

	double PreviousTime = 0.0;
	double CurrentTime = 0.0;
	double DeltaTime = 0.0;

	double TimeScale = 1.0;

	double DeltaFrameTime = 0.0;

	double CurrentRealTime = 0.0;
};

extern CGameLayers* GameLayersInstance;
