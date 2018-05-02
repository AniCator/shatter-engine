// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

CCamera::CCamera()
{

}

CCamera::~CCamera()
{

}

void CCamera::Update()
{
	ProjectionMatrix = glm::perspective( glm::radians( CameraSetup.FieldOfView ), CameraSetup.AspectRatio, CameraSetup.NearPlaneDistance, CameraSetup.FarPlaneDistance );

	glm::vec3 CameraPosition = CameraSetup.CameraPosition;
	CameraPosition.x = -CameraPosition.x;

	ViewMatrix = glm::lookAt(
		CameraPosition,
		CameraPosition + CameraSetup.CameraDirection,
		CameraSetup.CameraUpVector
	);
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

void CCamera::SetCameraPosition( glm::vec3& CameraPosition )
{
	CameraSetup.CameraPosition = CameraPosition;

	Update();
}

void CCamera::SetCameraDirection( glm::vec3& CameraDirection )
{
	CameraSetup.CameraDirection = CameraDirection;

	Update();
}

void CCamera::SetCameraUpVector( glm::vec3& CameraUpVector )
{
	CameraSetup.CameraUpVector = CameraUpVector;

	Update();
}

glm::mat4& CCamera::GetProjectionMatrix()
{
	return ProjectionMatrix;
}

glm::mat4& CCamera::GetViewMatrix()
{
	return ViewMatrix;
}

FCameraSetup& CCamera::GetCameraSetup()
{
	return CameraSetup;
}
