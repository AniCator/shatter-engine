// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Game.h"

#include <Engine/Audio/SoLoudSound.h>

#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/Display/Window.h>

#include <Engine/Profiling/Profiling.h>
#include <Engine/Profiling/Logging.h>

#include <Engine/Utility/ThreadPool.h>

CGameLayers* GameLayersInstance = new CGameLayers();

CGameLayers::CGameLayers()
{
	GameLayers.reserve( 128 );
}

CGameLayers::~CGameLayers()
{

}

void CGameLayers::Add( IGameLayer* GameLayer )
{
	GameLayers.push_back( GameLayer );
}

void CGameLayers::Initialize()
{
	ProfileScope();
	Log::Event( "Initializing game layers.\n" );

	SoLoudSound::StopStreams();

	PreviousTime = 0.0;
	CurrentTime = 0.0;
	DeltaTime = CurrentTime - PreviousTime;

	for( auto* GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Initialize();
		}
	}

	// Tick once.
	SoLoudSound::Tick();
}

void CGameLayers::Frame()
{
	ProfileAlways( "Game Frame" );
	OptickCategory( "Game Frame", Optick::Category::GameLogic );

	Vector4D Time;
	Time.X = static_cast<float>( GetCurrentTime() );
	Time.Y = static_cast<float>( GetDeltaTime() );
	Time.Z = static_cast<float>( GetPreviousTime() );
	Time.W = static_cast<float>( GetRealTime() );

	CWindow& Window = CWindow::Get();
	Window.GetRenderer().SetUniform( "Time", Time );

	for( auto* GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Frame();
		}
	}
}

void CGameLayers::Tick()
{
	ProfileAlways( "Game Tick" );
	OptickCategory( "Game Tick", Optick::Category::GameLogic );

	for( auto* GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Tick();
		}
	}

	SoLoudSound::Tick();
}

void CGameLayers::Shutdown()
{
	ProfileScope();
	SoLoudSound::StopAll();

	for( auto* GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Shutdown();
		}
	}
}

void CGameLayers::Time( const double& Time )
{
	PreviousTime = CurrentTime;
	CurrentTime = Time;
	DeltaTime = CurrentTime - PreviousTime;
}

void CGameLayers::FrameTime( const double& FrameTime )
{
	DeltaFrameTime = FrameTime;
}

void CGameLayers::RealTime( const double& RealTime )
{
	CurrentRealTime = RealTime;
}

double CGameLayers::GetPreviousTime() const
{
	return PreviousTime;
}

double CGameLayers::GetCurrentTime() const
{
	return CurrentTime;
}

double CGameLayers::GetRealTime() const
{
	return CurrentRealTime;
}

double CGameLayers::GetDeltaTime() const
{
	return DeltaTime;
}

double CGameLayers::GetTimeScale() const
{
	return TimeScale;
}

void CGameLayers::SetTimeScale( const double& TimeScaleIn )
{
	TimeScale = TimeScaleIn;
}

double CGameLayers::GetFrameTime() const
{
	return DeltaFrameTime;
}

std::vector<IGameLayer*> CGameLayers::GetGameLayers() const
{
	return GameLayers;
}
