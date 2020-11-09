// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/Body/Body.h>

class CPlaneBody : public CBody
{
public:
	CPlaneBody();
	virtual ~CPlaneBody();

	virtual void PreCollision() override;
	virtual void Collision( CBody* Body ) override;
	virtual void Tick() override;
	virtual void Debug() override;

	Vector3D PlaneOrigin = Vector3D::Zero;
	Vector3D PlaneNormal = Vector3D( 0.0f, 0.0f, 1.0f );
};