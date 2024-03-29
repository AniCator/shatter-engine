// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/Entity.h>
#include <Engine/Utility/Math.h>

class CPointEntity : public CEntity
{
public:
	CPointEntity();
	CPointEntity( const FTransform& Transform );
	virtual ~CPointEntity() override;

	virtual void Tick() override;

	virtual const FTransform& GetTransform();
	virtual const FTransform& GetLocalTransform() const;
	virtual void SetTransform(const FTransform& Transform );
	virtual void Load( const JSON::Vector& Objects ) override;

	virtual void Debug() override;

	Vector3D GetVelocity() const;

	virtual void Import( CData& Data ) override;
	virtual void Export( CData& Data ) override;

protected:
	FTransform Transform;
	FTransform WorldTransform;
	bool ShouldUpdateTransform;

	FTransform PreviousWorldTransform;
	Vector3D Velocity = Vector3D::Zero;
};
