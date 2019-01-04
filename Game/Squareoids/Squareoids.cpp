// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Squareoids.h"

#include <Engine/Utility/Locator/InputLocator.h>

#include "TitleScreen/TitleScreen.h"
#include "Battlefield/Battlefield.h"

#include <Engine/Application/Application.h>

#ifdef ConsoleWindowDisabled
#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#endif

CGameSquareoids* SquaroidsInstance = new CGameSquareoids();

void main()
{
	if( !GameLayersInstance )
	{
		Log::Event( Log::Fatal, "Game layers instance does not exist!\n" );
	}

	GameLayersInstance->Add( SquaroidsInstance );

	CApplication Application;
	Application.Run();
}

CGameSquareoids::CGameSquareoids()
{
	GameState = ESquareoidGameState::Unknown;
}

CGameSquareoids::~CGameSquareoids()
{

}

void CGameSquareoids::Initialize()
{
	IInput& Input = CInputLocator::GetService();
	Input.ClearActionBindings();

	GameState = ESquareoidGameState::Unknown;
}

void CGameSquareoids::Frame()
{

}

CSquareoidsTitleScreen* TitleScreen = nullptr;
CSquareoidsBattlefield* BattleField = nullptr;

void CGameSquareoids::Tick()
{
	IInput& Input = CInputLocator::GetService();
	const bool EscapeKeyPressed = Input.IsKeyDown( 256 );

	if( GameState == ESquareoidGameState::Unknown )
	{
		// Default to the title screen
		GameState = ESquareoidGameState::TitleScreen;
	}

	if( GameState == ESquareoidGameState::TitleScreen )
	{
		if( !TitleScreen )
		{
			TitleScreen = new CSquareoidsTitleScreen();
		}

		TitleScreen->Display();

		const bool SpaceBarPressed = Input.IsKeyDown( 32 );

		if( SpaceBarPressed && !EscapeKeyPressed )
		{
			delete TitleScreen;
			TitleScreen = nullptr;

			GameState = ESquareoidGameState::Game;
		}
	}
	else if( GameState == ESquareoidGameState::Game )
	{
		if( !BattleField )
		{
			BattleField = new CSquareoidsBattlefield();
		}

		BattleField->Update();

		// 256 is the escape key, temporarily hard-coded here so we don't have to include glfw3.h
		if( EscapeKeyPressed )
		{
			delete BattleField;
			BattleField = nullptr;

			GameState = ESquareoidGameState::TitleScreen;
		}
	}
}

void CGameSquareoids::Shutdown()
{
	GameState = ESquareoidGameState::Unknown;

	if( TitleScreen )
	{
		delete TitleScreen;
		TitleScreen = nullptr;
	}

	if( BattleField )
	{
		delete BattleField;
		BattleField = nullptr;
	}
}
