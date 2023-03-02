// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Gizmo.h"

#include <Engine/Display/UserInterface.h>
#include <Engine/Display/Window.h>

#include <Engine/Physics/Body/Body.h>

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/World/World.h>

#include <Engine/Utility/Locator/InputLocator.h>

bool PointGizmo( CPointEntity* Entity, DragEntity& DragPersistence )
{
	auto& Input = CInputLocator::Get();
	const bool WantsToDrag = Input.IsMouseDown( EMouse::LeftMouseButton );
	if( !WantsToDrag && DragPersistence.Entity )
	{
		DragPersistence.Entity = nullptr;
		DragPersistence.Plane = -1;
	}

	if( !Entity )
	{
		if( DragPersistence.Entity )
		{
			Entity = DragPersistence.Entity;
		}

		return false;
	}

	auto* World = CWorld::GetPrimaryWorld();
	if( !World )
		return false;

	auto* Camera = World->GetActiveCamera();
	if( !Camera )
		return false;

	const auto& CameraSetup = Camera->GetCameraSetup();

	auto Transform = Entity->GetTransform();
	const auto Origin = Transform.GetPosition();
	const auto DistanceToCamera = CameraSetup.CameraPosition.Distance( Origin );
	const auto Distance = DistanceToCamera * 0.05f;
	const auto X = Vector3D( Distance, 0.0f, 0.0f );
	const auto Y = Vector3D( 0.0f, Distance, 0.0f );
	const auto Z = Vector3D( 0.0f, 0.0f, Distance );
	UI::AddLine( Origin, Origin + X, Color::Red );
	UI::AddLine( Origin, Origin + Y, Color::Green );
	UI::AddLine( Origin, Origin + Z, Color::Blue );

	// XY Plane
	const auto MousePositionFixed = Input.GetMousePosition();
	const auto MousePosition = Vector2D( MousePositionFixed.X, MousePositionFixed.Y );
	auto& Window = CWindow::Get();
	auto& Renderer = Window.GetRenderer();
	const auto MousePositionWorldSpace = Renderer.ScreenPositionToWorld( MousePosition );
	const auto PlaneDragLambda = [&] ( const Vector3D& PlaneNormal )
	{
		Vector3D Intersection;
		const auto PlaneInfluence = Vector3D( 1.0f, 1.0f, 1.0f ) - PlaneNormal;
		const auto PlaneColor = Color( PlaneNormal.Y * 255, PlaneNormal.X * 255, PlaneNormal.Z * 255 );
		const auto Minimum = Origin + Vector3D( 0.25f, 0.25f, 0.25f ) * PlaneInfluence * Distance;
		const auto Maximum = Origin + Vector3D( 0.75f, 0.75f, 0.75f ) * PlaneInfluence * Distance;
		const auto PlaneCenter = Origin + ( Maximum - Minimum );
		bool PlaneIntersection = Math::PlaneIntersection( Intersection, CameraSetup.CameraPosition, MousePositionWorldSpace, PlaneCenter, PlaneNormal );
		UI::AddCircle( PlaneCenter, 1.0f, PlaneColor );

		bool OutsideX =
			Intersection.X < Minimum.X ||
			Intersection.X > Maximum.X
			;

		bool OutsideY =
			Intersection.Y < Minimum.Y ||
			Intersection.Y > Maximum.Y
			;

		bool OutsideZ =
			Intersection.Z < Minimum.Z ||
			Intersection.Z > Maximum.Z
			;

		OutsideX = OutsideX && PlaneInfluence.X > 0.5f;
		OutsideY = OutsideY && PlaneInfluence.Y > 0.5f;
		OutsideZ = OutsideZ && PlaneInfluence.Z > 0.5f;

		const bool OutsideBounds = OutsideX || OutsideY || OutsideZ;

		int LocalPlane = -1;
		if( PlaneInfluence.X < 0.5f )
		{
			LocalPlane = 0;
		}
		else if( PlaneInfluence.Y < 0.5f )
		{
			LocalPlane = 1;
		}
		else if( PlaneInfluence.Z < 0.5f )
		{
			LocalPlane = 2;
		}

		if( OutsideBounds && DragPersistence.Plane != LocalPlane )
		{
			PlaneIntersection = false;
		}

		UI::AddAABB( Minimum, Maximum, PlaneIntersection ?
			PlaneColor :
			Color( PlaneNormal.Y * 150, PlaneNormal.X * 150, PlaneNormal.Z * 150 ) );

		const auto DragCenter = Origin + Distance * 0.5f * PlaneInfluence;
		const auto DragWS = Intersection - Distance * 0.5f * PlaneInfluence;
		if( PlaneIntersection && WantsToDrag )
		{
			DragPersistence.Entity = Entity;

			if( DragPersistence.Plane < 0 || DragPersistence.Plane == LocalPlane )
			{
				DragPersistence.Plane = LocalPlane;

				UI::AddAABB( Minimum, Maximum, Color::White );

				const auto DragDistance = Origin.Distance( DragWS );
				if( DragDistance > 0.01f )
				{
					const auto OldPosition = Transform.GetPosition();
					auto* MeshEntity = Cast<CMeshEntity>( Entity );
					if( MeshEntity && MeshEntity->GetBody() )
					{
						auto* Body = MeshEntity->GetBody();
						if( Body->IsKinetic() )
						{
							Body->Velocity = ( DragWS - OldPosition );
						}
						else
						{
							Transform.SetPosition( DragWS );
							Entity->SetTransform( Transform );
						}

						Body->CalculateBounds();
					}
					else
					{
						Transform.SetPosition( DragWS );
						Entity->SetTransform( Transform );
					}
				}
			}

			return true;
		}

		return false;
	};

	return 
		PlaneDragLambda( Vector3D( 1.0f, 0.0f, 0.0f ) ) ||
		PlaneDragLambda( Vector3D( 0.0f, 1.0f, 0.0f ) ) ||
		PlaneDragLambda( Vector3D( 0.0f, 0.0f, 1.0f ) );
}

bool PointGizmo( FTransform* Transform, DragTransform& DragPersistence )
{
	auto& Input = CInputLocator::Get();
	const bool WantsToDrag = Input.IsMouseDown( EMouse::LeftMouseButton );
	if( !WantsToDrag && DragPersistence.Transform )
	{
		DragPersistence.Transform = nullptr;
		DragPersistence.Plane = -1;
	}

	if( !Transform )
	{
		if( DragPersistence.Transform )
		{
			Transform = DragPersistence.Transform;
		}

		return false;
	}

	auto* World = CWorld::GetPrimaryWorld();
	if( !World )
		return false;

	auto* Camera = World->GetActiveCamera();
	if( !Camera )
		return false;

	const auto& CameraSetup = Camera->GetCameraSetup();

	const auto Origin = Transform->GetPosition();
	const auto DistanceToCamera = CameraSetup.CameraPosition.Distance( Origin );
	const auto Distance = DistanceToCamera * 0.05f;
	const auto X = Vector3D( Distance, 0.0f, 0.0f );
	const auto Y = Vector3D( 0.0f, Distance, 0.0f );
	const auto Z = Vector3D( 0.0f, 0.0f, Distance );
	UI::AddLine( Origin, Origin + X, Color::Red );
	UI::AddLine( Origin, Origin + Y, Color::Green );
	UI::AddLine( Origin, Origin + Z, Color::Blue );

	// XY Plane
	const auto MousePositionFixed = Input.GetMousePosition();
	const auto MousePosition = Vector2D( MousePositionFixed.X, MousePositionFixed.Y );
	auto& Window = CWindow::Get();
	auto& Renderer = Window.GetRenderer();
	const auto MousePositionWorldSpace = Renderer.ScreenPositionToWorld( MousePosition );
	const auto PlaneDragLambda = [&] ( const Vector3D& PlaneNormal )
	{
		Vector3D Intersection;
		const auto PlaneInfluence = Vector3D( 1.0f, 1.0f, 1.0f ) - PlaneNormal;
		const auto PlaneColor = Color( PlaneNormal.Y * 255, PlaneNormal.X * 255, PlaneNormal.Z * 255 );
		const auto Minimum = Origin + Vector3D( 0.25f, 0.25f, 0.25f ) * PlaneInfluence * Distance;
		const auto Maximum = Origin + Vector3D( 0.75f, 0.75f, 0.75f ) * PlaneInfluence * Distance;
		const auto PlaneCenter = Origin + ( Maximum - Minimum );
		bool PlaneIntersection = Math::PlaneIntersection( Intersection, CameraSetup.CameraPosition, MousePositionWorldSpace, PlaneCenter, PlaneNormal );
		UI::AddCircle( PlaneCenter, 1.0f, PlaneColor );

		bool OutsideX =
			Intersection.X < Minimum.X ||
			Intersection.X > Maximum.X
			;

		bool OutsideY =
			Intersection.Y < Minimum.Y ||
			Intersection.Y > Maximum.Y
			;

		bool OutsideZ =
			Intersection.Z < Minimum.Z ||
			Intersection.Z > Maximum.Z
			;

		OutsideX = OutsideX && PlaneInfluence.X > 0.5f;
		OutsideY = OutsideY && PlaneInfluence.Y > 0.5f;
		OutsideZ = OutsideZ && PlaneInfluence.Z > 0.5f;

		const bool OutsideBounds = OutsideX || OutsideY || OutsideZ;

		int LocalPlane = -1;
		if( PlaneInfluence.X < 0.5f )
		{
			LocalPlane = 0;
		}
		else if( PlaneInfluence.Y < 0.5f )
		{
			LocalPlane = 1;
		}
		else if( PlaneInfluence.Z < 0.5f )
		{
			LocalPlane = 2;
		}

		if( OutsideBounds && DragPersistence.Plane != LocalPlane )
		{
			PlaneIntersection = false;
		}

		UI::AddAABB( Minimum, Maximum, PlaneIntersection ?
			PlaneColor :
			Color( PlaneNormal.Y * 150, PlaneNormal.X * 150, PlaneNormal.Z * 150 ) );

		const auto DragCenter = Origin + Distance * 0.5f * PlaneInfluence;
		const auto DragWS = Intersection - Distance * 0.5f * PlaneInfluence;
		if( PlaneIntersection && WantsToDrag )
		{
			DragPersistence.Transform = Transform;

			if( DragPersistence.Plane < 0 || DragPersistence.Plane == LocalPlane )
			{
				DragPersistence.Plane = LocalPlane;

				UI::AddAABB( Minimum, Maximum, Color::White );

				const auto DragDistance = Origin.Distance( DragWS );
				if( DragDistance > 0.01f )
				{
					Transform->SetPosition( DragWS );
				}
			}

			return true;
		}

		return false;
	};

	return PlaneDragLambda( Vector3D( 1.0f, 0.0f, 0.0f ) ) ||
		PlaneDragLambda( Vector3D( 0.0f, 1.0f, 0.0f ) ) ||
		PlaneDragLambda( Vector3D( 0.0f, 0.0f, 1.0f ) );
}

bool PointGizmo( Vector3D* Point, DragVector& DragPersistence )
{
	auto& Input = CInputLocator::Get();
	const bool WantsToDrag = Input.IsMouseDown( EMouse::LeftMouseButton );
	if( !WantsToDrag && DragPersistence.Point )
	{
		DragPersistence.Point = nullptr;
		DragPersistence.Plane = -1;
	}

	if( !Point )
	{
		if( DragPersistence.Point )
		{
			Point = DragPersistence.Point;
		}

		return false;
	}

	auto* World = CWorld::GetPrimaryWorld();
	if( !World )
		return false;

	auto* Camera = World->GetActiveCamera();
	if( !Camera )
		return false;

	const auto& CameraSetup = Camera->GetCameraSetup();

	const auto Origin = *Point;
	const auto DistanceToCamera = CameraSetup.CameraPosition.Distance( Origin );
	const auto Distance = DistanceToCamera * 0.05f;
	const auto X = Vector3D( Distance, 0.0f, 0.0f );
	const auto Y = Vector3D( 0.0f, Distance, 0.0f );
	const auto Z = Vector3D( 0.0f, 0.0f, Distance );
	UI::AddLine( Origin, Origin + X, Color::Red );
	UI::AddLine( Origin, Origin + Y, Color::Green );
	UI::AddLine( Origin, Origin + Z, Color::Blue );

	// XY Plane
	const auto MousePositionFixed = Input.GetMousePosition();
	const auto MousePosition = Vector2D( MousePositionFixed.X, MousePositionFixed.Y );
	auto& Window = CWindow::Get();
	auto& Renderer = Window.GetRenderer();
	const auto MousePositionWorldSpace = Renderer.ScreenPositionToWorld( MousePosition );
	const auto PlaneDragLambda = [&] ( const Vector3D& PlaneNormal )
	{
		Vector3D Intersection;
		const auto PlaneInfluence = Vector3D( 1.0f, 1.0f, 1.0f ) - PlaneNormal;
		const auto PlaneColor = Color( PlaneNormal.Y * 255, PlaneNormal.X * 255, PlaneNormal.Z * 255 );
		const auto Minimum = Origin + Vector3D( 0.25f, 0.25f, 0.25f ) * PlaneInfluence * Distance;
		const auto Maximum = Origin + Vector3D( 0.75f, 0.75f, 0.75f ) * PlaneInfluence * Distance;
		const auto PlaneCenter = Origin + ( Maximum - Minimum );
		bool PlaneIntersection = Math::PlaneIntersection( Intersection, CameraSetup.CameraPosition, MousePositionWorldSpace, PlaneCenter, PlaneNormal );
		UI::AddCircle( PlaneCenter, 1.0f, PlaneColor );

		bool OutsideX =
			Intersection.X < Minimum.X ||
			Intersection.X > Maximum.X
			;

		bool OutsideY =
			Intersection.Y < Minimum.Y ||
			Intersection.Y > Maximum.Y
			;

		bool OutsideZ =
			Intersection.Z < Minimum.Z ||
			Intersection.Z > Maximum.Z
			;

		OutsideX = OutsideX && PlaneInfluence.X > 0.5f;
		OutsideY = OutsideY && PlaneInfluence.Y > 0.5f;
		OutsideZ = OutsideZ && PlaneInfluence.Z > 0.5f;

		const bool OutsideBounds = OutsideX || OutsideY || OutsideZ;

		int LocalPlane = -1;
		if( PlaneInfluence.X < 0.5f )
		{
			LocalPlane = 0;
		}
		else if( PlaneInfluence.Y < 0.5f )
		{
			LocalPlane = 1;
		}
		else if( PlaneInfluence.Z < 0.5f )
		{
			LocalPlane = 2;
		}

		if( OutsideBounds && DragPersistence.Plane != LocalPlane )
		{
			PlaneIntersection = false;
		}

		UI::AddAABB( Minimum, Maximum, PlaneIntersection ?
			PlaneColor :
			Color( PlaneNormal.Y * 150, PlaneNormal.X * 150, PlaneNormal.Z * 150 ) );

		const auto DragCenter = Origin + Distance * 0.5f * PlaneInfluence;
		const auto DragWS = Intersection - Distance * 0.5f * PlaneInfluence;
		if( PlaneIntersection && WantsToDrag )
		{
			DragPersistence.Point = Point;

			if( DragPersistence.Plane < 0 || DragPersistence.Plane == LocalPlane )
			{
				DragPersistence.Plane = LocalPlane;

				UI::AddAABB( Minimum, Maximum, Color::White );

				const auto DragDistance = Origin.Distance( DragWS );
				if( DragDistance > 0.01f )
				{
					*Point = DragWS;
				}
			}

			return true;
		}

		return false;
	};

	return
		PlaneDragLambda( Vector3D( 1.0f, 0.0f, 0.0f ) ) ||
		PlaneDragLambda( Vector3D( 0.0f, 1.0f, 0.0f ) ) ||
		PlaneDragLambda( Vector3D( 0.0f, 0.0f, 1.0f ) );
}
