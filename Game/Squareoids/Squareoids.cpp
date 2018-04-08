// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Squareoids.h"

#include <Engine/Input/Input.h>

#include "TitleScreen/TitleScreen.h"
#include "Battlefield/Battlefield.h"

CGameSquareoids::CGameSquareoids()
{
	GameState = ESquareoidGameState::Unknown;
}

CGameSquareoids::~CGameSquareoids()
{

}

void CGameSquareoids::Initialize()
{
	CInput& Input = CInput::GetInstance();
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
	CInput& Input = CInput::GetInstance();
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

		if( Input.IsAnyKeyDown() && !EscapeKeyPressed )
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

		// 256 is the escape key, temporarily hardcoded here so we don't have to include glfw3.h
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
