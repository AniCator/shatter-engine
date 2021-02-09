// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

class CBody;
class CPhysicsScene;
class Vector3D;
struct FBounds;

class CPhysics
{
public:
	CPhysics();
	~CPhysics();

	void Construct();
	void Tick();
	void Destroy();

	void Register( CBody* Body );
	void Unregister( CBody* Body );

	CBody* Cast( const Vector3D& Start, const Vector3D& End ) const;
	std::vector<CBody*> Query( const FBounds& AABB ) const;
private:
	CPhysicsScene* Scene;
};
