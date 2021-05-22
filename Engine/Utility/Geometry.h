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
	
	inline float RayInBoundingBox( const Vector3D& Origin, const Vector3D& Direction, const Vector3D& Minimum, const Vector3D& Maximum )
	{
		// Direction has to be more than 0.
		const auto ScaleX = Math::Equal( Direction.X, 0.0f ) ? 1.0f : Direction.X;
		const auto ScaleY = Math::Equal( Direction.Y, 0.0f ) ? 1.0f : Direction.Y;
		const auto ScaleZ = Math::Equal( Direction.Z, 0.0f ) ? 1.0f : Direction.Z;
		
		const auto tXMinimum = ( Minimum.X - Origin.X ) * Direction.X;
		const auto tXMaximum = ( Maximum.X - Origin.X ) * Direction.X;

		const auto tYMinimum = ( Minimum.Y - Origin.Y ) * Direction.Y;
		const auto tYMaximum = ( Maximum.Y - Origin.Y ) * Direction.Y;

		const auto tZMinimum = ( Minimum.Z - Origin.Z ) * Direction.Z;
		const auto tZMaximum = ( Maximum.Z - Origin.Z ) * Direction.Z;

		const auto tMinimum = Math::Max(
			Math::Max(
				Math::Min( tXMinimum, tXMaximum ),
				Math::Min( tYMinimum, tYMaximum )
			),
			Math::Max( tZMinimum, tZMaximum )
		);

		const auto tMaximum = Math::Min(
			Math::Min(
				Math::Max( tXMinimum, tXMaximum ),
				Math::Max( tYMinimum, tYMaximum )
			),
			Math::Max( tZMinimum, tZMaximum )
		);

		if( tMaximum < 0.0f )
			return -1.0f;

		if( tMinimum > tMaximum )
			return -1.0f;

		if( tMinimum < 0.0f )
			return tMaximum;

		return tMinimum;
	}

	inline Result LineInBoundingBox( const Vector3D& Start, const Vector3D& End, const Vector3D& Minimum, const Vector3D& Maximum )
	{
		Result Result;
		const auto Direction = End - Start;

		const auto Distance = RayInBoundingBox( Start, Direction.Normalized(), Minimum, Maximum );
		Result.Hit = Distance >= 0.0f && Distance * Distance <= Direction.LengthSquared();
		Result.Distance = Result.Hit ? Distance : -1.0f;
		Result.Position = Result.Hit ? Start + Direction * Distance : End;

		return Result;
	}
}