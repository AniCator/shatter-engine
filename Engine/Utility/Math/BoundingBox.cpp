// Copyright © 2017, Christiaan Bakker, All rights reserved.

#include "BoundingBox.h"

#include <Engine/Utility/Math.h>

BoundingBox::BoundingBox( const Vector3D& Minimum, const Vector3D& Maximum )
{
	this->Minimum = Minimum;
	this->Maximum = Maximum;
}

bool BoundingBox::Intersects( const BoundingBox& B ) const
{
	//// Temporary extent calculation.
	//const auto Center = ( Maximum + Minimum ) * 0.5f;
	//const auto Extent = ( Maximum - Minimum ) * 0.5f;

	//const auto BCenter = ( B.Maximum + B.Minimum ) * 0.5f;
	//const auto BExtent = ( B.Maximum - B.Minimum ) * 0.5f;

	//// SAT version. | About equal performance with branched.
	//const auto DeltaX = Center.X - BCenter.X;
	//const auto PivotX = ( Extent.X + BExtent.X ) - fabs( DeltaX );
	//if( PivotX <= 0.0f )
	//	return false;

	//const auto DeltaY = Center.Y - BCenter.Y;
	//const auto PivotY = ( Extent.Y + BExtent.Y ) - fabs( DeltaY );
	//if( PivotY <= 0.0f )
	//	return false;

	//const auto DeltaZ = Center.Z - BCenter.Z;
	//const auto PivotZ = ( Extent.Z + BExtent.Z ) - fabs( DeltaZ );
	//if( PivotZ <= 0.0f )
	//	return false;

	//return true;

	// Branched version in function call. (inlined)
	return Math::BoundingBoxIntersection( Minimum, Maximum, B.Minimum, B.Maximum );
}

BoundingBox BoundingBox::Combine( const BoundingBox& B ) const
{
	return Combine( *this, B );
}

Vector3D BoundingBox::Center() const
{
	return ( Maximum + Minimum ) * 0.5f;
}

Vector3D BoundingBox::Size() const
{
	return Maximum - Minimum;
}

BoundingBox BoundingBox::Combine( const BoundingBox& A, const BoundingBox& B )
{
	const Vector3D Minimum(
		Math::Min( A.Minimum.X, B.Minimum.X ),
		Math::Min( A.Minimum.Y, B.Minimum.Y ),
		Math::Min( A.Minimum.Z, B.Minimum.Z )
	);

	const Vector3D Maximum(
		Math::Max( A.Maximum.X, B.Maximum.X ),
		Math::Max( A.Maximum.Y, B.Maximum.Y ),
		Math::Max( A.Maximum.Z, B.Maximum.Z )
	);

	return { Minimum, Maximum };
}

BoundingBoxSIMD::BoundingBoxSIMD( const Vector3D& Minimum, const Vector3D& Maximum )
{
	this->Minimum = _mm_setr_ps( Minimum.X, Minimum.Y, Minimum.Z, 0.0f );
	this->Maximum = _mm_setr_ps( Maximum.X, Maximum.Y, Maximum.Z, 1.0f );
}

BoundingBoxSIMD::BoundingBoxSIMD( const __m128& Minimum, const __m128& Maximum )
{
	this->Minimum = Minimum;
	this->Maximum = Maximum;
}

bool BoundingBoxSIMD::Intersects( const BoundingBoxSIMD& B ) const
{
	/*if( ( MinimumA.X < MaximumB.X && MaximumA.X > MinimumB.X ) &&
		( MinimumA.Y < MaximumB.Y && MaximumA.Y > MinimumB.Y ) &&
		( MinimumA.Z < MaximumB.Z && MaximumA.Z > MinimumB.Z ) )
	{
		return true;
	}

	return false;*/

	// MinimumA.X < MaximumB.X
	// MinimumA.Y < MaximumB.Y
	// MinimumA.Z < MaximumB.Z

	// MaximumA.X > MinimumB.X
	// MaximumA.Y > MinimumB.Y
	// MaximumA.Z > MinimumB.Z

	// Alternative
	// MinimumA.X < MaximumB.X
	// MinimumA.Y < MaximumB.Y
	// MinimumA.Z < MaximumB.Z

	// MaximumB.X < MinimumA.X
	// MaximumB.Y < MinimumA.Y
	// MaximumB.Z < MinimumA.Z

	// MinimumA.X < MaximumB.X && MaximumA.X > MinimumB.X -> TestX
	// MinimumA.Y < MaximumB.Y && MaximumA.Y > MinimumB.Y -> TestY
	// MinimumA.Z < MaximumB.Z && MaximumA.Z > MinimumB.Z -> TestZ
	// TestX && TestY && TestZ

	// Axis test
	// MinimumA.X < MaximumB.X && MaximumA.X > MinimumB.X
	// Less than, greater than, AND

	// Set mask to 0
	const auto ComparisonMask = _mm_set_ps1( 0 );

	// Minimum A is less than Maximum B
	const auto ResultMin = _mm_cmple_ps( Minimum, B.Maximum );

	// Maximum A is larger than Minimum B
	const auto ResultMax = _mm_cmpge_ps( Maximum, B.Minimum );

	// Compare the two.
	const auto ResultAnd = _mm_and_ps( ResultMin, ResultMax );

	// Check for equality with 0 mask.
	const auto ResultCmp = _mm_cmpeq_ss( ResultAnd, ComparisonMask );

	// Get the result.
	const auto Mask = _mm_movemask_ps( ResultCmp );

	return ( Mask & 0x01 ) == 0;

	// Store the result.
	Vector4D Vector = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f );
	_mm_storeu_ps( &Vector.X, ResultAnd );
	const int X = static_cast<int>( Vector.X );
	const int Y = static_cast<int>( Vector.Y );
	const int Z = static_cast<int>( Vector.Z );
	return 0 == X == Y == Z;
}

BoundingBoxSIMD BoundingBoxSIMD::Combine( const BoundingBoxSIMD& B ) const
{
	return Combine( *this, B );
}

BoundingBoxSIMD BoundingBoxSIMD::Combine( const BoundingBoxSIMD& A, const BoundingBoxSIMD& B )
{
	const auto Minimum = _mm_min_ps( A.Minimum, B.Minimum );
	const auto Maximum = _mm_max_ps( A.Maximum, B.Maximum );
	return { Minimum, Maximum };
}

BoundingBox BoundingBoxSIMD::Fetch() const
{
	Vector4D StoredMinimum = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f );
	_mm_storeu_ps( &StoredMinimum.X, Minimum );

	Vector4D StoredMaximum = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f );
	_mm_storeu_ps( &StoredMaximum.X, Maximum );

	return {
		{ StoredMinimum.X, StoredMinimum.Y, StoredMinimum.Z },
		{ StoredMaximum.X, StoredMaximum.Y, StoredMaximum.Z }
	};
}
