// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/Primitive.h>

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

	std::vector<FVertex> Vertices;
};

class CBody
{
public:
	CBody( CMeshEntity* Owner, const FBounds& LocalBounds, const bool Static, const bool Stationary );
	~CBody();

	void Construct( class CPhysics* Physics );
	void PreCollision();
	virtual void Collision( CBody* Body );
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
class CTriggerBody : public CBody
{
public:
	CTriggerBody( CMeshEntity* Owner, const FBounds& Bounds ) : CBody( Owner, Bounds, false, true )
	{
		Block = false;
	}

	~CTriggerBody()
	{

	};

	virtual void Collision( CBody* Body ) override
	{
		auto Collider = dynamic_cast<TriggerType*>( Body->Owner );
		if( Collider )
		{
			CBody::Collision( Body );
			if( Contacts > 0 )
			{
				Entity = Collider;
			}
		}
	}

	TriggerType* Entity;
};
