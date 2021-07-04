// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Display/Rendering/Particle/ParticleEmitter.h>

class ParticleEntity : public CPointEntity
{
public:
	void Construct() override;
	void Tick() override;
	void Frame() override;
	void Destroy() override;

	void Reload() override;
	void Load( const JSON::Vector& Objects ) override;
	void Import( CData& Data ) override;
	void Export( CData& Data ) override;
	
protected:
	ParticleEmitter Emitter;

	class CShader* Compute = nullptr;
	class CShader* Render = nullptr;
	size_t Count = 1000;

	std::string ComputeName;
	std::string RenderName;
};
