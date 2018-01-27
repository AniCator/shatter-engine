// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include <Game/Game.h>

// AICritters.h
#include <Game/AICritters/AICritters.h>
CAICritters* AICrittersInstance = new CAICritters();
// CTestLayer* TestLayerInstance = new CTestLayer();

// CauseEffect.h
#include <Game/CauseEffect/CauseEffect.h>
CCauseEffect* CauseEffectInstance = new CCauseEffect();

void CGameLayers::RegisterGameLayers()
{
	GameLayersInstance->Add( AICrittersInstance );

	GameLayersInstance->Add( CauseEffectInstance );
}
