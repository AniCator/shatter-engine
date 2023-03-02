// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Response.h"

#include <Engine/Utility/Math.h>
#include <Engine/Display/UserInterface.h>

CollisionResponse Response::SphereSphere( const BoundingSphere& A, const BoundingSphere& B )
{
	const float RadiusSquaredTotal = A.GetRadiusSquared() + B.GetRadiusSquared();

	// Calculate the difference between the two origins.
	const Vector3D CenterA = A.Origin();
	const Vector3D CenterB = B.Origin();
	const Vector3D Difference = CenterB - CenterA;
	const float DistanceSquared = Difference.LengthSquared();

	// Check if the spheres overlap.
	if( DistanceSquared > RadiusSquaredTotal || DistanceSquared == 0.0f )
		return {}; // We're not overlapping.

	CollisionResponse Response;

	// Calculate the separation distance.
	Response.Distance = ( DistanceSquared - RadiusSquaredTotal );

	// Check if we're dividing by zero.
	if( Response.Distance == 0.0f )
	{
		Response.Distance = 1.0f;
		Response.Normal = WorldUp;
		return Response;
	}

	// Set the output distance and normal.
	Response.Distance = Math::Abs( Response.Distance / Response.Distance ) * 0.5f;
	Response.Normal = Difference.Normalized();

	// Calculate the intersection point. (this isn't used right now but it can be used for angular velocity)
	float DistanceToIntersection = A.GetRadius() - Response.Distance;
	Response.Point = CenterA + Response.Normal * DistanceToIntersection;

	// UI::AddSphere( B.Origin(), B.GetRadius(), Color::Cyan, 1.0f );
	// UI::AddSphere( A.Origin(), A.GetRadius(), Color::Green, 1.0f );

	return Response;
}

CollisionResponse Response::SphereAABB( const BoundingSphere& Sphere, const BoundingBox& Box )
{
	Vector3D PointOnBox = Math::ProjectOnAABB( Sphere.Origin(), Box );
	Vector3D Direction = Sphere.Origin() - PointOnBox;

	const float DistanceSquared = Direction.LengthSquared();
	if( DistanceSquared > Sphere.GetRadiusSquared() )
		return {}; // We're not colliding with the bounding box.

	CollisionResponse Response;
	const bool Inside = DistanceSquared == 0.0f;
	if( Inside )
	{
		// The sphere is inside of the box.
		Response.Normal = Box.Center() - PointOnBox;

		// Find the shortest distance.
		if( Response.Normal.X < Response.Normal.Y && Response.Normal.X < Response.Normal.Z )
		{
			Response.Normal.X = Math::Sign( Response.Normal.X );
			Response.Normal.Y = 0.0f;
			Response.Normal.Z = 0.0f;
		}
		else if( Response.Normal.Y < Response.Normal.Z )
		{
			Response.Normal.X = 0.0f;
			Response.Normal.Y = Math::Sign( Response.Normal.Y );
			Response.Normal.Z = 0.0f;
		}
		else
		{
			Response.Normal.X = 0.0f;
			Response.Normal.Y = 0.0f;
			Response.Normal.Z = Math::Sign( Response.Normal.Z );
		}
	}
	else
	{
		Response.Normal = PointOnBox - Sphere.Origin();
		Response.Normal.Normalize();
	}

	Vector3D PointOnSphere = Sphere.Origin() - Response.Normal * Sphere.GetRadius();
	Response.Distance = PointOnBox.Distance( PointOnSphere );
	Response.Point = PointOnBox + ( PointOnSphere - PointOnBox ) * 0.5f;

	// UI::AddCircle( PointOnBox, 5.0f, Inside ? Color::Red : Color::Yellow );
	// UI::AddLine( Sphere.Origin(), Sphere.Origin() + Response.Normal, Color::Blue, 0.1f );
	// UI::AddLine( PointOnBox, PointOnBox + Response.Normal, Inside ? Color::Blue : Color::Purple );
	// UI::AddSphere( Sphere.Origin(), Sphere.GetRadius(), Color::Cyan );
	// UI::AddAABB( Box.Minimum, Box.Maximum, Color::Cyan );

	return Response;
}

CollisionResponse Response::AABBAABB( const BoundingBox& A, const BoundingBox& B )
{
	CollisionResponse Response;

	const Vector3D& CenterA = A.Center();
	const Vector3D& CenterB = B.Center();

	const Vector3D SizeA = A.Size();
	const Vector3D SizeB = B.Size();

	const Vector3D HalfSizeA = SizeA * 0.5f;
	const Vector3D HalfSizeB = SizeB * 0.5f;

	const Vector3D CenterDistance = ( CenterB - CenterA );
	const auto Overlap = ( HalfSizeA + HalfSizeB ) - Math::Abs( CenterDistance );
	if( Overlap.X < Overlap.Y && Overlap.X < Overlap.Z )
	{
		Response.Distance = Overlap.X;
		Response.Normal.X = Math::Sign( CenterDistance.X );
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = 0.0f;
	}
	else if( Overlap.Y < Overlap.Z )
	{
		Response.Distance = Overlap.Y;
		Response.Normal.X = 0.0f;
		Response.Normal.Y = Math::Sign( CenterDistance.Y );
		Response.Normal.Z = 0.0f;
	}
	else
	{
		Response.Distance = Overlap.Z;
		Response.Normal.X = 0.0f;
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = Math::Sign( CenterDistance.Z );
	}

	Response.Normal.Normalize();
	return Response;
}
