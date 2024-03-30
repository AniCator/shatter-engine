// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Geometry.h"

// #define PerformanceCounter

#ifdef PerformanceCounter
#include <Engine/Profiling/Profiling.h>
#endif

float Geometry::RayInBoundingBox( const Vector3D& Origin, const Vector3D& Direction, const BoundingBox& Bounds )
{
	const auto InverseDirection = 1.0f / Direction;
		
	const auto XMinimum = ( Bounds.Minimum.X - Origin.X ) * InverseDirection.X;
	const auto XMaximum = ( Bounds.Maximum.X - Origin.X ) * InverseDirection.X;

	const auto YMinimum = ( Bounds.Minimum.Y - Origin.Y ) * InverseDirection.Y;
	const auto YMaximum = ( Bounds.Maximum.Y - Origin.Y ) * InverseDirection.Y;

	const auto ZMinimum = ( Bounds.Minimum.Z - Origin.Z ) * InverseDirection.Z;
	const auto ZMaximum = ( Bounds.Maximum.Z - Origin.Z ) * InverseDirection.Z;

	const auto Minimum = Math::Max(
		Math::Max(
			Math::Min( XMinimum, XMaximum ),
			Math::Min( YMinimum, YMaximum )
		),
		Math::Min( ZMinimum, ZMaximum )
	);

	const auto Maximum = Math::Min(
		Math::Min(
			Math::Max( XMinimum, XMaximum ),
			Math::Max( YMinimum, YMaximum )
		),
		Math::Max( ZMinimum, ZMaximum )
	);

	if( Maximum < 0.0f )
		return -1.0f;

	if( Minimum > Maximum )
		return -1.0f;

	if( Minimum < 0.0f )
		return 0.0f; // We're inside of the box.

	// NOTE: This is the old code for being inside the box returned the other side of the box.
	/*if( Minimum < 0.0f )
		return Maximum;*/

	return Minimum;
}

Geometry::Result Geometry::LineInBoundingBox( const Vector3D& Start, const Vector3D& End, const BoundingBox& Bounds )
{
	Result Result;
	const auto Direction = End - Start;
	auto Normalized = Direction;
	float Length = Normalized.Normalize();

	const auto Distance = RayInBoundingBox( Start, Normalized, Bounds );
	Result.Hit = Distance >= 0.0f && Distance <= Length;
	Result.Distance = Result.Hit ? Distance : -1.0f;
	Result.Position = Result.Hit ? Start + Normalized * Distance : End;

	// Find the dominant axis.
	Result.Normal = ( Result.Position - Bounds.Center() ) / Bounds.Size();
	const Vector3D AbsoluteNormal = Math::Abs( Result.Normal );
	if( AbsoluteNormal.X > AbsoluteNormal.Y && AbsoluteNormal.X > AbsoluteNormal.Z )
	{
		Result.Normal.X = Math::Sign( Result.Normal.X );
		Result.Normal.Y = 0.0f;
		Result.Normal.Z = 0.0f;
	}
	else if( AbsoluteNormal.Y > AbsoluteNormal.Z )
	{
		Result.Normal.X = 0.0f;
		Result.Normal.Y = Math::Sign( Result.Normal.Y );
		Result.Normal.Z = 0.0f;
	}
	else
	{
		Result.Normal.X = 0.0f;
		Result.Normal.Y = 0.0f;
		Result.Normal.Z = Math::Sign( Result.Normal.Z );
	}

#ifdef PerformanceCounter
	CProfiler::Get().AddCounterEntry( ProfileTimeEntry( "LineInBoundingBox", 1 ), true, false );
#endif

	return Result;
}

float Geometry::RayInSphere( const Vector3D& Origin, const Vector3D& Direction, const BoundingSphere& Bounds )
{
	const auto LocalOrigin = Bounds.Origin() - Origin;
	const auto Radius = Bounds.GetRadiusSquared();
	const auto DistanceToOrigin = LocalOrigin.LengthSquared();
	const auto Cosine = LocalOrigin.Dot( Direction );
	const auto Bias = DistanceToOrigin - Cosine * Cosine;
	const auto DistanceToSurface = sqrt( Radius - Bias );

	if( ( Radius - Bias ) < 0.0f )
	{
		// Ray did not intersect with the sphere.
		return -1.0f;
	}
	else if( DistanceToOrigin < Radius )
	{
		return Cosine + DistanceToSurface;
	}

	return Cosine - DistanceToSurface;
}

Geometry::Result Geometry::LineInSphere( const Vector3D& Start, const Vector3D& End, const BoundingSphere& Bounds )
{
	Result Result;
	const auto Direction = End - Start;
	auto Normalized = Direction;
	float Length = Normalized.Normalize();

	const auto Distance = RayInSphere( Start, Normalized, Bounds );
	Result.Hit = Distance >= 0.0f && Distance <= Length;
	Result.Distance = Result.Hit ? Distance : -1.0f;
	Result.Position = Result.Hit ? Start + Normalized * Distance : End;

	Result.Normal = Result.Position - Bounds.Origin();
	Result.Normal.Normalize();

#ifdef PerformanceCounter
	CProfiler::Get().AddCounterEntry( ProfileTimeEntry( "LineInSphere", 1 ), true, false );
#endif

	return Result;
}

float Geometry::RayInPlane( const Vector3D& Origin, const Vector3D& Direction, const Plane& Plane )
{
	float DdotN = Direction.Dot( Plane.Normal );
	float OdotN = Origin.Dot( Plane.Normal );

	if( DdotN >= 0.0f )
	{
		return -1.0f;
	}

	float Distance = ( Plane.Distance - OdotN ) / DdotN;
	if( Distance >= 0.0f )
	{
		return Distance;
	}

	return -1.0f;
}

Geometry::Result Geometry::LineInPlane( const Vector3D& Start, const Vector3D& End, const Plane& Plane )
{
	Geometry::Result Result;
	const auto Direction = End - Start;
	auto Normalized = Direction;
	float Length = Normalized.Normalize();

	const auto Distance = RayInPlane( Start, Normalized, Plane );
	Result.Hit = Distance >= 0.0f && Distance <= Length;
	Result.Distance = Result.Hit ? Distance : -1.0f;
	Result.Position = Result.Hit ? Start + Normalized * Distance : End;

	Result.Normal = Plane.Normal;

#ifdef PerformanceCounter
	CProfiler::Get().AddCounterEntry( ProfileTimeEntry( "LineInPlane", 1 ), true, false );
#endif

	return Result;
}
