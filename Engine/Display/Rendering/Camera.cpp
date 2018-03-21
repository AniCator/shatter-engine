// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Camera.h"

CCamera::CCamera()
{

}

CCamera::~CCamera()
{

}

void CCamera::SetFieldOfView( float FieldOfView )
{
	CameraSetup.FieldOfView = FieldOfView;
}

void CCamera::SetAspectRatio( float AspectRatio )
{
	CameraSetup.AspectRatio = AspectRatio;
}

void CCamera::SetNearPlaneDistance( float NearPlaneDistance )
{
	CameraSetup.NearPlaneDistance = NearPlaneDistance;
}

void CCamera::SetFarPlaneDistance( float FarPlaneDistance )
{
	CameraSetup.FarPlaneDistance = FarPlaneDistance;
}

void CCamera::SetCameraPosition( glm::vec3 CameraPosition )
{
	CameraSetup.CameraPosition = CameraPosition;
}

void CCamera::SetCameraDirection( glm::vec3 CameraDirection )
{
	CameraSetup.CameraDirection = CameraDirection;
}

void CCamera::SetCameraUpVector( glm::vec3 CameraUpVector )
{
	CameraSetup.CameraUpVector = CameraUpVector;
}

FCameraSetup& CCamera::GetCameraSetup()
{
	return CameraSetup;
}
