// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class CBody;
class CPhysicsScene;
class Vector3D;

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

	CBody* Cast( const Vector3D& Start, const Vector3D& End );
private:
	CPhysicsScene* Scene;
};
