// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/Entity.h>
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

class CSoundEntity : public CEntity
{
public:
	CSoundEntity();
	CSoundEntity( FTransform& Transform );
	virtual ~CSoundEntity() override;

	virtual void Spawn( FTransform& Transform );

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;

	virtual const FTransform& GetTransform() const { return Transform; };
	virtual void Load( const JSON::Vector& Objects ) override;

	void Play();
	void Stop();

public:
	FTransform Transform;
	CSound* Sound;

	EFalloff::Type Falloff;
	float Radius;
	bool AutoPlay;
	bool Loop;
};
