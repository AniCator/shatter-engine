// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glm/glm.hpp>
#include <Engine/Utility/Math.h>

struct FCameraSetup
{
	FCameraSetup()
	{
		Orthographic = false;
		FieldOfView = 60.0f;
		OrthographicScale = 10.0f;
		AspectRatio = 1280.0f / 720.0f;
		NearPlaneDistance = 1.0f;
		FarPlaneDistance = 100000.0f;
		CameraPosition = { 0.0f, 0.0f, 600.0f };
		//CameraPosition = glm::vec3( 4.0f, 3.0f, 3.0f );
		CameraDirection = { 0.0f, 0.0f, -1.0f };
		CameraRightVector = { 1.0f, 0.0f, 0.0f };
		CameraUpVector = { 0.0f, 0.0f, 1.0f };
	};

	bool Orthographic;
	float FieldOfView;
	float OrthographicScale;

	float AspectRatio;
	float NearPlaneDistance;
	float FarPlaneDistance;
	Vector3D CameraPosition;
	Vector3D CameraDirection;
	Vector3D CameraRightVector;
	Vector3D CameraUpVector;

	static FCameraSetup Mix( const FCameraSetup& A, const FCameraSetup& B, const float& Alpha );
};

class CCamera
{
public:
	CCamera();
	~CCamera();

	void Update();
	void DrawFrustum();

	void SetFieldOfView( const float& FieldOfView );
	void SetOrthographicScale( const float& OrthographicScale );
	void SetAspectRatio( const float& AspectRatio);
	void SetNearPlaneDistance( const float& NearPlaneDistance );
	void SetFarPlaneDistance( const float& FarPlaneDistance );
	void SetCameraPosition( const Vector3D& CameraPosition );
	void SetCameraDirection( const Vector3D& CameraDirection );
	void SetCameraOrientation( const Vector3D& CameraOrientation );
	void SetCameraUpVector( const Vector3D& CameraUpVector );

	const Vector3D& GetCameraPosition() const;

	const glm::mat4& GetProjectionMatrix() const;
	const glm::mat4& GetViewMatrix() const;
	const glm::mat4& GetViewProjectionInverse() const;

	FCameraSetup& GetCameraSetup();

	Vector3D CameraOrientation;
private:
	FCameraSetup CameraSetup;
	FFrustum Frustum;

	glm::mat4 ProjectionMatrix;
	glm::mat4 ViewMatrix;

	glm::mat4 ProjectionViewInverseMatrix;

	glm::quat CameraQuaternion;
};
