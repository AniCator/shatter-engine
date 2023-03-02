// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Response.h"

#include <Engine/Utility/Math.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Display/Rendering/Mesh.h>

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

bool AxisTest( const Vector3D& Axis, const Vector3D& A, const Vector3D& B, const Vector3D& C, const float& Radius )
{
	const float PointA = A.Dot( Axis );
	const float PointB = B.Dot( Axis );
	const float PointC = C.Dot( Axis );
	return Math::Max( -Math::VectorMax( PointA, PointB, PointC ), Math::VectorMin( PointA, PointB, PointC ) ) > Radius;
}

CollisionResponse Response::TriangleAABB( const VertexFormat& A, const VertexFormat& B, const VertexFormat& C, const Vector3D& Center, const Vector3D& Extent )
{
	CollisionResponse Response;

	const Vector3D VertexA = A.Position - Center;
	const Vector3D VertexB = B.Position - Center;
	const Vector3D VertexC = C.Position - Center;

	// Direction vector pointing from B to A.
	const Vector3D EdgeA = VertexB - VertexA;

	// Direction vector pointing from C to B.
	const Vector3D EdgeB = VertexC - VertexB;

	// Direction vector pointing from A to C.
	const Vector3D EdgeC = VertexA - VertexC;

	Vector3D A00 = Vector3D( 0.0f, -EdgeA.Z, EdgeA.Y );
	const float PointA = VertexA.Z * VertexB.Y - VertexA.Y * VertexB.Z;
	const float PointC = VertexC.Z * ( VertexB.Y - VertexA.Z ) - VertexC.Z * ( VertexB.Z - VertexA.Z );
	const float RadiusA00 = Extent.Y * Math::Abs( EdgeA.Z ) + Extent.Z * Math::Abs( EdgeA.Y );
	if( Math::Max( -Math::Max( PointA, PointC ), Math::Min( PointA, PointC ) ) > RadiusA00 )
	{
		return Response;
	}

	Vector3D A01 = Vector3D( 0.0f, -EdgeB.Z, EdgeB.Y );
	const float RadiusA01 = Extent.Y * Math::Abs( EdgeB.Z ) + Extent.Z * Math::Abs( EdgeB.Y );
	if( AxisTest( A01, VertexA, VertexB, VertexC, RadiusA01 ) )
	{
		return Response;
	}

	Vector3D A02 = Vector3D( 0.0f, -EdgeC.Z, EdgeC.Y );
	const float RadiusA02 = Extent.Y * Math::Abs( EdgeC.Z ) + Extent.Z * Math::Abs( EdgeC.Y );
	if( AxisTest( A02, VertexA, VertexB, VertexC, RadiusA02 ) )
	{
		return Response;
	}

	Vector3D A10 = Vector3D( EdgeA.Z, 0.0f, -EdgeA.X );
	const float RadiusA10 = Extent.X * Math::Abs( EdgeA.Z ) + Extent.Z * Math::Abs( EdgeA.X );
	if( AxisTest( A10, VertexA, VertexB, VertexC, RadiusA10 ) )
	{
		return Response;
	}

	Vector3D A11 = Vector3D( EdgeB.Z, 0.0f, -EdgeB.X );
	const float RadiusA11 = Extent.X * Math::Abs( EdgeB.Z ) + Extent.Z * Math::Abs( EdgeB.X );
	if( AxisTest( A11, VertexA, VertexB, VertexC, RadiusA11 ) )
	{
		return Response;
	}

	Vector3D A12 = Vector3D( EdgeC.Z, 0.0f, -EdgeC.X );
	const float RadiusA12 = Extent.X * Math::Abs( EdgeC.Z ) + Extent.Z * Math::Abs( EdgeC.X );
	if( AxisTest( A12, VertexA, VertexB, VertexC, RadiusA12 ) )
	{
		return Response;
	}

	Vector3D A20 = Vector3D( -EdgeA.Y, EdgeA.X, 0.0f );
	const float RadiusA20 = Extent.X * Math::Abs( EdgeA.Y ) + Extent.Z * Math::Abs( EdgeA.X );
	if( AxisTest( A20, VertexA, VertexB, VertexC, RadiusA20 ) )
	{
		return Response;
	}

	Vector3D A21 = Vector3D( -EdgeB.Y, EdgeB.X, 0.0f );
	const float RadiusA21 = Extent.X * Math::Abs( EdgeB.Y ) + Extent.Z * Math::Abs( EdgeB.X );
	if( AxisTest( A21, VertexA, VertexB, VertexC, RadiusA21 ) )
	{
		return Response;
	}

	Vector3D A22 = Vector3D( -EdgeC.Y, EdgeC.X, 0.0f );
	const float RadiusA22 = Extent.X * Math::Abs( EdgeC.Y ) + Extent.Z * Math::Abs( EdgeC.X );
	if( AxisTest( A22, VertexA, VertexB, VertexC, RadiusA22 ) )
	{
		return Response;
	}

	if( Math::VectorMax( VertexA.X, VertexB.X, VertexC.X ) < -Extent.X || Math::VectorMin( VertexA.X, VertexB.X, VertexC.X ) > Extent.X )
	{
		return Response;
	}

	if( Math::VectorMax( VertexA.Y, VertexB.Y, VertexC.Y ) < -Extent.Y || Math::VectorMin( VertexA.Y, VertexB.Y, VertexC.Y ) > Extent.Y )
	{
		return Response;
	}

	if( Math::VectorMax( VertexA.Z, VertexB.Z, VertexC.Z ) < -Extent.Z || Math::VectorMin( VertexA.Z, VertexB.Z, VertexC.Z ) > Extent.Z )
	{
		return Response;
	}

	Response.Normal = EdgeA.Cross( EdgeB );
	Response.Distance = Response.Normal.Dot( VertexA );
	ByteToVector( A.Normal, Response.Normal );

	// const float Radius = Extent.X * Math::Abs( Response.Normal.X ) + Extent.Y * Math::Abs( Response.Normal.Y ) + Extent.Z * Math::Abs( Response.Normal.Z );
	// const float DistanceFromCenter = Response.Normal.Dot( Extent ) - Response.Distance;
	// Response.Distance = Radius * DistanceFromCenter;

	return Response;
}
