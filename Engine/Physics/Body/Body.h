// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

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

	CMeshEntity* Owner = nullptr;
	CEntity* Ghost = nullptr;

	bool Static = false;
	bool Stationary = true;
	bool Block = true;
	bool Contact = false;
	FTransform PreviousTransform;
	FBounds LocalBounds;
	FBounds WorldBounds;
	Vector3D DeltaPosition = Vector3D::Zero;
	Vector3D Acceleration = Vector3D::Zero;
	Vector3D Velocity = Vector3D::Zero;
	Vector3D Depenetration = Vector3D::Zero;
	Vector3D Normal = Vector3D::Zero;
	float Mass = 1.0f;
	float InverseMass = 1.0f;
	size_t Contacts = 0;

	TriangleTree* Tree = nullptr;
};