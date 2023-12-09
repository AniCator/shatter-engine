// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <Engine/Display/UserInterface.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/Serialize.h>

CData& operator<<( CData& Data, const CCamera& Camera )
{
	Data << Camera.CameraOrientation;
	Data << Camera.CameraSetup;
	Data << Camera.ProjectionMatrix;
	Data << Camera.ViewMatrix;
	Data << Camera.ProjectionViewInverseMatrix;
	Data << Camera.CameraQuaternion;
	Data << Camera.Frustum;
	return Data;
}

CData& operator>>( CData& Data, CCamera& Camera )
{
	Data >> Camera.CameraOrientation;
	Data >> Camera.CameraSetup;
	Data >> Camera.ProjectionMatrix;
	Data >> Camera.ViewMatrix;
	Data >> Camera.ProjectionViewInverseMatrix;
	Data >> Camera.CameraQuaternion;
	Data >> Camera.Frustum;
	return Data;
}

FCameraSetup FCameraSetup::Mix( const FCameraSetup& A, const FCameraSetup& B, const float& Alpha )
{
	return A;
}

float HalfSpace( const Plane& Plane, const Vector3D& Point )
{
	return Plane.Normal.Dot( Point ) + Plane.Distance;
}

bool HalfSpaceTest( const Plane& Plane, const Vector3D& Point, const float& Radius = 0.0f )
{
	if( HalfSpace( Plane, Point ) >= Radius )
	{
		return true;
	}

	return false;
}

bool PointTest( const Plane& Plane, const Vector3D& Point )
{
	return Point.Dot( Plane.Normal ) + Plane.Distance > 0.0f;
}

bool Frustum::Contains( const Vector3D& Point ) const
{
	for( int Side = 0; Side < Frustum::Maximum; Side++ )
	{
		if( PointTest( Plane[Side], Point ) )
			return false;
	}

	return true;
}

bool SphereTest( const Plane& Plane, const Vector3D& Point, const float Radius )
{
	return Point.Dot( Plane.Normal ) + Plane.Distance > Radius;
}

bool Frustum::Contains( const Vector3D& Point, const float Radius ) const
{
	for( int Side = 0; Side < Frustum::Maximum; Side++ )
	{
		if( SphereTest( Plane[Side], Point, Radius ) )
			return false;
	}

	return true;
}

bool Frustum::Contains( const BoundingSphere& Sphere ) const
{
	for( int Side = 0; Side < Frustum::Maximum; Side++ )
	{
		if( SphereTest( Plane[Side], Sphere.Origin(), Sphere.GetRadius() ) )
			return false;
	}

	return true;
}

float DistanceToBox( const Plane& Plane, const BoundingBox& Box )
{
	const float Radius = Box.Size().Dot( Math::Abs( Plane.Normal ) );
	const float Distance = Plane.Normal.Dot( Box.Center() ) + Plane.Distance;
	if( Math::Abs( Distance ) < Radius )
	{
		return 0.0f; // Intersecting with plane.
	}
	else if( Distance < 0.0f )
	{
		return Distance + Radius;
	}

	return Distance - Radius;
}

bool BoxTest( const Plane& Plane, const BoundingBox& Box )
{
	return DistanceToBox( Plane, Box ) > 0.0f;
}

bool Frustum::Contains( const BoundingBox& Box ) const
{
	for( int Side = 0; Side < Frustum::Maximum; Side++ )
	{
		if( BoxTest( Plane[Side], Box ) )
			return false;
	}

	return true;
}

Frustum::Frustum( FCameraSetup& Setup )
{
	Vector2D NearOffset;
	Vector2D FarOffset;

	float Scale;
	if( !Setup.Orthographic )
	{
		Scale = tan( glm::radians( Setup.FieldOfView * 0.5f ) );

		NearOffset.Y = Scale * Setup.NearPlaneDistance;
		NearOffset.X = NearOffset.Y * Setup.AspectRatio;

		FarOffset.Y = Scale * Setup.FarPlaneDistance;
		FarOffset.X = FarOffset.Y * Setup.AspectRatio;
	}
	else
	{
		Scale = Setup.OrthographicScale * 0.5f;

		NearOffset.Y = Scale * Setup.NearPlaneDistance;
		NearOffset.X = NearOffset.Y;

		FarOffset.Y = Scale * Setup.FarPlaneDistance;
		FarOffset.X = FarOffset.Y;
	}

	const Vector3D NearCenter = Setup.CameraPosition + Setup.CameraDirection * Setup.NearPlaneDistance;
	const Vector3D FarCenter = Setup.CameraPosition + Setup.CameraDirection * Setup.FarPlaneDistance;

	// Calculate the Frustum.
	TopRightNear = NearCenter + Setup.CameraUpVector * NearOffset.Y - Setup.CameraRightVector * NearOffset.X;
	TopRightFar = FarCenter + Setup.CameraUpVector * FarOffset.Y - Setup.CameraRightVector * FarOffset.X;

	BottomLeftNear = NearCenter - Setup.CameraUpVector * NearOffset.Y + Setup.CameraRightVector * NearOffset.X;
	BottomLeftFar = FarCenter - Setup.CameraUpVector * FarOffset.Y + Setup.CameraRightVector * FarOffset.X;

	TopLeftNear = NearCenter + Setup.CameraUpVector * NearOffset.Y + Setup.CameraRightVector * NearOffset.X;
	TopLeftFar = FarCenter + Setup.CameraUpVector * FarOffset.Y + Setup.CameraRightVector * FarOffset.X;

	BottomRightNear = NearCenter - Setup.CameraUpVector * NearOffset.Y - Setup.CameraRightVector * NearOffset.X;
	BottomRightFar = FarCenter - Setup.CameraUpVector * FarOffset.Y - Setup.CameraRightVector * FarOffset.X;

	// Calculate the plane vectors.
	Plane[Top] = ::Plane( TopLeftNear, TopRightFar, TopLeftFar );
	Plane[Bottom] = ::Plane( BottomLeftNear, BottomRightFar, BottomRightNear );

	Plane[Left] = ::Plane( BottomLeftNear, TopLeftFar, BottomLeftFar );
	Plane[Right] = ::Plane( BottomRightNear, TopRightFar, TopRightNear );

	Plane[Near] = ::Plane( BottomLeftNear, TopRightNear, TopLeftNear );
	Plane[Far] = ::Plane( TopLeftFar, TopRightFar, BottomLeftFar );
}

CCamera::CCamera()
{
	CameraSetup = FCameraSetup();
	CameraOrientation = { 0.0f, 0.0f, 0.0f };

	Frustum = ::Frustum( CameraSetup );
}

CCamera::~CCamera()
{

}

void CCamera::Update()
{
	if( !CameraSetup.Orthographic )
	{
		ProjectionMatrix = glm::perspective( glm::radians( CameraSetup.FieldOfView ), CameraSetup.AspectRatio, CameraSetup.NearPlaneDistance, CameraSetup.FarPlaneDistance );
	}
	else
	{
		ProjectionMatrix = glm::ortho( -CameraSetup.OrthographicScale * CameraSetup.AspectRatio, CameraSetup.OrthographicScale * CameraSetup.AspectRatio, -CameraSetup.OrthographicScale, CameraSetup.OrthographicScale, CameraSetup.NearPlaneDistance, CameraSetup.FarPlaneDistance );
	}

	const glm::vec3 CameraPosition = Math::ToGLM( CameraSetup.CameraPosition );

	CameraSetup.CameraRightVector = WorldUp.Cross( CameraSetup.CameraDirection ).Normalized();
	CameraSetup.CameraUpVector = CameraSetup.CameraDirection.Cross( CameraSetup.CameraRightVector ).Normalized() - glm::radians( CameraOrientation[2] ) * CameraSetup.CameraRightVector;

	ViewMatrix = glm::lookAt(
		glm::vec3( 0.0f ),
		Math::ToGLM( CameraSetup.CameraDirection ),
		Math::ToGLM( CameraSetup.CameraUpVector )
	);

	// Translate afterwards because it could cause precision issues if we do it in lookAt.
	ViewMatrix = glm::translate( ViewMatrix, -CameraPosition );

	ProjectionViewInverseMatrix = glm::inverse( ProjectionMatrix * ViewMatrix );

	if( Freeze )
	{
		DrawFrustum(); // Always draw the frustum when frozen.
		return;
	}

	Frustum = ::Frustum( CameraSetup );
	
}

void CCamera::DrawFrustum() const
{
	UI::AddLine( Frustum.TopRightNear, Frustum.TopRightFar, Color::Red );
	UI::AddLine( Frustum.BottomLeftNear, Frustum.BottomLeftFar, Color::Green );
	UI::AddLine( Frustum.TopLeftNear, Frustum.TopLeftFar, Color::Blue );
	UI::AddLine( Frustum.BottomRightNear, Frustum.BottomRightFar, Color( 255, 255, 0 ) );

	const Color FrustumBorderColor = Color( 128, 128, 128 );
	UI::AddLine( Frustum.TopLeftNear, Frustum.TopRightNear, FrustumBorderColor );
	UI::AddLine( Frustum.BottomLeftNear, Frustum.BottomRightNear, FrustumBorderColor );

	UI::AddLine( Frustum.TopLeftNear, Frustum.BottomLeftNear, FrustumBorderColor );
	UI::AddLine( Frustum.TopRightNear, Frustum.BottomRightNear, FrustumBorderColor );

	UI::AddLine( Frustum.TopLeftFar, Frustum.TopRightFar, FrustumBorderColor );
	UI::AddLine( Frustum.BottomLeftFar, Frustum.BottomRightFar, FrustumBorderColor );

	UI::AddLine( Frustum.TopLeftFar, Frustum.BottomLeftFar, FrustumBorderColor );
	UI::AddLine( Frustum.TopRightFar, Frustum.BottomRightFar, FrustumBorderColor );

	UI::AddLine( CameraSetup.CameraPosition, CameraSetup.CameraPosition + CameraSetup.CameraDirection * CameraSetup.NearPlaneDistance, Color::White );

	const Color FrustumPlaneColor = Color( 255, 0, 255 );
	const Color FrustumClipPlaneColor = Color( 0, 255, 255 );
	// UI::AddLine( Frustum.Plane[Frustum::Top].Origin, Frustum.Plane[Frustum::Top].Origin + Frustum.Plane[Frustum::Top].Normal, FrustumPlaneColor );
	// UI::AddLine( Frustum.Plane[Frustum::Bottom].Origin, Frustum.Plane[Frustum::Bottom].Origin + Frustum.Plane[Frustum::Bottom].Normal, FrustumPlaneColor );
	// 
	// UI::AddLine( Frustum.Plane[Frustum::Left].Origin, Frustum.Plane[Frustum::Left].Origin + Frustum.Plane[Frustum::Left].Normal, FrustumPlaneColor );
	// UI::AddLine( Frustum.Plane[Frustum::Right].Origin, Frustum.Plane[Frustum::Right].Origin + Frustum.Plane[Frustum::Right].Normal, FrustumPlaneColor );
	// 
	// UI::AddLine( Frustum.Plane[Frustum::Near].Origin, Frustum.Plane[Frustum::Near].Origin + Frustum.Plane[Frustum::Near].Normal, FrustumClipPlaneColor );
	// UI::AddLine( Frustum.Plane[Frustum::Far].Origin, Frustum.Plane[Frustum::Far].Origin + Frustum.Plane[Frustum::Far].Normal, FrustumClipPlaneColor );

	UI::AddCircle( Frustum.Plane[Frustum::Top].Origin, 2.0f, Color::White );
	UI::AddCircle( Frustum.Plane[Frustum::Bottom].Origin, 2.0f, Color::White );
	
	UI::AddCircle( Frustum.Plane[Frustum::Left].Origin, 2.0f, Color::White );
	UI::AddCircle( Frustum.Plane[Frustum::Right].Origin, 2.0f, Color::White );
	
	UI::AddCircle( Frustum.Plane[Frustum::Near].Origin, 2.0f, Color::White );
	UI::AddCircle( Frustum.Plane[Frustum::Far].Origin, 2.0f, Color::White );
}

void CCamera::SetFieldOfView( const float& FieldOfView )
{
	CameraSetup.Orthographic = false;
	CameraSetup.FieldOfView = FieldOfView;

	Update();
}

void CCamera::SetOrthographicScale( const float& Scale )
{
	CameraSetup.Orthographic = true;
	CameraSetup.OrthographicScale = Scale;

	Update();
}

void CCamera::SetAspectRatio( const float& Ratio )
{
	CameraSetup.AspectRatio = Ratio;

	Update();
}

void CCamera::SetNearPlaneDistance( const float& Distance )
{
	CameraSetup.NearPlaneDistance = Distance;

	Update();
}

void CCamera::SetFarPlaneDistance( const float& Distance )
{
	CameraSetup.FarPlaneDistance = Distance;

	Update();
}

void CCamera::SetCameraPosition( const Vector3D& Position )
{
	CameraSetup.CameraPosition = Position;

	Update();
}

void CCamera::SetCameraDirection( const Vector3D& Direction )
{
	CameraSetup.CameraDirection = Direction;

	Update();
}

void CCamera::SetCameraOrientation( const Vector3D& Orientation )
{
	this->CameraOrientation = Orientation;
	this->CameraQuaternion = glm::quat( Math::ToGLM( Orientation ) );

	CameraSetup.CameraDirection[1] = cos( glm::radians( Orientation[0] ) ) * cos( glm::radians( Orientation[1] ) );
	CameraSetup.CameraDirection[2] = sin( glm::radians( Orientation[0] ) );
	CameraSetup.CameraDirection[0] = cos( glm::radians( Orientation[0] ) ) * sin( glm::radians( Orientation[1] ) );
	CameraSetup.CameraDirection.Normalize();

	Update();
}

void CCamera::SetCameraUpVector( const Vector3D& Vector )
{
	CameraSetup.CameraUpVector = Vector;

	Update();
}

const Vector3D& CCamera::GetCameraPosition() const
{
	return CameraSetup.CameraPosition;
}

const glm::mat4& CCamera::GetProjectionMatrix() const
{
	return ProjectionMatrix;
}

const glm::mat4& CCamera::GetViewMatrix() const
{
	return ViewMatrix;
}

const glm::mat4& CCamera::GetViewProjectionInverse() const
{
	return ProjectionViewInverseMatrix;
}

FCameraSetup& CCamera::GetCameraSetup()
{
	return CameraSetup;
}

FCameraSetup CCamera::GetCameraSetup() const
{
	return CameraSetup;
}

Frustum CCamera::GetFrustum() const
{
	return Frustum;
}

CCamera CCamera::Lerp( const CCamera& A, const CCamera& B, const float& Alpha )
{
	CCamera Blended = A;

	FCameraSetup& BlendSetup = Blended.GetCameraSetup();
	const auto& SetupA = A.GetCameraSetup();
	const auto& SetupB = B.GetCameraSetup();

	BlendSetup.CameraPosition = Math::Lerp( SetupA.CameraPosition, SetupB.CameraPosition, Alpha );
	BlendSetup.FieldOfView = Math::Lerp( SetupA.FieldOfView, SetupB.FieldOfView, Alpha );
	Blended.SetCameraOrientation( Math::Lerp( A.CameraOrientation, B.CameraOrientation, Alpha ) );
	BlendSetup.CameraDirection = Math::Lerp( SetupA.CameraDirection, SetupB.CameraDirection, Alpha );

	Blended.Update();
	return Blended;
}

CCamera CreateControlPoint( const CCamera& Setup, const float& Tangent )
{
	CCamera ControlPoint = Setup;

	auto& ControlPointSetup = ControlPoint.GetCameraSetup();
	ControlPointSetup.CameraPosition += ControlPointSetup.CameraDirection * Tangent;

	return ControlPoint;
}

CCamera CCamera::BezierBlend( const CCamera& A, const float& TangentA, const CCamera& B, const float& TangentB, const float& Factor )
{
	const auto ControlPointA = CreateControlPoint( A, TangentA );
	const auto ControlPointB = CreateControlPoint( B, TangentB );

	// De Casteljau blending.
	const auto BlendA = CCamera::Lerp( A, ControlPointA, Factor );
	const auto BlendB = CCamera::Lerp( ControlPointA, ControlPointB, Factor );
	const auto BlendC = CCamera::Lerp( ControlPointB, B, Factor );

	const auto BlendD = CCamera::Lerp( BlendA, BlendB, Factor );
	const auto BlendE = CCamera::Lerp( BlendB, BlendC, Factor );

	return CCamera::Lerp( BlendD, BlendE, Factor );
}

CCamera CCamera::HandheldSimulation( const CCamera& Camera, const float& Factor, const double& Time )
{
	CCamera HandheldCamera = Camera;

	HandheldCamera.CameraOrientation.Pitch += sinf( Time * 3.0 ) * 0.314f * Factor;
	HandheldCamera.CameraOrientation.Yaw += cosf( Time * 2.0 ) * 0.314f * Factor;

	HandheldCamera.CameraOrientation.Pitch -= cosf( Time * 6.0 ) * 0.0314f * Factor;
	HandheldCamera.CameraOrientation.Yaw -= sinf( Time * 7.0 ) * 0.0314f * Factor;

	HandheldCamera.CameraOrientation.Pitch += sinf( Time * 12.0 ) * 0.0314f * Factor;
	HandheldCamera.CameraOrientation.Yaw += cosf( Time * 14.0 ) * 0.0314f * Factor;

	HandheldCamera.CameraOrientation.Pitch -= cosf( Time * 24.0 ) * 0.00314f * Factor;
	HandheldCamera.CameraOrientation.Yaw -= sinf( Time * 28.0 ) * 0.00314f * Factor;

	HandheldCamera.CameraOrientation.Pitch -= sinf( Time * 96.0 ) * 0.000314f * Factor;
	HandheldCamera.CameraOrientation.Yaw -= cosf( Time * 112.0 ) * 0.000314f * Factor;

	HandheldCamera.SetCameraOrientation( HandheldCamera.CameraOrientation );

	return HandheldCamera;
}
