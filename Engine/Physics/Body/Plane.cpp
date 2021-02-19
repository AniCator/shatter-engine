// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Plane.h"

#include <Engine/Display/UserInterface.h>

CPlaneBody::CPlaneBody() : CBody()
{
	Block = false;
	Static = false;
	Stationary = true;
}

CPlaneBody::~CPlaneBody()
{

}

void CPlaneBody::PreCollision()
{
	// Planes currently only support mesh entities.
	if( Ghost || !Owner )
		return;

	auto Transform = Owner->GetTransform();
	CalculateBounds();

	// Configure a point on the plane.
	PlaneOrigin = Transform.GetPosition();

	// Calculate the plane normal.
	PlaneNormal = Transform.Rotate( Vector3D( 0.0f, 0.0f, 1.0f ) );
	PlaneNormal.Normalize();
}

bool CPlaneBody::Collision( CBody* Body )
{
	bool Collided = false;

	// Planes currently only support mesh entities.
	if( Ghost || !Owner )
		return Collided;

	if( !Body->Block || Body->Static )
		return Collided;

	if( ShouldIgnoreBody( Body ) )
		return Collided;

	const FBounds& BoundsA = Body->GetBounds();
	const FBounds& BoundsB = GetBounds();
	if( !Math::BoundingBoxIntersection( BoundsA.Minimum, BoundsA.Maximum, BoundsB.Minimum, BoundsB.Maximum ) )
	 	return Collided;

	const auto Bounds = Body->GetBounds();
	const auto Center = ( Bounds.Maximum + Bounds.Minimum ) * 0.5f;
	const auto Delta = Center - PlaneOrigin;
	const auto Distance = PlaneNormal.Dot( Delta );
	float Sign = Math::Sign( Distance );

	// Check if we're behind the plane, exit here if we're not checking back faces.
	if( !TwoSidedCollision && Sign < 0.0f )
	{
		return Collided;
	}

	if( ProjectToSurface )
	{
		// Ensure the interacting body is always pushed to the positive side of the plane.
		Sign = 1.0f;
	}

	// Multiply the plane normal with the sign value to ensure we can handle scenarios where the other body approaches the plane from its backside.
	const auto SignedNormal = PlaneNormal * Sign;

	const auto InvertedNormal = Vector3D( 1.0f - PlaneNormal.X, 1.0f - PlaneNormal.Y, 1.0f - PlaneNormal.Z );
	const auto Extent = ( Bounds.Maximum - Bounds.Minimum );
	const auto AbsoluteDistance = Math::Abs( Distance );
	if( ( AbsoluteDistance * AbsoluteDistance ) > Extent.LengthSquared() )
		return Collided;

	// Project a vector along the plane towards the interacting body.
	const auto Tangent = SignedNormal.Cross( Delta );
	const auto ProjectedNormal = Tangent.Cross( SignedNormal );
	const auto ProjectedPosition = PlaneOrigin + ProjectedNormal;

	auto ProjectedVectorMaximum = Bounds.Maximum - ProjectedPosition;
	ProjectedVectorMaximum = Math::ProjectOnVector( ProjectedVectorMaximum, SignedNormal );
	const auto DistanceToMaximum = ProjectedVectorMaximum.Length();

	auto ProjectedVectorMinimum = Bounds.Minimum - ProjectedPosition;
	ProjectedVectorMinimum = Math::ProjectOnVector( ProjectedVectorMinimum, SignedNormal );
	const auto DistanceToMinimum = ProjectedVectorMinimum.Length();
	const bool MaximumClosest = DistanceToMaximum < DistanceToMinimum;

	const auto ProjectedVector = MaximumClosest ? ProjectedVectorMaximum : ProjectedVectorMinimum;
	const float ProjectedProduct = Math::Max( 0.0f, ProjectedVector.Normalized().Dot( SignedNormal ) * -1.0f );

	float ProjectedDistance = MaximumClosest ? DistanceToMaximum : DistanceToMinimum;
	const auto ClosestPosition = MaximumClosest ? Bounds.Maximum : Bounds.Minimum;
	ProjectedDistance = Math::Clamp( ProjectedDistance, 0.0f, 1.0f ) * ProjectedProduct;

	if( ProjectedDistance > 0.00001f )
	{
		const Vector3D RelativeVelocity = Velocity - Body->Velocity;
		const float VelocityAlongNormal = RelativeVelocity.Dot( SignedNormal ) + ProjectedDistance * InverseMass;
		const float Scale = VelocityAlongNormal / ( InverseMass + Body->InverseMass );
		
		Body->Depenetration -= SignedNormal * ProjectedDistance;
		Body->Contacts++;
		Collided = true;
	}

	// UI::AddLine( PlaneOrigin, PlaneOrigin + Delta, Color::Green, 0.25 );
	// UI::AddLine( PlaneOrigin, ProjectedPosition, Color::Blue );
	// UI::AddLine( ProjectedPosition, ProjectedPosition - ProjectedDistance * PlaneNormal, Color::Red );
	// UI::AddCircle( Center, 3.0f, Color::Red );
	// UI::AddCircle( PlaneOrigin, 3.0f, Color::Blue );
	// UI::AddCircle( ProjectedPosition, 3.0f, Color::Green );
	// UI::AddCircle( ClosestPosition, 3.0f, Color::White );
	// UI::AddAABB( Bounds.Minimum, Bounds.Maximum, Color::Green, 0.25 );
	// UI::AddAABB( Bounds.Minimum - Body->Depenetration, Bounds.Maximum - Body->Depenetration, Color::Red, 0.25 );
	// UI::AddAABB( BoundsB.Minimum, BoundsB.Maximum - Body->Depenetration, Color::Red, 0.25 );
	// UI::AddLine( PlaneOrigin, PlaneOrigin + Tangent, Color( 255, 0, 255 ) );
	// UI::AddLine( PlaneOrigin, PlaneOrigin + PlaneNormal, Color::Blue );

	return Collided;
}

void CPlaneBody::Tick()
{
	
}

void CPlaneBody::Debug()
{
	CBody::Debug();
	UI::AddLine( PlaneOrigin, PlaneOrigin + PlaneNormal, Color::Blue );
}

BodyType CPlaneBody::GetType() const
{
	return BodyType::Plane;
}
