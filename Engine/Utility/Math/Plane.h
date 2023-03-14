#pragma once
#include <Engine/Utility/Math/Vector.h>

struct Plane
{
	Plane() = default;
	Plane( const Vector3D& Origin, const Vector3D& Normal )
	{
		this->Origin = Origin;
		this->Normal = Normal.Normalized();
		Distance = Origin.Dot( this->Normal );
	}

	// Construct plane using triangle.
	Plane( const Vector3D& A, const Vector3D& B, const Vector3D& C )
	{
		Origin = B - A;

		const Vector3D Edge = C - A;
		Normal = Origin.Cross( Edge ).Normalized();
		Distance = -Normal.Dot( A );
		Origin = A;
	}

	Vector3D Origin = Vector3D::Zero;
	Vector3D Normal = Vector3D( 0.0f, 0.0f, 1.0f );

	// Distance to ( 0, 0, 0 )
	float Distance = 0.0f;
};