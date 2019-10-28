// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <Engine/Utility/Math.h>

class CMeshEntity;

struct TriangleTree
{
	TriangleTree()
	{
		Upper = nullptr;
		Lower = nullptr;
	}

	~TriangleTree()
	{
		delete Upper;
		delete Lower;
	}

	FBounds Bounds;

	TriangleTree* Upper;
	TriangleTree* Lower;

	std::vector<Vector3D> Vertices;
};

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

	void Debug();

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
	Vector3D Depenetration;
	Vector3D Normal;
	float Mass;
	float InverseMass;
	size_t Contacts;

	TriangleTree* Tree;
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

	virtual void Collision( CPhysicsComponent* Component ) override
	{
		auto Collider = dynamic_cast<TriggerType*>( Component->Owner );
		if( Collider )
		{
			CPhysicsComponent::Collision( Component );
			if( Contacts > 0 )
			{
				Entity = Collider;
			}
		}
	}

	TriggerType* Entity;
};
