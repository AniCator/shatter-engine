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
	void Tick();
	void Destroy( class CPhysics* Physics );

	void CalculateBounds();
	FBounds GetBounds() const;

	CMeshEntity* Owner;

	bool Static;
	Vector3D CorrectiveForce;
	FTransform PreviousTransform;
	Vector3D PreviousVelocity;
	FBounds Bounds;
};
