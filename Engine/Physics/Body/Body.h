// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <set>

#include <Engine/Physics/Body/Shared.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

class CBody
{
public:
	CBody();
	virtual ~CBody();

	void Construct();
	void Construct( class CPhysics* Physics );
	virtual void PreCollision();

	// Simulate environmental factors that should be executed before doing any collision work.
	virtual void Simulate();

	virtual bool Collision( CBody* Body );
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

	virtual BodyType GetType() const;
	
	virtual bool ShouldIgnoreBody( CBody* Body ) const;

	// Adds a body to the ignore list which prevents this body from being checked during collisions.
	// Setting Clear to true will remove the body from the list. (if it exists)
	virtual void Ignore( CBody* Body, const bool Clear = false );

	// Adds an entity's body to the ignore list which prevents this body from being checked during collisions.
	// Setting Clear to true will remove the body from the list. (if it exists)
	virtual void Ignore( CMeshEntity* Entity, const bool Clear = false );

	CMeshEntity* Owner = nullptr;

	// Used to represent any owner that isn't a mesh entity.
	CEntity* Ghost = nullptr;

	// This body does not move.
	bool Static = false;

	// This body is able to move on its own but cannot be moved by other bodies.
	bool Stationary = true;

	// This body is allowed to block other bodies from moving through it.
	bool Block = true;

	// This body can be pulled down by gravity.
	bool AffectedByGravity = true;

	bool Contact = false;
	FTransform PreviousTransform;
	FBounds LocalBounds;
	FBounds WorldBounds;
	Vector3D DeltaPosition = Vector3D::Zero;
	Vector3D Acceleration = Vector3D::Zero;
	Vector3D Velocity = Vector3D::Zero;

	// Offset vector that determines where the body will be projected to when penetrating surfaces.
	Vector3D Depenetration = Vector3D::Zero;
	Vector3D Normal = Vector3D::Zero;
	float Mass = 1.0f;
	float InverseMass = -1.0f;
	size_t Contacts = 0;
	bool Handled = false;

	TriangleTree* Tree = nullptr;

	std::set<CMeshEntity*> IgnoredBodies;
};