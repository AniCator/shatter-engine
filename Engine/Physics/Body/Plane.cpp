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

	const auto Transform = Owner->GetTransform();
	CalculateBounds();

	// Configure a point on the plane.
	PlaneOrigin = Transform.GetPosition();

	// Calculate the plane normal.
	const auto ForwardVector = Math::EulerToDirection( Transform.GetOrientation() );
	const auto RightVector = ForwardVector.Cross( WorldUp );
	PlaneNormal = RightVector.Cross( ForwardVector );

	if( PlaneNormal.LengthSquared() < 0.01f )
	{
		PlaneNormal = WorldRight.Cross( ForwardVector );
	}
}

bool CPlaneBody::Collision( CBody* Body )
{
	bool Collided = false;

	// Planes currently only support mesh entities.
	if( Ghost || !Owner )
		return Collided;

	if( !Body->Block )
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

	// Project a vector along the plane towards the interacting body.
	const auto Tangent = SignedNormal.Cross( Delta );
	const auto ProjectedNormal = Tangent.Cross( SignedNormal );
	const auto ProjectedPosition = PlaneOrigin + ProjectedNormal;

	auto ProjectedVector = Bounds.Maximum - ProjectedPosition;
	ProjectedVector = Math::ProjectOnVector( ProjectedVector, SignedNormal );
	const auto DistanceToMaximum = ProjectedVector.Length();

	ProjectedVector = Bounds.Minimum - ProjectedPosition;
	ProjectedVector = Math::ProjectOnVector( ProjectedVector, SignedNormal );
	const auto DistanceToMinimum = ProjectedVector.Length();
	const bool MaximumClosest = DistanceToMaximum < DistanceToMinimum;

	float ProjectedDistance = MaximumClosest ? DistanceToMaximum : DistanceToMinimum;
	const auto ClosestPosition = MaximumClosest ? Bounds.Maximum : Bounds.Minimum;

	Body->Depenetration -= SignedNormal * ProjectedDistance;

	if( ProjectedDistance > 0.001f )
	{
		Body->Contacts++;
		Collided = true;
	}

	// UI::AddLine( PlaneOrigin, PlaneOrigin + Delta, Color::Green );
	// UI::AddLine( PlaneOrigin, ProjectedPosition, Color::Blue );
	// UI::AddLine( ProjectedPosition, ProjectedPosition - ProjectedDistance * PlaneNormal, Color::Red );
	// UI::AddCircle( Center, 3.0f, Color::Red );
	// UI::AddCircle( PlaneOrigin, 3.0f, Color::Blue );
	// UI::AddCircle( ProjectedPosition, 3.0f, Color::Green );
	// UI::AddCircle( ClosestPosition, 3.0f, Color::White );
	// UI::AddAABB( Bounds.Minimum, Bounds.Maximum, Color::Green );
	// UI::AddAABB( Bounds.Minimum - Body->Depenetration, Bounds.Maximum - Body->Depenetration, Color::Red );

	// UI::AddLine( PlaneOrigin, PlaneOrigin + ProjectedNormal, Color::Blue );
	// UI::AddLine( PlaneOrigin, PlaneOrigin + SignedNormal, Color( 255, 255, 0 ) );

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
