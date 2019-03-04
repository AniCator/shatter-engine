// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

CCamera::CCamera()
{
	CameraOrientation = glm::vec3( 0.0f, 0.0f, 0.0f );
}

CCamera::~CCamera()
{

}

void CCamera::Update()
{
	ProjectionMatrix = glm::perspective( glm::radians( CameraSetup.FieldOfView ), CameraSetup.AspectRatio, CameraSetup.NearPlaneDistance, CameraSetup.FarPlaneDistance );

	glm::vec3 CameraPosition = CameraSetup.CameraPosition;

	CameraSetup.CameraRightVector = glm::normalize( glm::cross( WorldUp, CameraSetup.CameraDirection ) );
	CameraSetup.CameraUpVector = glm::normalize( glm::cross( CameraSetup.CameraDirection, CameraSetup.CameraRightVector ) );

	ViewMatrix = glm::lookAt(
		CameraPosition,
		CameraPosition + CameraSetup.CameraDirection,
		CameraSetup.CameraUpVector
	);

	ProjectionViewInverseMatrix = glm::inverse( ProjectionMatrix * ViewMatrix );
}

void CCamera::SetFieldOfView( float& FieldOfView )
{
	CameraSetup.FieldOfView = FieldOfView;

	Update();
}

void CCamera::SetAspectRatio( float& AspectRatio )
{
	CameraSetup.AspectRatio = AspectRatio;

	Update();
}

void CCamera::SetNearPlaneDistance( float& NearPlaneDistance )
{
	CameraSetup.NearPlaneDistance = NearPlaneDistance;

	Update();
}

void CCamera::SetFarPlaneDistance( float& FarPlaneDistance )
{
	CameraSetup.FarPlaneDistance = FarPlaneDistance;

	Update();
}

void CCamera::SetCameraPosition( const glm::vec3& CameraPosition )
{
	CameraSetup.CameraPosition = CameraPosition;

	Update();
}

void CCamera::SetCameraDirection( const glm::vec3& CameraDirection )
{
	CameraSetup.CameraDirection = CameraDirection;

	Update();
}

void CCamera::SetCameraOrientation( const glm::vec3& CameraOrientation )
{
	this->CameraOrientation = CameraOrientation;
	this->CameraQuaternion = glm::quat( CameraOrientation );

	CameraSetup.CameraDirection = glm::rotate( CameraQuaternion, glm::vec4( this->CameraOrientation, 1.0f ) );

	Update();
}

void CCamera::SetCameraUpVector( const glm::vec3& CameraUpVector )
{
	CameraSetup.CameraUpVector = CameraUpVector;

	Update();
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
