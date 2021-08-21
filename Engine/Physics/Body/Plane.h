// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/Body/Body.h>

class CPlaneBody : public CBody
{
public:
	CPlaneBody();
	virtual ~CPlaneBody();

	virtual void PreCollision() override;
	virtual bool Collision( CBody* Body ) override;
	virtual void Tick() override;
	virtual void Debug() const override;
	virtual BodyType GetType() const override;

	// A point on the plane.
	Vector3D PlaneOrigin = Vector3D::Zero;

	// The up vector of the plane.
	Vector3D PlaneNormal = Vector3D( 0.0f, 0.0f, 1.0f );

	// When disabled, objects on the other side of the normal are not considered for collision when intersecting.
	bool TwoSidedCollision = true;

	// When a body is below the surface of the plane, it is projected back onto it.
	// This is useful for when you're colliding with a plane that you shouldn't get below. (possibly height fields in the future)
	bool ProjectToSurface = true;
};