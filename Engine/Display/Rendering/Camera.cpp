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

Corners::Corners( FCameraSetup& Setup )
{
	Vector2D NearOffset;
	Vector2D FarOffset;

	float Scale;
	if( !Setup.Orthographic )
	{
		Scale = tan( glm::radians( Setup.FieldOfView ) * 0.5f );

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

	TopRightNear = NearCenter + Setup.CameraUpVector * NearOffset.Y - Setup.CameraRightVector * NearOffset.X;
	TopRightFar = FarCenter + Setup.CameraUpVector * FarOffset.Y - Setup.CameraRightVector * FarOffset.X;

	BottomLeftNear = NearCenter - Setup.CameraUpVector * NearOffset.Y + Setup.CameraRightVector * NearOffset.X;
	BottomLeftFar = FarCenter - Setup.CameraUpVector * FarOffset.Y + Setup.CameraRightVector * FarOffset.X;

	TopLeftNear = NearCenter + Setup.CameraUpVector * NearOffset.Y + Setup.CameraRightVector * NearOffset.X;
	TopLeftFar = FarCenter + Setup.CameraUpVector * FarOffset.Y + Setup.CameraRightVector * FarOffset.X;

	BottomRightNear = NearCenter - Setup.CameraUpVector * NearOffset.Y - Setup.CameraRightVector * NearOffset.X;
	BottomRightFar = FarCenter - Setup.CameraUpVector * FarOffset.Y - Setup.CameraRightVector * FarOffset.X;
}

CCamera::CCamera()
{
	CameraSetup = FCameraSetup();
	CameraOrientation = { 0.0f, 0.0f, 0.0f };
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

	glm::vec3 CameraPosition = Math::ToGLM( CameraSetup.CameraPosition );

	CameraSetup.CameraRightVector = WorldUp.Cross( CameraSetup.CameraDirection ).Normalized();
	CameraSetup.CameraUpVector = CameraSetup.CameraDirection.Cross( CameraSetup.CameraRightVector ).Normalized() - glm::radians( CameraOrientation[2] ) * CameraSetup.CameraRightVector;

	ViewMatrix = glm::lookAt(
		CameraPosition,
		CameraPosition + Math::ToGLM( CameraSetup.CameraDirection ),
		Math::ToGLM( CameraSetup.CameraUpVector )
	);

	ProjectionViewInverseMatrix = glm::inverse( ProjectionMatrix * ViewMatrix );

	// Calculate the frustum's corners.
	Corners = ::Corners( CameraSetup );
	
}

void CCamera::DrawFrustum() const
{
	const auto Corners = GetCorners();

	UI::AddLine( Corners.TopRightNear, Corners.TopRightFar, Color::Red );
	UI::AddLine( Corners.BottomLeftNear, Corners.BottomLeftFar, Color::Green );
	UI::AddLine( Corners.TopLeftNear, Corners.TopLeftFar, Color::Blue );
	UI::AddLine( Corners.BottomRightNear, Corners.BottomRightFar, Color( 255, 255, 0 ) );

	const Color FrustumBorderColor = Color( 128, 128, 128 );
	UI::AddLine( Corners.TopLeftNear, Corners.TopRightNear, FrustumBorderColor );
	UI::AddLine( Corners.BottomLeftNear, Corners.BottomRightNear, FrustumBorderColor );

	UI::AddLine( Corners.TopLeftNear, Corners.BottomLeftNear, FrustumBorderColor );
	UI::AddLine( Corners.TopRightNear, Corners.BottomRightNear, FrustumBorderColor );

	UI::AddLine( Corners.TopLeftFar, Corners.TopRightFar, FrustumBorderColor );
	UI::AddLine( Corners.BottomLeftFar, Corners.BottomRightFar, FrustumBorderColor );

	UI::AddLine( Corners.TopLeftFar, Corners.BottomLeftFar, FrustumBorderColor );
	UI::AddLine( Corners.TopRightFar, Corners.BottomRightFar, FrustumBorderColor );

	UI::AddLine( CameraSetup.CameraPosition, CameraSetup.CameraPosition + CameraSetup.CameraDirection * CameraSetup.NearPlaneDistance, Color::White );
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

Corners CCamera::GetCorners() const
{
	return Corners;
}
