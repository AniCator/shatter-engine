// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include <Game/Game.h>

// AICritters.h
#include <Game/AICritters/AICritters.h>
// CAICritters* AICrittersInstance = new CAICritters();
CTestLayer* TestLayerInstance = new CTestLayer();

// CauseEffect.h
// #include <Game/CauseEffect/CauseEffect.h>
// CCauseEffect* CauseEffectInstance = new CCauseEffect();

// Squaroids.h
// #include <Game/Squareoids/Squareoids.h>
// CGameSquareoids* SquareoidsInstance = new CGameSquareoids();

#include <Engine/Utility/Test/PerformanceStringTest.h>

extern CStringPerformanceTest StringPerformanceTest;

void CGameLayers::RegisterGameLayers()
{
	// StringPerformanceTest.Run();
	// Add( SquareoidsInstance );
	// Add( TestLayerInstance );
}
