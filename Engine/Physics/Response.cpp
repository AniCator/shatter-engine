// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Response.h"

#include <Engine/Utility/Math.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Display/Rendering/Mesh.h>

#include <Engine/Utility/Math/BoundingBox.h>
#include <Engine/Utility/Math/Plane.h>

// #define PerformClassIII

namespace Test
{
	Vector3D PointOnLine( const Vector3D& Start, const Vector3D& End, const Vector3D& Point )
	{
		// Calculate the local space ray.
		auto Ray = End - Start;

		// Transform the point into the line's local space.
		auto RelativePoint = Point - Start;

		// Project the point onto the ray.
		float DistanceAlongLine = RelativePoint.Dot( Ray ) / Ray.LengthSquared();

		// Clamp it to a range of [0-1].
		DistanceAlongLine = Math::Saturate( DistanceAlongLine );

		// Construct the new point.
		return Start + Ray * DistanceAlongLine;
	}

	bool IsPointInLine( const Vector3D& Point, const Vector3D& Start, const Vector3D& End )
	{
		auto Closest = PointOnLine( Start, End, Point );
		return Math::Equal( ( Closest - Point ).LengthSquared(), 0.0f );
	}

	bool IsPointInPlane( const Vector3D& Point, const Plane& Plane )
	{
		return Math::Equal( Point.Dot( Plane.Normal ) - Plane.Distance, 0.0f );
	}

	Vector3D PointOnPlane( const Vector3D& Point, const Plane& Plane )
	{
		return Math::ProjectOnPlane( Point, Plane );
	}

	bool IsPointInSphere( const Vector3D& Point, const BoundingSphere& Sphere )
	{
		return Point.DistanceSquared( Sphere.Origin() ) < Sphere.GetRadiusSquared();
	}

	Vector3D PointOnSphere( const BoundingSphere& Sphere, const Vector3D& Point )
	{
		// Fetch the origin.
		auto Origin = Sphere.Origin();

		// Transform the point into the sphere's local space.
		auto RelativePoint = Point - Origin;

		// Normalize the point vector into a directional unit vector.
		RelativePoint.Normalize();

		// Scale it by the size of the sphere to project it onto its surface.
		RelativePoint *= Sphere.GetRadius();

		// Transform the point back into world space.
		return RelativePoint + Origin;
	}

	bool IsPointInAABB( const Vector3D& Point, const BoundingBox& Box )
	{
		if( Point.X < Box.Minimum.X || Point.Y < Box.Minimum.Y || Point.Z < Box.Minimum.Z )
		{
			return false;
		}

		if( Point.X > Box.Maximum.X || Point.Y > Box.Maximum.Y || Point.Z > Box.Maximum.Z )
		{
			return false;
		}

		return true;
	}

	Vector3D PointOnAABB( const BoundingBox& Box, const Vector3D& Point )
	{
		Vector3D Result = Point;

		// Clamp the point to the extents of the box.
		Result.X = Math::Max( Result.X, Box.Minimum.X );
		Result.Y = Math::Max( Result.Y, Box.Minimum.Y );
		Result.Z = Math::Max( Result.Z, Box.Minimum.Z );

		Result.X = Math::Min( Result.X, Box.Maximum.X );
		Result.Y = Math::Min( Result.Y, Box.Maximum.Y );
		Result.Z = Math::Min( Result.Z, Box.Maximum.Z );

		return Result;
	}

	bool IsPointInTriangle( const Vector3D& Point, const VertexFormat& A, const VertexFormat& B, const VertexFormat& C )
	{
		// Calculate the triangle corners relative to the given point.
		const Vector3D RelativeA = A.Position - Point;
		const Vector3D RelativeB = B.Position - Point;
		const Vector3D RelativeC = C.Position - Point;

		// Calculate the triangle edge normals.
		const Vector3D U = RelativeB.Cross( RelativeC );
		const Vector3D V = RelativeC.Cross( RelativeA );
		const Vector3D W = RelativeA.Cross( RelativeB );

		if( U.Dot( V ) < 0.0f )
		{
			return false;
		}
		else if( U.Dot( W ) < 0.0f )
		{
			return false;
		}

		return true;
	}

	Vector3D PointOnTriangle( const VertexFormat& A, const VertexFormat& B, const VertexFormat& C, const Vector3D& Point )
	{
		// Calculate the triangle's plane.
		const Plane Plane = { A.Position, B.Position, C.Position };
		const auto Closest = Test::PointOnPlane( Point, Plane );
		if( Test::IsPointInTriangle( Closest, A, B, C ) )
		{
			// The closest point is located within the triangle.
			return Closest;
		}

		// Find the closest points on the triangle edges.
		const Vector3D AB = Test::PointOnLine( A.Position, B.Position, Point );
		const Vector3D BC = Test::PointOnLine( B.Position, C.Position, Point );
		const Vector3D CA = Test::PointOnLine( C.Position, A.Position, Point );

		// Calculate their distances to the given point.
		float LengthSquaredAB = Point.DistanceSquared( AB );
		float LengthSquaredBC = Point.DistanceSquared( BC );
		float LengthSquaredCA = Point.DistanceSquared( CA );

		// Test which projected point is closest.
		if( LengthSquaredAB < LengthSquaredBC && LengthSquaredAB < LengthSquaredCA )
		{
			return AB;
		}

		if( LengthSquaredBC < LengthSquaredAB && LengthSquaredBC < LengthSquaredCA )
		{
			return BC;
		}

		return CA;
	}
}

struct Triangle
{
	VertexFormat Points[3];
};

struct Interval
{
	float Minimum = 0.0f;
	float Maximum = 0.0f;

	static Interval Get( const BoundingBox& Box, const Vector3D& Axis )
	{
		Vector3D Corners[8];
		Corners[0] = { Box.Minimum.X, Box.Maximum.Y, Box.Maximum.Z };
		Corners[1] = { Box.Minimum.X, Box.Maximum.Y, Box.Minimum.Z };
		Corners[2] = { Box.Minimum.X, Box.Minimum.Y, Box.Maximum.Z };
		Corners[3] = { Box.Minimum.X, Box.Minimum.Y, Box.Minimum.Z };
		Corners[4] = { Box.Maximum.X, Box.Maximum.Y, Box.Maximum.Z };
		Corners[5] = { Box.Maximum.X, Box.Maximum.Y, Box.Minimum.Z };
		Corners[6] = { Box.Maximum.X, Box.Minimum.Y, Box.Maximum.Z };
		Corners[7] = { Box.Maximum.X, Box.Minimum.Y, Box.Minimum.Z };

		Interval Interval;
		Interval.Minimum = Axis.Dot( Corners[0] );
		for( uint8_t Index = 0; Index < 8; Index++ )
		{
			const float DistanceToCorner = Axis.Dot( Corners[Index] );
			Interval.Minimum = Math::Min( Interval.Minimum, DistanceToCorner );
			Interval.Maximum = Math::Max( Interval.Maximum, DistanceToCorner );
		}

		return Interval;
	}

	static Interval Get( const Triangle& Triangle, const Vector3D& Axis )
	{
		Interval Interval;
		Interval.Minimum = Axis.Dot( Triangle.Points[0].Position );
		for( uint8_t Index = 0; Index < 8; Index++ )
		{
			const float DistanceToCorner = Axis.Dot( Triangle.Points[Index].Position );
			Interval.Minimum = Math::Min( Interval.Minimum, DistanceToCorner );
			Interval.Maximum = Math::Max( Interval.Maximum, DistanceToCorner );
		}

		return Interval;
	}
};

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
	const Vector3D PointOnPlane = Math::ProjectOnPlane( Origin, Plane );
	const Vector3D Direction = Origin - PointOnPlane;

	// UI::AddLine( Origin, PointOnPlane, Color::Green );
	// UI::AddCircle( Origin, 5.0f, Color::Green );
	// UI::AddCircle( PointOnPlane, 5.0f, Color::Red );

	const float DistanceSquared = Direction.LengthSquared();
	if( DistanceSquared > Sphere.GetRadiusSquared() )
		return {}; // We're not colliding with the plane.

	CollisionResponse Response;
	Response.Normal = PointOnPlane - Origin;
	Response.Distance = Response.Normal.Normalize();

	const Vector3D PointOnSphere = Response.Normal * Sphere.GetRadius() + Origin;
	// const Vector3D PointOnSphere = Origin - Response.Normal * Sphere.GetRadius();
	Response.Distance = PointOnPlane.Distance( PointOnSphere );
	Response.Point = PointOnPlane + ( PointOnSphere - PointOnPlane ) * 0.5f;

	// UI::AddSphere( Origin, Sphere.GetRadius(), Color::Cyan );
	// UI::AddCircle( PointOnPlane, 5.0f, Color::Green );

	return Response;
}

CollisionResponse Response::SphereAABB( const BoundingSphere& Sphere, const BoundingBox& Box )
{
	const Vector3D Origin = Sphere.Origin();
	const Vector3D LocalOrigin = ( Box.Center() - Origin ) / Box.Size();

	// Project the sphere's origin onto the bounding box to calculate the closest point.
	const Vector3D PointOnBox = Math::ProjectOnAABB( Origin, Box );
	const Vector3D Direction = ( Origin - PointOnBox );

	const float DistanceSquared = Direction.LengthSquared();
	if( DistanceSquared > Sphere.GetRadiusSquared() )
		return {}; // We're not colliding with the bounding box.

	CollisionResponse Response;
	Response.Normal = PointOnBox - Origin;

	/*const Vector3D HalfSize = Box.Size() * 0.5f;

	Response.Distance = Response.Normal.X;
	if( Response.Distance > HalfSize.X )
		Response.Distance = HalfSize.X;
	if( Response.Distance < -HalfSize.X )
		Response.Distance = -HalfSize.X;
	Response.Point.X = Response.Distance;

	Response.Distance = Response.Normal.Y;
	if( Response.Distance > HalfSize.Y )
		Response.Distance = HalfSize.Y;
	if( Response.Distance < -HalfSize.Y )
		Response.Distance = -HalfSize.Y;
	Response.Point.Y = Response.Distance;

	Response.Distance = Response.Normal.Z;
	if( Response.Distance > HalfSize.Z )
		Response.Distance = HalfSize.Z;
	if( Response.Distance < -HalfSize.Z )
		Response.Distance = -HalfSize.Z;
	Response.Point.Z = Response.Distance;

	Response.Normal = Origin - Response.Point;*/

	// Find the dominant axis.
	/*const Vector3D AbsoluteNormal = Math::Abs( Response.Normal );
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
	}*/

	Response.Normal.Normalize();
	// const Vector3D PointOnSphere = Origin + Response.Normal * Sphere.GetRadius();
	// Response.Distance = PointOnBox.Distance( PointOnSphere );
	// Response.Point = PointOnBox + ( PointOnSphere - PointOnBox ) * 0.5f;
	// Response.Point = LocalOrigin.Normalized() * Sphere.GetRadius() + Origin;
	Response.Point = PointOnBox;
	// Response.Distance = Sphere.GetRadius() - ( Response.Point - LocalOrigin ).Length();
	Response.Distance = Sphere.GetRadius() - PointOnBox.Distance( Origin );

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
#if 0
	const Vector3D PointOnBox = Math::ProjectOnAABB( A.Center(), B );
	Vector3D Direction = PointOnBox - A.Center();

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
		UI::AddLine( PointOnBox, PointOnBox + Response.Normal, Color::Yellow );
	}
	else
	{
		// The sphere is touching the box from outside.
		Response.Normal = Direction;
		UI::AddLine( PointOnBox, PointOnBox + Response.Normal, Color::Purple );
	}

	// Find the dominant axis.
	const Vector3D AbsoluteNormal = Math::Abs( Response.Normal );
	if( AbsoluteNormal.X > AbsoluteNormal.Y && AbsoluteNormal.X > AbsoluteNormal.Z )
	{
		Response.Normal.X = Math::Sign( Response.Normal.X );
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = 0.0f;

		Response.Distance = Direction.X - ( Response.Normal.X * B.Size().X * 0.5f );
	}
	else if( AbsoluteNormal.Y > AbsoluteNormal.Z )
	{
		Response.Normal.X = 0.0f;
		Response.Normal.Y = Math::Sign( Response.Normal.Y );
		Response.Normal.Z = 0.0f;

		Response.Distance = Direction.Y - ( Response.Normal.Y * B.Size().Y * 0.5f );
	}
	else
	{
		Response.Normal.X = 0.0f;
		Response.Normal.Y = 0.0f;
		Response.Normal.Z = Math::Sign( Response.Normal.Z );

		Response.Distance = Direction.Z - ( Response.Normal.Z * B.Size().Z * 0.5f );
	}

	UI::AddLine( PointOnBox, PointOnBox + Response.Normal, Color::Cyan );
	UI::AddText( PointOnBox, std::to_string( Response.Distance ).c_str() );

	return Response;
#else // old
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

namespace SAT
{
	// Class I: Checks if the triangle's bounds overlap with that of the box.
	bool Bounds( const Vector3D& A, const Vector3D& B, const Vector3D& C, const Vector3D& Extent )
	{
		if( Math::VectorMax( A.X, B.X, C.X ) < -Extent.X || Math::VectorMin( A.X, B.X, C.X ) > Extent.X )
			return false;

		if( Math::VectorMax( A.Y, B.Y, C.Y ) < -Extent.Y || Math::VectorMin( A.Y, B.Y, C.Y ) > Extent.Y )
			return false;

		if( Math::VectorMax( A.Z, B.Z, C.Z ) < -Extent.Z || Math::VectorMin( A.Z, B.Z, C.Z ) > Extent.Z )
			return false;

		return true;
	}

	bool Axis( const Vector3D& Axis, const Vector3D& A, const Vector3D& B, const Vector3D& C, const float& Radius )
	{
		const float DotA = A.Dot( Axis );
		const float DotB = B.Dot( Axis );
		const float DotC = C.Dot( Axis );
		return Math::Max( -Math::VectorMax( DotA, DotB, DotC ), Math::VectorMin( DotA, DotB, DotC ) ) > Radius;
	}

	// Class III: Checks all of the triangle edges against the bounds. (assuming the box is at the origin)
	bool ClassIII( const Vector3D& VertexA, const Vector3D& VertexB, const Vector3D& VertexC, const Vector3D& EdgeA, const Vector3D& EdgeB, const Vector3D& EdgeC, const Vector3D& Extent )
	{
		Vector3D A00 = Vector3D( 0.0f, -EdgeA.Z, EdgeA.Y );
		const float PointA = VertexA.Z * VertexB.Y - VertexA.Y * VertexB.Z;
		const float PointC = VertexC.Z * ( VertexB.Y - VertexA.Z ) - VertexC.Z * ( VertexB.Z - VertexA.Z );
		const float RadiusA00 = Extent.Y * Math::Abs( EdgeA.Z ) + Extent.Z * Math::Abs( EdgeA.Y );
		if( Math::Max( -Math::Max( PointA, PointC ), Math::Min( PointA, PointC ) ) > RadiusA00 )
		{
			return false;
		}

		Vector3D A01 = Vector3D( 0.0f, -EdgeB.Z, EdgeB.Y );
		const float RadiusA01 = Extent.Y * Math::Abs( EdgeB.Z ) + Extent.Z * Math::Abs( EdgeB.Y );
		if( Axis( A01, VertexA, VertexB, VertexC, RadiusA01 ) )
			return false;

		Vector3D A02 = Vector3D( 0.0f, -EdgeC.Z, EdgeC.Y );
		const float RadiusA02 = Extent.Y * Math::Abs( EdgeC.Z ) + Extent.Z * Math::Abs( EdgeC.Y );
		if( Axis( A02, VertexA, VertexB, VertexC, RadiusA02 ) )
			return false;

		Vector3D A10 = Vector3D( EdgeA.Z, 0.0f, -EdgeA.X );
		const float RadiusA10 = Extent.X * Math::Abs( EdgeA.Z ) + Extent.Z * Math::Abs( EdgeA.X );
		if( Axis( A10, VertexA, VertexB, VertexC, RadiusA10 ) )
			return false;

		Vector3D A11 = Vector3D( EdgeB.Z, 0.0f, -EdgeB.X );
		const float RadiusA11 = Extent.X * Math::Abs( EdgeB.Z ) + Extent.Z * Math::Abs( EdgeB.X );
		if( Axis( A11, VertexA, VertexB, VertexC, RadiusA11 ) )
			return false;

		Vector3D A12 = Vector3D( EdgeC.Z, 0.0f, -EdgeC.X );
		const float RadiusA12 = Extent.X * Math::Abs( EdgeC.Z ) + Extent.Z * Math::Abs( EdgeC.X );
		if( Axis( A12, VertexA, VertexB, VertexC, RadiusA12 ) )
			return false;

		Vector3D A20 = Vector3D( -EdgeA.Y, EdgeA.X, 0.0f );
		const float RadiusA20 = Extent.X * Math::Abs( EdgeA.Y ) + Extent.Z * Math::Abs( EdgeA.X );
		if( Axis( A20, VertexA, VertexB, VertexC, RadiusA20 ) )
			return false;

		Vector3D A21 = Vector3D( -EdgeB.Y, EdgeB.X, 0.0f );
		const float RadiusA21 = Extent.X * Math::Abs( EdgeB.Y ) + Extent.Z * Math::Abs( EdgeB.X );
		if( Axis( A21, VertexA, VertexB, VertexC, RadiusA21 ) )
			return false;

		Vector3D A22 = Vector3D( -EdgeC.Y, EdgeC.X, 0.0f );
		const float RadiusA22 = Extent.X * Math::Abs( EdgeC.Y ) + Extent.Z * Math::Abs( EdgeC.X );
		if( Axis( A22, VertexA, VertexB, VertexC, RadiusA22 ) )
			return false;

		return true;
	}
}

CollisionResponse Response::TriangleAABB( const VertexFormat& A, const VertexFormat& B, const VertexFormat& C, const Vector3D& Center, const Vector3D& Extent )
{
	CollisionResponse Response;

	// Move the box to the origin.
	const Vector3D VertexA = A.Position - Center;
	const Vector3D VertexB = B.Position - Center;
	const Vector3D VertexC = C.Position - Center;

	// Class I: Test the AABB against the bounds of the triangle.
	if( !SAT::Bounds( VertexA, VertexB, VertexC, Extent ) )
		return Response;

	// Direction vector pointing from B to A.
	const Vector3D EdgeA = VertexB - VertexA;

	// Direction vector pointing from C to B.
	const Vector3D EdgeB = VertexC - VertexB;

	// Direction vector pointing from A to C.
	const Vector3D EdgeC = VertexA - VertexC;

	// Plane TrianglePlane = { VertexA, VertexB, VertexC };
	Vector3D Normal = EdgeA.Cross( EdgeC ).Normalized();
	float Distance = -Normal.Dot( VertexA );

	// Class II: Check if the plane is out of range.
	const float Radius = Extent.Dot( Math::Abs( Response.Normal ) );
	if( Math::Abs( Distance ) < Radius )
		return Response;

#if defined( PerformClassIII )
	// Class III: Test against each triangle edge.
	if( !SAT::ClassIII( VertexA, VertexB, VertexC, EdgeA, EdgeB, EdgeC, Extent ) )
		return Response;
#endif

	Response.Normal = Normal;
	Response.Distance = Distance;

	return Response;
}

CollisionResponse Response::TriangleSphere( const VertexFormat& A, const VertexFormat& B, const VertexFormat& C, const BoundingSphere& Sphere )
{
	const auto Origin = Sphere.Origin();
	const auto Closest = Test::PointOnTriangle( A, B, C, Origin );
	if( Closest.DistanceSquared( Origin ) >= Sphere.GetRadiusSquared() )
	{
		return {};
	}

	Plane Plane;
	Plane.Origin = Origin;
	Plane.Normal = ( C.Position - A.Position ).Cross( B.Position - A.Position );
	Plane.Normal.Normalize();
	Plane.Distance = Plane.Normal.Dot( A.Position );

	CollisionResponse Response = SpherePlane( Sphere, Plane ); // TODO: Naughty because we're creating the plane twice. (PointOnTriangle creates a plane too)
	return Response;
}
