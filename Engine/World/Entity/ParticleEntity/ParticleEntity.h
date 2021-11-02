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

	void Debug() override;

protected:
	ParticleEmitter Emitter;

	class ParticleAsset* Asset = nullptr;
	std::string ParticleAssetName;
	BoundingBox Bounds;
};
