// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <Engine/Display/UserInterface.h>
#include <Engine/Utility/Data.h>

CData& operator<<( CData& Data, CCamera& Camera )
{
	Data << Camera.GetCameraSetup();
	return Data;
}

CData& operator>>( CData& Data, CCamera& Camera )
{
	Data >> Camera.GetCameraSetup();
	return Data;
}

FCameraSetup FCameraSetup::Mix( const FCameraSetup& A, const FCameraSetup& B, const float& Alpha )
{
	return A;
}

Plane::Plane()
{
	Point = Vector3D::Zero;
	Normal = Vector3D( 0.0f, 0.0f, 1.0f );
	Distance = 1.0f;
}

Plane::Plane( const Vector3D& Point, const Vector3D& Normal )
{
	this->Point = Point;
	this->Normal = Normal;
	Distance = Normal.Length();
}

Plane::Plane( const Vector3D& A, const Vector3D& B, const Vector3D& C )
{
	Point = B - A;

	const Vector3D Edge = C - A;
	Normal = Point.Cross( Edge ).Normalized();
	Distance = -Normal.Dot( A );
	Point = A;
}

float HalfSpace( const Plane& Plane, const Vector3D& Point )
{
	return Plane.Normal.X * Point.X + Plane.Normal.Y * Point.Y + Plane.Normal.Z * Point.Z + Plane.Distance;
}

bool HalfSpaceTest( const Plane& Plane, const Vector3D& Point, const float& Radius = 0.0f )
{
	if( HalfSpace( Plane, Point ) >= Radius )
	{
		return false;
	}

	return true;
}

float DistanceToPlane( const Plane& Plane, const Vector3D& Point )
{
	return Plane.Point.Dot( Point );
}

bool PlaneTest( const Plane& Plane, const Vector3D& Point )
{
	const auto Difference = ( Point - Plane.Point );
	return Difference.Dot( Plane.Normal ) > 0.0f;
}

bool SphereTest( const Plane& Plane, const Vector3D& Point, const float& Radius )
{
	// const auto Difference = ( Point - Plane.Point );
	// return Point.Dot( Plane.Normal ) + Plane.Distance + Radius * Radius >= 0.0f;
	return HalfSpaceTest( Plane, Point, Radius );
}

bool Frustum::Contains( const Vector3D& Point, const float Radius ) const
{
	// return HalfSpaceTest( Plane[Near], Point, Radius );
	
	if( !SphereTest( Plane[Near], Point, Radius ) )
		return false;

	if( !SphereTest( Plane[Far], Point, Radius ) )
		return false;
	
	if( !SphereTest( Plane[Top], Point, Radius ) )
		return false;

	if( !SphereTest( Plane[Bottom], Point, Radius ) )
		return false;

	if( !SphereTest( Plane[Left], Point, Radius ) )
		return false;

	if( !SphereTest( Plane[Right], Point, Radius ) )
		return false;

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
	Plane[Top] = ::Plane(TopLeftNear, TopRightFar, TopLeftFar );
	Plane[Bottom] = ::Plane(BottomLeftNear, BottomRightFar, BottomRightNear );

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
		ProjectionMatrix = glm::ortho( -CameraSetup.OrthographicScale, CameraSetup.OrthographicScale, -CameraSetup.OrthographicScale, CameraSetup.OrthographicScale, CameraSetup.NearPlaneDistance, CameraSetup.FarPlaneDistance );
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
	// UI::AddLine( Frustum.Plane[Frustum::Top].Point, Frustum.Plane[Frustum::Top].Point + Frustum.Plane[Frustum::Top].Normal, FrustumPlaneColor );
	// UI::AddLine( Frustum.Plane[Frustum::Bottom].Point, Frustum.Plane[Frustum::Bottom].Point + Frustum.Plane[Frustum::Bottom].Normal, FrustumPlaneColor );
	// 
	// UI::AddLine( Frustum.Plane[Frustum::Left].Point, Frustum.Plane[Frustum::Left].Point + Frustum.Plane[Frustum::Left].Normal, FrustumPlaneColor );
	// UI::AddLine( Frustum.Plane[Frustum::Right].Point, Frustum.Plane[Frustum::Right].Point + Frustum.Plane[Frustum::Right].Normal, FrustumPlaneColor );
	// 
	// UI::AddLine( Frustum.Plane[Frustum::Near].Point, Frustum.Plane[Frustum::Near].Point + Frustum.Plane[Frustum::Near].Normal, FrustumClipPlaneColor );
	// UI::AddLine( Frustum.Plane[Frustum::Far].Point, Frustum.Plane[Frustum::Far].Point + Frustum.Plane[Frustum::Far].Normal, FrustumClipPlaneColor );

	UI::AddCircle( Frustum.Plane[Frustum::Top].Point, 2.0f, Color::White );
	UI::AddCircle( Frustum.Plane[Frustum::Bottom].Point, 2.0f, Color::White );
	
	UI::AddCircle( Frustum.Plane[Frustum::Left].Point, 2.0f, Color::White );
	UI::AddCircle( Frustum.Plane[Frustum::Right].Point, 2.0f, Color::White );
	
	UI::AddCircle( Frustum.Plane[Frustum::Near].Point, 2.0f, Color::White );
	UI::AddCircle( Frustum.Plane[Frustum::Far].Point, 2.0f, Color::White );
}

void CCamera::SetFieldOfView( const float& FieldOfView )
{
	CameraSetup.Orthographic = false;
	CameraSetup.FieldOfView = FieldOfView;

	Update();
}

void CCamera::SetOrthographicScale( const float& OrthographicScale )
{
	CameraSetup.Orthographic = true;
	CameraSetup.OrthographicScale = OrthographicScale;

	Update();
}

void CCamera::SetAspectRatio( const float& AspectRatio )
{
	CameraSetup.AspectRatio = AspectRatio;

	Update();
}

void CCamera::SetNearPlaneDistance( const float& NearPlaneDistance )
{
	CameraSetup.NearPlaneDistance = NearPlaneDistance;

	Update();
}

void CCamera::SetFarPlaneDistance( const float& FarPlaneDistance )
{
	CameraSetup.FarPlaneDistance = FarPlaneDistance;

	Update();
}

void CCamera::SetCameraPosition( const Vector3D& CameraPosition )
{
	CameraSetup.CameraPosition = CameraPosition;

	Update();
}

void CCamera::SetCameraDirection( const Vector3D& CameraDirection )
{
	CameraSetup.CameraDirection = CameraDirection;

	Update();
}

void CCamera::SetCameraOrientation( const Vector3D& CameraOrientation )
{
	this->CameraOrientation = CameraOrientation;
	this->CameraQuaternion = glm::quat( Math::ToGLM( CameraOrientation ) );

	CameraSetup.CameraDirection[1] = cos( glm::radians( CameraOrientation[0] ) ) * cos( glm::radians( CameraOrientation[1] ) );
	CameraSetup.CameraDirection[2] = sin( glm::radians( CameraOrientation[0] ) );
	CameraSetup.CameraDirection[0] = cos( glm::radians( CameraOrientation[0] ) ) * sin( glm::radians( CameraOrientation[1] ) );
	CameraSetup.CameraDirection.Normalize();

	Update();
}

void CCamera::SetCameraUpVector( const Vector3D& CameraUpVector )
{
	CameraSetup.CameraUpVector = CameraUpVector;

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

CCamera CCamera::Lerp( const CCamera& B, const float& Alpha ) const
{
	CCamera Blended = *this;

	FCameraSetup& BlendSetup = Blended.GetCameraSetup();
	const auto& SetupA = GetCameraSetup();
	const auto& SetupB = B.GetCameraSetup();

	BlendSetup.CameraPosition = Math::Lerp( SetupA.CameraPosition, SetupB.CameraPosition, Alpha );
	BlendSetup.FieldOfView = Math::Lerp( SetupA.FieldOfView, SetupB.FieldOfView, Alpha );
	Blended.SetCameraOrientation( Math::Lerp( CameraOrientation, B.CameraOrientation, Alpha ) );
	BlendSetup.CameraDirection = Math::Lerp( SetupA.CameraDirection, SetupB.CameraDirection, Alpha );

	Blended.Update();
	return Blended;
}
