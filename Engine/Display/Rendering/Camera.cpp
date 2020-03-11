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

	glm::vec3 CameraPosition = Vector3DToInitializerList( CameraSetup.CameraPosition );

	CameraSetup.CameraRightVector = WorldUp.Cross( CameraSetup.CameraDirection ).Normalized();
	CameraSetup.CameraUpVector = CameraSetup.CameraDirection.Cross( CameraSetup.CameraRightVector ).Normalized() - glm::radians( CameraOrientation[2] ) * CameraSetup.CameraRightVector;

	ViewMatrix = glm::lookAt(
		CameraPosition,
		CameraPosition + Math::ToGLM( CameraSetup.CameraDirection ),
		Vector3DToInitializerList( CameraSetup.CameraUpVector )
	);

	ProjectionViewInverseMatrix = glm::inverse( ProjectionMatrix * ViewMatrix );
}

void CCamera::DrawFrustum()
{
	Vector2D NearOffset;
	Vector2D FarOffset;

	float Scale;
	if( !CameraSetup.Orthographic )
	{
		Scale = tan( glm::radians( CameraSetup.FieldOfView ) * 0.5f );

		NearOffset.Y = Scale * CameraSetup.NearPlaneDistance;
		NearOffset.X = NearOffset.Y * CameraSetup.AspectRatio;

		FarOffset.Y = Scale * CameraSetup.FarPlaneDistance;
		FarOffset.X = FarOffset.Y * CameraSetup.AspectRatio;
	}
	else
	{
		Scale = CameraSetup.OrthographicScale * 0.5f;

		NearOffset.Y = Scale * CameraSetup.NearPlaneDistance;
		NearOffset.X = NearOffset.Y;

		FarOffset.Y = Scale * CameraSetup.FarPlaneDistance;
		FarOffset.X = FarOffset.Y;
	}

	Vector3D NearCenter = CameraSetup.CameraPosition + CameraSetup.CameraDirection * CameraSetup.NearPlaneDistance;
	Vector3D FarCenter = CameraSetup.CameraPosition + CameraSetup.CameraDirection * CameraSetup.FarPlaneDistance;

	Vector3D CornerTopRightNear = NearCenter + CameraSetup.CameraUpVector * NearOffset.Y - CameraSetup.CameraRightVector * NearOffset.X;
	Vector3D CornerTopRightFar = FarCenter + CameraSetup.CameraUpVector * FarOffset.Y - CameraSetup.CameraRightVector * FarOffset.X;
	UI::AddLine( CornerTopRightNear, CornerTopRightFar, Color::Red );

	Vector3D CornerBottomLeftNear = NearCenter - CameraSetup.CameraUpVector * NearOffset.Y + CameraSetup.CameraRightVector * NearOffset.X;
	Vector3D CornerBottomLeftFar = FarCenter - CameraSetup.CameraUpVector * FarOffset.Y + CameraSetup.CameraRightVector * FarOffset.X;
	UI::AddLine( CornerBottomLeftNear, CornerBottomLeftFar, Color::Green );

	Vector3D CornerTopLeftNear = NearCenter + CameraSetup.CameraUpVector * NearOffset.Y + CameraSetup.CameraRightVector * NearOffset.X;
	Vector3D CornerTopLeftFar = FarCenter + CameraSetup.CameraUpVector * FarOffset.Y + CameraSetup.CameraRightVector * FarOffset.X;
	UI::AddLine( CornerTopLeftNear, CornerTopLeftFar, Color::Blue );

	Vector3D CornerBottomRightNear = NearCenter - CameraSetup.CameraUpVector * NearOffset.Y - CameraSetup.CameraRightVector * NearOffset.X;
	Vector3D CornerBottomRightFar = FarCenter - CameraSetup.CameraUpVector * FarOffset.Y - CameraSetup.CameraRightVector * FarOffset.X;
	UI::AddLine( CornerBottomRightNear, CornerBottomRightFar, Color( 255, 255, 0 ) );

	const Color FrustumBorderColor = Color( 128, 128, 128 );
	UI::AddLine( CornerTopLeftNear, CornerTopRightNear, FrustumBorderColor );
	UI::AddLine( CornerBottomLeftNear, CornerBottomRightNear, FrustumBorderColor );

	UI::AddLine( CornerTopLeftNear, CornerBottomLeftNear, FrustumBorderColor );
	UI::AddLine( CornerTopRightNear, CornerBottomRightNear, FrustumBorderColor );

	UI::AddLine( CornerTopLeftFar, CornerTopRightFar, FrustumBorderColor );
	UI::AddLine( CornerBottomLeftFar, CornerBottomRightFar, FrustumBorderColor );

	UI::AddLine( CornerTopLeftFar, CornerBottomLeftFar, FrustumBorderColor );
	UI::AddLine( CornerTopRightFar, CornerBottomRightFar, FrustumBorderColor );

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
	this->CameraQuaternion = glm::quat( Vector3DToInitializerList( CameraOrientation ) );

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
