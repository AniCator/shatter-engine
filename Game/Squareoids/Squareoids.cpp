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

}

void CGameSquareoids::Frame()
{

}

CSquareoidsTitleScreen* TitleScreen = nullptr;
CSquareoidsBattlefield* BattleField = nullptr;

void CGameSquareoids::Tick()
{
	CInput& Input = CInput::GetInstance();

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

		if( Input.IsAnyKeyDown() )
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
	}
}

void CGameSquareoids::Shutdown()
{

}
