// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Math.h"

namespace Geometry
{
	struct Result
	{
		Vector3D Position = Vector3D::Zero;
		bool Hit = false;
		float Distance = 0.0f;
	};
	
	inline float RayInBoundingBox( const Vector3D& Origin, const Vector3D& Direction, const FBounds& Bounds )
	{
		// Direction has to be more than 0.
		const auto ScaleX = Math::Equal( Direction.X, 0.0f ) ? 1.0f : Direction.X;
		const auto ScaleY = Math::Equal( Direction.Y, 0.0f ) ? 1.0f : Direction.Y;
		const auto ScaleZ = Math::Equal( Direction.Z, 0.0f ) ? 1.0f : Direction.Z;
		
		const auto XMinimum = ( Bounds.Minimum.X - Origin.X ) * Direction.X;
		const auto XMaximum = ( Bounds.Maximum.X - Origin.X ) * Direction.X;

		const auto YMinimum = ( Bounds.Minimum.Y - Origin.Y ) * Direction.Y;
		const auto YMaximum = ( Bounds.Maximum.Y - Origin.Y ) * Direction.Y;

		const auto ZMinimum = ( Bounds.Minimum.Z - Origin.Z ) * Direction.Z;
		const auto ZMaximum = ( Bounds.Maximum.Z - Origin.Z ) * Direction.Z;

		const auto Minimum = Math::Max(
			Math::Max(
				Math::Min( XMinimum, XMaximum ),
				Math::Min( YMinimum, YMaximum )
			),
			Math::Max( ZMinimum, ZMaximum )
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

	inline Result LineInBoundingBox( const Vector3D& Start, const Vector3D& End, const FBounds& Bounds )
	{
		Result Result;
		const auto Direction = End - Start;

		const auto Distance = RayInBoundingBox( Start, Direction.Normalized(), Bounds );
		Result.Hit = Distance >= 0.0f && Distance * Distance <= Direction.LengthSquared();
		Result.Distance = Result.Hit ? Distance : -1.0f;
		Result.Position = Result.Hit ? Start + Direction * Distance : End;

		return Result;
	}
}