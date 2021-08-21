// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

#include <Engine/Utility/Geometry.h>

class CBody;
class CPhysicsScene;
class Vector3D;
struct BoundingBox;

class CPhysics
{
public:
	CPhysics();
	~CPhysics();

	void Tick() const;
	void Destroy() const;

	void Register( CBody* Body ) const;
	void Unregister( CBody* Body ) const;

	Geometry::Result Cast( const Vector3D& Start, const Vector3D& End ) const;
	Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore ) const;
	std::vector<CBody*> Query( const BoundingBox& AABB ) const;
private:
	CPhysicsScene* Scene;
};
