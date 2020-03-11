// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/Primitive.h>

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

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
	CBody( CMeshEntity* Owner, const bool Static, const bool Stationary );
	CBody( CMeshEntity* Owner, const FBounds& LocalBounds, const bool Static, const bool Stationary );
	CBody( CEntity* Parent, const FBounds& LocalBounds, const bool Static, const bool Stationary );
	~CBody();

	void Construct();
	void Construct( class CPhysics* Physics );
	virtual void PreCollision();
	virtual void Collision( CBody* Body );
	virtual void Tick();
	void Destroy( class CPhysics* Physics );

	virtual void CalculateBounds();
	FBounds GetBounds() const;
	virtual void SetBounds( const FBounds& Bounds )
	{
		LocalBounds = Bounds;
		CalculateBounds();
	}

	virtual const FTransform& GetTransform() const;

	virtual void Debug();

	CMeshEntity* Owner;
	CEntity* Ghost;

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
	CTriggerBody( CMeshEntity* Owner ) : CBody( Owner, false, true )
	{
		Block = false;
		Static = false;
		Stationary = true;

		CalculateBounds();
	}

	CTriggerBody( CEntity* Parent, const FBounds& Bounds ) : CBody( Parent, Bounds, false, true )
	{
		Block = false;
		Static = false;
		Stationary = true;

		CalculateBounds();
	}

	~CTriggerBody()
	{

	};

	virtual void PreCollision() override
	{
		Block = false;
		Static = false;
		Stationary = true;

		Entity = nullptr;
		CBody::PreCollision();
	}

	virtual void Collision( CBody* Body ) override
	{
		CEntity* Target = Body->Owner ? Body->Owner : Body->Ghost;
		TriggerType Collider = dynamic_cast<TriggerType>( Target );
		if( Collider )
		{
			Entity = Collider;
			Contacts++;
		}
	}

	virtual void Tick() override
	{

	}

	TriggerType Entity;

	virtual const FTransform& GetTransform() const override
	{
		return TriggerTransform;
	}

	virtual void SetTransform( const FTransform& Transform )
	{
		TriggerTransform = Transform;
		CalculateBounds();
	}

	FTransform TriggerTransform;
};

class Interactable
{
public:
	virtual void Interact( Interactable* Caller ) = 0;
};
