// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Game.h"

#include <Engine/Audio/SimpleSound.h>

#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/Display/Window.h>

#include <Engine/Profiling/Profiling.h>
#include <Engine/Profiling/Logging.h>

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
	Log::Event( "Initializing game layers.\n" );

	CSimpleSound::StopMusic();

	PreviousTime = 0.0f;
	CurrentTime = 0.0f;
	DeltaTime = CurrentTime - PreviousTime;

	for( auto GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Initialize();
		}
	}

	// Tick once.
	CSimpleSound::Tick();
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
	// Profile( "Game" );

	Vector4D Time;
	Time.X = static_cast<float>( GetCurrentTime() );
	Time.Y = static_cast<float>( GetDeltaTime() );
	Time.Z = static_cast<float>( GetPreviousTime() );
	Time.W = static_cast<float>( GetTimeScale() );

	CWindow& Window = CWindow::Get();
	Window.GetRenderer().SetUniformBuffer( "Time", Time );

	for( auto GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Tick();
		}
	}

	CSimpleSound::Tick();
}

void CGameLayers::Shutdown()
{
	CSimpleSound::StopAll();

	for( auto GameLayer : GameLayers )
	{
		if( GameLayer )
		{
			GameLayer->Shutdown();
		}
	}
}

void CGameLayers::Time( float& Time )
{
	PreviousTime = CurrentTime;
	CurrentTime = Time;
	DeltaTime = CurrentTime - PreviousTime;
}

float CGameLayers::GetPreviousTime() const
{
	return PreviousTime;
}

float CGameLayers::GetCurrentTime() const
{
	return CurrentTime;
}

float CGameLayers::GetDeltaTime() const
{
	return DeltaTime;
}

float CGameLayers::GetTimeScale() const
{
	return TimeScale;
}

void CGameLayers::SetTimeScale( float TimeScaleIn )
{
	TimeScale = TimeScaleIn;
}

std::vector<IGameLayer*> CGameLayers::GetGameLayers() const
{
	return GameLayers;
}
