// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Rendering/StorageBuffer.h>
#include <Engine/Display/Rendering/Particle/Particle.h>

class ParticleEmitter
{
public:
	ParticleEmitter();

	void Initialize();
	void Destroy();
	
	void Tick();
	void Frame();
protected:
	class CShader* Compute = nullptr;
	class CShader* Render = nullptr;
	class CRenderable* Renderable = nullptr;

	size_t Count = 8192;

	friend class ParticleRenderable;
};
