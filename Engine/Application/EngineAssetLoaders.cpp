// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "EngineAssetLoaders.h"

#include <Engine/Display/Rendering/Particle/Particle.h>

void InitializeEngineAssetLoaders()
{
	ParticleSystem::RegisterAssetLoader();
}
