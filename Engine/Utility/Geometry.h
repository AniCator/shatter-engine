// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Math.h"
#include "GeometryResult.h"
#include <Engine/Physics/Body/Body.h>

namespace Geometry
{	
	inline float RayInBoundingBox( const Vector3D& Origin, const Vector3D& Direction, const BoundingBox& Bounds )
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
			return Maximum;

		return Minimum;
	}

	inline Result LineInBoundingBox( const Vector3D& Start, const Vector3D& End, const BoundingBox& Bounds )
	{
		Result Result;
		const auto Direction = End - Start;
		const auto Normalized = Direction.Normalized();

		const auto Distance = RayInBoundingBox( Start, Normalized, Bounds );
		Result.Hit = Distance >= 0.0f && Distance * Distance <= Direction.LengthSquared();
		Result.Distance = Result.Hit ? Distance : -1.0f;
		Result.Position = Result.Hit ? Start + Normalized * Distance : End;

		return Result;
	}
}