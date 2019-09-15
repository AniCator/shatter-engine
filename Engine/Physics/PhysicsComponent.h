// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math.h>

class CMeshEntity;

class CPhysicsComponent
{
public:
	CPhysicsComponent( CMeshEntity* Owner, const FBounds& LocalBounds, const bool Static, const bool Stationary );
	~CPhysicsComponent();

	void Construct( class CPhysics* Physics );
	void PreCollision();
	virtual void Collision( CPhysicsComponent* Component );
	void Tick();
	void Destroy( class CPhysics* Physics );

	virtual void CalculateBounds();
	FBounds GetBounds() const;

	CMeshEntity* Owner;

	bool Static;
	bool Stationary;
	bool Block;
	bool Contact;
	FTransform PreviousTransform;
	FBounds LocalBounds;
	FBounds WorldBounds;
	Vector3D DeltaPosition;
	Vector3D Acceleration;
	Vector3D Velocity;
	Vector3D Normal;
	float Mass;
	size_t Contacts;
};

template<typename TriggerType>
class CTriggerComponent : public CPhysicsComponent
{
public:
	CTriggerComponent( CMeshEntity* Owner, const FBounds& Bounds ) : CPhysicsComponent( Owner, Bounds, false, true )
	{
		Block = false;
	}

	~CTriggerComponent()
	{

	};

	virtual void Collision( CPhysicsComponent* Component )
	{
		auto Collider = dynamic_cast<TriggerType*>( Component->Owner );
		if( Collider )
		{
			CPhysicsComponent::Collision( Component );
		}
	}
};
