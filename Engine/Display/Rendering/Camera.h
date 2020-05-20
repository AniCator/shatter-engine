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

struct Plane
{
	Plane()
	{
		Point = Vector3D::Zero;
		Normal = Vector3D( 0.0f, 0.0f, 1.0f );
	};

	Plane(const Vector3D& Point, const Vector3D& Normal)
	{
		this->Point = Point;
		this->Normal = Normal;
	};

	Plane( const Vector3D& A, const Vector3D& B, const Vector3D& C )
	{
		Point = B - A;

		const Vector3D Edge = C - A;
		Normal = Point.Cross( Edge ).Normalized();
	};

	Vector3D Point;
	Vector3D Normal;
};

struct Frustum
{
	Frustum() = default;
	Frustum( FCameraSetup& Setup );

	bool Contains( const Vector3D& Point ) const;

	Vector3D TopRightNear;
	Vector3D BottomLeftNear;
	Vector3D TopLeftNear;
	Vector3D BottomRightNear;

	Vector3D TopLeftFar;
	Vector3D BottomLeftFar;
	Vector3D TopRightFar;
	Vector3D BottomRightFar;

	enum
	{
		Top,
		Bottom,
		Left,
		Right,
		Near,
		Far,
		Maximum
	};

	Plane Plane[Maximum];
};

class CCamera
{
public:
	CCamera();
	~CCamera();

	void Update();
	void DrawFrustum() const;

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
	Frustum GetFrustum() const;

	Vector3D CameraOrientation;
private:
	FCameraSetup CameraSetup;

	glm::mat4 ProjectionMatrix;
	glm::mat4 ViewMatrix;

	glm::mat4 ProjectionViewInverseMatrix;

	glm::quat CameraQuaternion;

	Frustum Frustum;
};
