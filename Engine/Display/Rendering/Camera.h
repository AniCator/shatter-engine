// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glm/glm.hpp>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/Math/Plane.h>

class CData;

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

struct Frustum
{
	Frustum() = default;
	Frustum( FCameraSetup& Setup );

	bool Contains( const Vector3D& Point ) const;
	bool Contains( const Vector3D& Point, const float Radius = 0.0f ) const;
	bool Contains( const BoundingSphere& Sphere ) const;
	bool Contains( const BoundingBox& Box ) const;

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
	void SetOffset( const int Index );
	int GetOffset() const;
	glm::mat4 GetOffsetMatrix() const;
	Vector2D GetOffsetVector() const;

	const Vector3D& GetCameraPosition() const;

	const glm::mat4& GetProjectionMatrix() const;
	const glm::mat4& GetViewMatrix() const;
	const glm::mat4& GetViewProjectionInverse() const;

	FCameraSetup& GetCameraSetup();
	FCameraSetup GetCameraSetup() const;
	Frustum GetFrustum() const;

	friend CData& operator<<( CData& Data, const CCamera& Camera );
	friend CData& operator>>( CData& Data, CCamera& Camera );

	static CCamera Lerp( const CCamera& A, const CCamera& B, const float& Alpha );
	static CCamera BezierBlend( const CCamera& A, const float& TangentA, const CCamera& B, const float& TangentB, const float& Factor );
	static CCamera HandheldSimulation( const CCamera& Camera, const float& Factor, const double& Time );

	Vector3D CameraOrientation;

	// Disables frustum updates.
	bool Freeze = false;
private:
	FCameraSetup CameraSetup;

	int OffsetIndex = -1;
	glm::mat4 ProjectionMatrix;
	glm::mat4 ViewMatrix;

	glm::mat4 ProjectionViewInverseMatrix;

	glm::quat CameraQuaternion;

	Frustum Frustum;
};
