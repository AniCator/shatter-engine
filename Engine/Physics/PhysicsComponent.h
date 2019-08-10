// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math.h>

class CMeshEntity;

class CPhysicsComponent
{
public:
	CPhysicsComponent( CMeshEntity* Owner );
	~CPhysicsComponent();

	void Construct( class CPhysics* Physics );
	void Collision( CPhysicsComponent* Component );
	void Tick();
	void Destroy( class CPhysics* Physics );

	void CalculateBounds();
	FBounds GetBounds() const;

	CMeshEntity* Owner;

	bool Static;
	bool Block;
	bool Contact;
	FTransform PreviousTransform;
	FBounds Bounds;
	Vector3D Position;
	Vector3D Acceleration;
	Vector3D Velocity;
	float Mass;
	size_t Contacts;
};
