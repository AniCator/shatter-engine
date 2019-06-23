// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

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
	ProjectionMatrix = glm::perspective( glm::radians( CameraSetup.FieldOfView ), CameraSetup.AspectRatio, CameraSetup.NearPlaneDistance, CameraSetup.FarPlaneDistance );

	glm::vec3 CameraPosition = Vector3DToInitializerList( CameraSetup.CameraPosition );

	CameraSetup.CameraRightVector = WorldUp.Cross( CameraSetup.CameraDirection ).Normalized();
	CameraSetup.CameraUpVector = CameraSetup.CameraDirection.Cross( CameraSetup.CameraRightVector ).Normalized();

	ViewMatrix = glm::lookAt(
		CameraPosition,
		CameraPosition + Math::ToGLM( CameraSetup.CameraDirection ),
		Vector3DToInitializerList( CameraSetup.CameraUpVector )
	);

	ProjectionViewInverseMatrix = glm::inverse( ProjectionMatrix * ViewMatrix );
}

void CCamera::SetFieldOfView( const float& FieldOfView )
{
	CameraSetup.FieldOfView = FieldOfView;

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
