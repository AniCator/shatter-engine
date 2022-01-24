// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

#include <Engine/Physics/Geometry.h>

class CBody;
class CPhysicsScene;
class Vector3D;
struct BoundingBox;

enum class PollType : int8_t
{
	// Polls both the static and dynamic scene.
	All,

	// Poll just the static bodies.
	Static,

	// Poll just the dynamic bodies.
	Dynamic
};

class CPhysics
{
public:
	CPhysics();
	~CPhysics();

	// Waits for the workers to finish their tasks.
	void Guard() const;

	void Tick( const double& Time );
	void Destroy() const;

	void Register( CBody* Body ) const;
	void Unregister( CBody* Body ) const;

	Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const PollType& Type = PollType::All ) const;
	Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore, const PollType& Type = PollType::All ) const;
	std::vector<CBody*> Query( const BoundingBox& AABB, const PollType& Type = PollType::All ) const;

	double CurrentTime = -1.0;
	double TimeStep = 1.0 / 120.0;
	double ActualDeltaTime = -1.0;
private:
	CPhysicsScene* Scene;
};
