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
}

void CPlaneBody::Collision( CBody* Body )
{
	// Planes currently only support mesh entities.
	if( Ghost || !Owner )
		return;

	if( !Body->Block )
		return;

	const auto Bounds = Body->GetBounds();
	const auto Center = ( Bounds.Maximum + Bounds.Minimum ) * 0.5f;
	const auto Delta = Center - PlaneOrigin;
	const auto Distance = PlaneNormal.Dot( Delta );
	const auto Tangent = PlaneNormal.Cross( Delta );
	const auto ProjectedNormal = Tangent.Cross( PlaneNormal );
	const auto ProjectedPosition = PlaneOrigin + ProjectedNormal;

	auto ProjectedVector = Bounds.Maximum - ProjectedPosition;
	ProjectedVector = Math::ProjectOnVector( ProjectedVector, PlaneNormal );
	const auto DistanceToMaximum = ProjectedVector.Length();

	ProjectedVector = Bounds.Minimum - ProjectedPosition;
	ProjectedVector = Math::ProjectOnVector( ProjectedVector, PlaneNormal );
	const auto DistanceToMinimum = ProjectedVector.Length();
	const bool MaximumClosest = DistanceToMaximum < DistanceToMinimum;

	float ProjectedDistance = MaximumClosest ? DistanceToMaximum : DistanceToMinimum;
	const auto ClosestPosition = MaximumClosest ? Bounds.Maximum : Bounds.Minimum;

	Body->Depenetration += PlaneNormal * ProjectedDistance;

	if( ProjectedDistance > 0.001f )
	{
		Body->Contacts++;
	}

	UI::AddLine( PlaneOrigin, PlaneOrigin + Delta, Color::Green );
	UI::AddLine( PlaneOrigin, ProjectedPosition, Color::Blue );
	UI::AddLine( ProjectedPosition, ProjectedPosition - ProjectedDistance * PlaneNormal, Color::Red );
	UI::AddCircle( Center, 3.0f, Color::Red );
	UI::AddCircle( PlaneOrigin, 3.0f, Color::Blue );
	UI::AddCircle( ProjectedPosition, 3.0f, Color::Green );
	UI::AddCircle( ClosestPosition, 3.0f, Color::White );
	UI::AddAABB( Bounds.Minimum, Bounds.Maximum, Color::Green );
	UI::AddAABB( Bounds.Minimum + Body->Depenetration, Bounds.Maximum + Body->Depenetration, Color::Red );
}

void CPlaneBody::Tick()
{
	
}

void CPlaneBody::Debug()
{
	CBody::Debug();
	UI::AddLine( PlaneOrigin, PlaneOrigin + PlaneNormal, Color::Blue );
}