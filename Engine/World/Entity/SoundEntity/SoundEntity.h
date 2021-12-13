// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Audio/Sound.h>
#include <Engine/Audio/SoundInstance.h>
#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Utility/Math.h>

class SoundEntity : public CPointEntity
{
public:
	SoundEntity();
	SoundEntity( FTransform& Transform );
	virtual ~SoundEntity() override;

	virtual void Spawn( FTransform& Transform );

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;

	virtual void Load( const JSON::Vector& Objects ) override;
	virtual void Reload() override;

	void Play();
	void Stop();

	void UpdateSound();

	virtual void Import( CData& Data ) override;
	virtual void Export( CData& Data ) override;

public:
	CSound* Asset = nullptr;
	SoundInstance Sound;

	EFalloff::Type Falloff;
	float Radius;
	float Rate = 1.0f;
	float Volume;
	float FadeIn;
	float FadeOut;
	bool AutoPlay;
	bool Loop;
	bool Is3D = true;
	Bus::Type Bus = Bus::SFX;
	
	bool OutOfRange = false;
	float Range = -1.0f;

	bool AutoPlayed;

	std::string SoundName;
};
