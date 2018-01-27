// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Game.h"
#include <Profiling/Profiling.h>

CGameLayers* GameLayersInstance = new CGameLayers();

CGameLayers::CGameLayers()
{
	GameLayers.reserve( 128 );

	TimeScale = 1.0f;
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
	for( auto GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Initialize();
		}
	}
}

void CGameLayers::Frame()
{
	for( auto GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Frame();
		}
	}
}

void CGameLayers::Tick()
{
	Profile( "Game" );

	for( auto GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Tick();
		}
	}
}

void CGameLayers::Shutdown()
{
	for( auto GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Shutdown();
		}
	}
}

void CGameLayers::Time( double& Time )
{
	PreviousTime = CurrentTime;
	CurrentTime = Time;
	DeltaTime = CurrentTime - PreviousTime;
}

double CGameLayers::GetPreviousTime() const
{
	return PreviousTime;
}

double CGameLayers::GetCurrentTime() const
{
	return CurrentTime;
}

double CGameLayers::GetDeltaTime() const
{
	return DeltaTime;
}

double CGameLayers::GetTimeScale() const
{
	return TimeScale;
}

void CGameLayers::SetTimeScale( double TimeScaleIn )
{
	TimeScale = TimeScaleIn;
}
