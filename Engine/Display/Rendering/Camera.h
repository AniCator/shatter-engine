// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glm/glm.hpp>
#include <Engine/Utility/Math.h>

static const glm::vec3 WorldRight = glm::vec3( 1.0f, 0.0f, 0.0f );
static const glm::vec3 WorldForward = glm::vec3( 0.0f, 1.0f, 0.0f );
static const glm::vec3 WorldUp = glm::vec3( 0.0f, 0.0f, 1.0f );

struct FCameraSetup
{
	FCameraSetup()
	{
		FieldOfView = 60.0f;
		AspectRatio = 1280.0f / 720.0f;
		NearPlaneDistance = 1.0f;
		FarPlaneDistance = 100000.0f;
		CameraPosition = glm::vec3( 0.0f, 0.0f, 600.0f );
		//CameraPosition = glm::vec3( 4.0f, 3.0f, 3.0f );
		CameraDirection = glm::vec3( 0.0f, 0.0f, -1.0f );
		CameraRightVector = glm::vec3( 1.0f, 0.0f, 0.0f );
		CameraUpVector = glm::vec3( 0.0f, 0.0f, 1.0f );
	};

	float FieldOfView;
	float AspectRatio;
	float NearPlaneDistance;
	float FarPlaneDistance;
	glm::vec3 CameraPosition;
	glm::vec3 CameraDirection;
	glm::vec3 CameraRightVector;
	glm::vec3 CameraUpVector;
};

class CCamera
{
public:
	CCamera();
	~CCamera();

	void Update();

	void SetFieldOfView( float& FieldOfView );
	void SetAspectRatio( float& AspectRatio);
	void SetNearPlaneDistance( float& NearPlaneDistance );
	void SetFarPlaneDistance( float& FarPlaneDistance );
	void SetCameraPosition( const glm::vec3& CameraPosition );
	void SetCameraDirection( const glm::vec3& CameraDirection );
	void SetCameraOrientation( const glm::vec3& CameraOrientation );
	void SetCameraUpVector( const glm::vec3& CameraUpVector );

	const glm::mat4& GetProjectionMatrix() const;
	const glm::mat4& GetViewMatrix() const;
	const glm::mat4& GetViewProjectionInverse() const;

	FCameraSetup& GetCameraSetup();
private:
	FCameraSetup CameraSetup;
	FFrustum Frustum;

	glm::mat4 ProjectionMatrix;
	glm::mat4 ViewMatrix;

	glm::mat4 ProjectionViewInverseMatrix;

	glm::vec3 CameraOrientation;
	glm::quat CameraQuaternion;
};
