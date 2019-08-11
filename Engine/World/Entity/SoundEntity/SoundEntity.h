// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Utility/Math.h>

class CSound;

namespace EFalloff
{
	enum Type
	{
		None = 0,
		Linear,
		InverseSquare
	};
}

class CSoundEntity : public CPointEntity
{
public:
	CSoundEntity();
	CSoundEntity( FTransform& Transform );
	virtual ~CSoundEntity() override;

	virtual void Spawn( FTransform& Transform );

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;

	virtual void Load( const JSON::Vector& Objects ) override;

	void Play();
	void Stop();

	void UpdateSound();

	virtual void Import( CData& Data ) override;
	virtual void Export( CData& Data ) override;

public:
	CSound* Sound;

	EFalloff::Type Falloff;
	float Radius;
	float Volume;
	float FadeIn;
	float FadeOut;
	bool AutoPlay;
	bool Loop;

	bool AutoPlayed;

	FName SoundName;
};
