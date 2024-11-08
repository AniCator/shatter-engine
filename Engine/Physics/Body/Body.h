// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <set>

#include <Engine/Physics/Body/Shared.h>
#include <Engine/Physics/GeometryResult.h>
#include <Engine/Physics/PhysicalSurface.h>
#include <Engine/Physics/CollisionResponse.h>
#include <Engine/Utility/Structures/Testable.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

struct ContactManifold
{
	CollisionResponse Response;
	class CBody* Other = nullptr;
};

class CBody : public Testable
{
public:
	CBody() = default;
	~CBody() override;

	void Construct();
	void Construct( class CPhysics* Physics );
	virtual void PreCollision();

	// Simulate environmental factors that should be executed before doing any collision work.
	virtual void Simulate();

	// The collision function is used to generate collision responses and update the velocity vectors of bodies.
	// These are applied in the Tick function.
	virtual void Collision( CBody* Body );

	// The tick currently applies the collision results produced by the Collision function to the owning object and resets the body's internal values.
	virtual void Tick();
	void Destroy();

	virtual void CalculateBounds();
	BoundingBoxSIMD GetBounds() const;
	BoundingBox GetWorldBounds() const;
	virtual void SetBounds( const BoundingBox& Bounds )
	{
		LocalBounds = Bounds;
		CalculateBounds();
	}

	virtual const FTransform& GetTransform() const;
	virtual void SetTransform( const FTransform& Transform ) const;

	virtual void Debug() const override;
	
	virtual bool ShouldIgnoreBody( CBody* Body ) const;

	// Adds a body to the ignore list which prevents this body from being checked during collisions.
	// Setting Clear to true will remove the body from the list. (if it exists)
	virtual void Ignore( CBody* Body, const bool Clear = false );

	// Adds an entity's body to the ignore list which prevents this body from being checked during collisions.
	// Setting Clear to true will remove the body from the list. (if it exists)
	virtual void Ignore( CMeshEntity* Entity, const bool Clear = false );

	void Query( const BoundingBoxSIMD& Box, QueryResult& Result ) override;
	Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore = std::vector<Testable*>() ) const override;

	void SetMass( const float Mass );
	
	CMeshEntity* Owner = nullptr;

	// Used to represent any owner that isn't a mesh entity.
	CEntity* Ghost = nullptr;

	// This body does not move.
	bool Static = false;

	// This body is able to move on its own but cannot be moved by other bodies.
	bool Stationary = true;

	bool IsKinetic() const
	{
		return !Static && !Stationary;
	}

	// This body is allowed to block other bodies from moving through it.
	bool Block = true;

	// This body can be pulled down by gravity.
	bool AffectedByGravity = true;

	// Uses continuous collision detection.
	bool Continuous = false;

	// Determines what collision tests should be used for this object.
	BodyType Type = BodyType::AABB;

	// Determines what integrator to use.
	Integrator Integrator = Integrator::Euler;

	// This body hasn't moved for a while.
	bool Sleeping = false;
	double LastActivity = -1.0;

	bool Contact = true;
	FTransform PreviousTransform;
	BoundingBox LocalBounds;
	BoundingBox WorldBounds;
	BoundingBox WorldBoundsSwept;
	BoundingBoxSIMD WorldBoundsSweptSIMD;
	BoundingSphere WorldSphere;
	BoundingBoxSIMD WorldBoundsSIMD;
	BoundingBoxSIMD InflatedBoundsSIMD;

	// Generates the inner spherical bounds, based on the world bounds.
	BoundingSphere InnerSphere() const;

	// Velocity that is applied to the body multiplied by the time delta.
	Vector3D LinearVelocity = Vector3D::Zero;

	// Acceleration over time.
	Vector3D Acceleration = Vector3D::Zero;

	// Velocity.
	Vector3D ActualVelocity = Vector3D::Zero;
	Vector3D Velocity = Vector3D::Zero;
	float Damping = 1.0f;
	float Restitution = 1.0f;
	float Friction = 0.05f;

	// 0.47f - Sphere
	// 0.5f  - Cone
	// 1.0f  - Humanoid
	// 1.05f - Cube
	float DragCoefficient = 1.0f;

	Vector3D Gravity = Vector3D( 0.0f, 0.0f, -9.81f );

	// Offset vector that determines where the body will be projected to when penetrating surfaces.
	Vector3D Depenetration = Vector3D::Zero;
	Vector3D Normal = Vector3D::Zero;
	float Mass = 1.0f;
	float InverseMass = -1.0f;

	std::vector<ContactManifold> Contacts;
	CMeshEntity* ContactEntity = nullptr;

	// The surface material of this body.
	PhysicalSurface Surface = PhysicalSurface::None;

	struct TriangleTree* Tree = nullptr;

	std::vector<CMeshEntity*> IgnoredBodies;
	class CPhysics* Physics = nullptr;

	struct Constraint
	{
		
	};
	std::vector<Constraint> Constraints;
};