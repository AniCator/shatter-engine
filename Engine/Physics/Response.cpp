// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Response.h"

#include <Engine/Utility/Math.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Display/Rendering/Mesh.h>

#include <Engine/Utility/Math/BoundingBox.h>
#include <Engine/Utility/Math/Plane.h>

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
	Response.Distance = ( DistanceSquared - RadiusSquaredTotal ) * -2.0f;

	// Check if we're dividing by zero.
	if( Response.Distance != 0.0f )
	{
		// Response.Distance = sqrtf( Math::Abs( Response.Distance ) );
	}

	// Assign the response normal.
	Response.Normal = Difference.Normalized();

	// Calculate the intersection point. (this isn't used right now but it can be used for angular velocity)
	float DistanceToIntersection = A.GetRadius() * 0.5f - Response.Distance;
	Response.Point = CenterA + Response.Normal * DistanceToIntersection;

	// UI::AddSphere( B.Origin(), B.GetRadius(), Color::Cyan, 1.0f );
	// UI::AddSphere( A.Origin(), A.GetRadius(), Color::Green, 1.0f );
	// UI::AddCircle( Response.Point, 5.0f );
	// UI::AddLine( Response.Point, Response.Point + Response.Normal, Response.Distance > 0.0f ? Color::Blue : Color::Purple );
	// UI::AddLine( Response.Point, Response.Point + Response.Normal * Response.Distance, Color::Yellow );

	return Response;
}

CollisionResponse Response::SpherePlane( const BoundingSphere& Sphere, const Plane& Plane )
{
	const Vector3D Origin = Sphere.Origin();

	if( Origin.Dot( Plane.Normal ) - Plane.Distance != 0.0f )
		return {}; // We're not on the plane.

	const Vector3D PointOnPlane = Math::ProjectOnPlane( Origin, Plane );
	const Vector3D Direction = Origin - PointOnPlane;

	UI::AddLine( Origin, PointOnPlane, Color::Green );
	// UI::AddCircle( Origin, 5.0f, Color::Green );
	// UI::AddCircle( PointOnPlane, 5.0f, Color::Red );

	const float DistanceSquared = Direction.LengthSquared();
	if( DistanceSquared > Sphere.GetRadiusSquared() )
		return {}; // We're not colliding with the plane.

	CollisionResponse Response;
	Response.Normal = PointOnPlane - Origin;
	Response.Distance = Response.Normal.Normalize();

	const Vector3D PointOnSphere = Response.Normal * Sphere.GetRadius() + Origin;
	Response.Distance = PointOnPlane.Distance( PointOnSphere );
	Response.Point = PointOnPlane + ( PointOnSphere - PointOnPlane ) * 0.5f;

	UI::AddSphere( Origin, Sphere.GetRadius(), Color::Cyan );
	UI::AddCircle( PointOnPlane, 5.0f, Color::Green );
	UI::AddCircle( PointOnSphere, 5.0f, Color::Blue );

	return Response;
}

CollisionResponse Response::SphereAABB( const BoundingSphere& Sphere, const BoundingBox& Box )
{
	const Vector3D Origin = Sphere.Origin();
	const Vector3D PointOnBox = Math::ProjectOnAABB( Origin, Box );
	const Vector3D Direction = Origin - PointOnBox;

	const float DistanceSquared = Direction.LengthSquared();
	if( DistanceSquared > Sphere.GetRadiusSquared() )
		return {}; // We're not colliding with the bounding box.

	CollisionResponse Response;
	const bool Inside = DistanceSquared == 0.0f;
	if( Inside )
	{
		// The sphere is inside of the box.
		Response.Normal = Box.Center() - PointOnBox;
	}
	else
	{
		// The sphere is touching the box from outside.
		Response.Normal = PointOnBox - Origin;
	}

	// Find the dominant axis.
	const Vector3D AbsoluteNormal = Math::Abs( Response.Normal );
	if( AbsoluteNormal.X > AbsoluteNormal.Y && AbsoluteNormal.X > AbsoluteNormal.Z )
	{
		Response.Normal.X = Math::Sign( Response.Normal.X );
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = 0.0f;
	}
	else if( AbsoluteNormal.Y > AbsoluteNormal.Z )
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

	const Vector3D PointOnSphere = Origin + Response.Normal * Sphere.GetRadius();
	Response.Distance = PointOnBox.Distance( PointOnSphere );
	Response.Point = PointOnBox + ( PointOnSphere - PointOnBox ) * 0.5f;

	// UI::AddCircle( PointOnBox, 5.0f, Color::Red );
	// UI::AddCircle( PointOnSphere, 5.0f, Color::Green );
	// UI::AddLine( Origin, Origin + Response.Normal, Color::Blue, 0.1f );
	// UI::AddLine( PointOnBox, PointOnBox + Response.Normal, Response.Distance > 0.0f ? Color::Blue : Color::Purple );
	// UI::AddSphere( Origin, Sphere.GetRadius(), Color::Cyan );
	// UI::AddAABB( Box.Minimum, Box.Maximum, Color::Cyan );

	return Response;
}

CollisionResponse Response::AABBPlane( const BoundingBox& Box, const Plane& Plane )
{
	const Vector3D Center = Box.Center();
	if( Math::Equal( Center.Dot( Plane.Normal ) - Plane.Distance, 0.0f ) )
		return {}; // We're not on the plane.

	const Vector3D Size = Box.Size() * 0.5f;
	Vector3D AbsoluteNormal = Math::Abs( Plane.Normal );
	const float ProjectionDistance = Size.Dot( AbsoluteNormal );
	const float CosineAngle = Plane.Normal.Dot( Center );
	const float DistanceToCenter = CosineAngle - Plane.Distance;

	if( DistanceToCenter > ProjectionDistance )
		return {}; // The bounding box is not intersecting with the plane.

	CollisionResponse Response;

	Response.Normal = Plane.Normal * -1.0f;
	Response.Distance = fabs( ProjectionDistance - DistanceToCenter );

	return Response;
}

CollisionResponse Response::AABBAABB( const BoundingBox& A, const BoundingBox& B )
{
	const Vector3D PointOnBox = Math::ProjectOnAABB( A.Center(), B );
	const Vector3D Direction = PointOnBox - A.Center();

	const Vector3D Size = ( A.Size() + B.Size() );

	const float DistanceSquared = Direction.LengthSquared();
	//if( DistanceSquared > Size.LengthSquared() )
	//	return {}; // We're not colliding.

	CollisionResponse Response;

	const bool Inside = DistanceSquared == 0.0f;
	if( Inside )
	{
		// The sphere is inside of the box.
		Response.Normal = B.Center() - PointOnBox;
	}
	else
	{
		// The sphere is touching the box from outside.
		Response.Normal = Direction;
	}

	// Find the dominant axis.
	const Vector3D AbsoluteNormal = Math::Abs( Response.Normal );
	if( AbsoluteNormal.X > AbsoluteNormal.Y && AbsoluteNormal.X > AbsoluteNormal.Z )
	{
		Response.Normal.X = Math::Sign( Response.Normal.X );
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = 0.0f;

		Response.Distance = Direction.X - ( Response.Normal.X * A.Size().X * 0.5f );
	}
	else if( AbsoluteNormal.Y > AbsoluteNormal.Z )
	{
		Response.Normal.X = 0.0f;
		Response.Normal.Y = Math::Sign( Response.Normal.Y );
		Response.Normal.Z = 0.0f;

		Response.Distance = Direction.Y - ( Response.Normal.Y * A.Size().Y * 0.5f );
	}
	else
	{
		Response.Normal.X = 0.0f;
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = Math::Sign( Response.Normal.Z );

		Response.Distance = Direction.Z - ( Response.Normal.Z * A.Size().Z * 0.5f );
	}

	// UI::AddCircle( PointOnBox + Response.Normal * Response.Distance, 5.0f, Color::Red );
	// UI::AddCircle( PointOnBox, 5.0f, Inside ? Color::Yellow : Color::Green );
	// UI::AddLine( PointOnBox, PointOnBox - Response.Normal, Color::Cyan );
	// UI::AddLine( PointOnBox, PointOnBox + Response.Normal, Response.Distance > 0.0f ? Color::Cyan : Color::Purple, 1.0f );

	return Response;
#if 0 // old
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

	return Response;
#endif
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

	Plane TrianglePlane = { VertexA, VertexB, VertexC };
	Response.Normal = TrianglePlane.Normal;
	Response.Distance = Response.Normal.Dot( VertexA ) + TrianglePlane.Distance * 0.986f;
	ByteToVector( A.Normal, Response.Normal );

	// const float Radius = Extent.X * Math::Abs( Response.Normal.X ) + Extent.Y * Math::Abs( Response.Normal.Y ) + Extent.Z * Math::Abs( Response.Normal.Z );
	// const float DistanceFromCenter = Response.Normal.Dot( Extent ) - Response.Distance;
	// Response.Distance = Radius * DistanceFromCenter;

	return Response;
}
