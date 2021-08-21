// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math/Vector.h>

struct BoundingBox
{
	BoundingBox() = default;
	BoundingBox( const Vector3D& Minimum, const Vector3D& Maximum );

	bool Intersects( const BoundingBox& B ) const;
	BoundingBox Combine( const BoundingBox& B ) const;

	Vector3D Minimum = Vector3D::Zero;
	Vector3D Maximum = Vector3D::Zero;

	static BoundingBox Combine( const BoundingBox& A, const BoundingBox& B );
};

struct BoundingBoxSIMD
{
	BoundingBoxSIMD() = default;
	BoundingBoxSIMD( const Vector3D& Minimum, const Vector3D& Maximum );
	BoundingBoxSIMD( const __m128& Minimum, const __m128& Maximum );

	bool Intersects( const BoundingBoxSIMD& B ) const;
	BoundingBoxSIMD Combine( const BoundingBoxSIMD& B ) const;

	__m128 Minimum;
	__m128 Maximum;

	static BoundingBoxSIMD Combine( const BoundingBoxSIMD& A, const BoundingBoxSIMD& B );

	BoundingBox Fetch() const;
};