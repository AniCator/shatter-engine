// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math/Vector.h>
#include <Engine/Utility/Math/Matrix.h>

#include <ThirdParty/glm/glm.hpp>

struct FTransform
{
public:
	FTransform();
	FTransform( const Vector3D& Position, const Vector3D& Orientation, const Vector3D& Size );

	void SetPosition( const Vector3D& Position );
	void SetOrientation( const Vector3D& Orientation );
	void SetSize( const Vector3D& Size );

	void SetTransform( const Vector3D& Position, const Vector3D& Orientation, const Vector3D& Size );
	void SetTransform( const Vector3D& Position, const Vector3D& Orientation );

	Matrix4D& GetRotationMatrix();
	Matrix4D& GetTransformationMatrix();
	Matrix4D& GetTransformationMatrixInverse();

	const Vector3D& GetPosition() const;
	const Vector3D& GetOrientation() const;
	const Vector3D& GetSize() const;

	Vector3D Position( const Vector3D& Position );
	Vector3D Rotate( const Vector3D& Position );
	glm::vec3 RotateEuler( const glm::vec3& EulerRadians );
	Vector3D Scale( const Vector3D& Position );

	glm::vec3 Transform( const glm::vec3& Position );
	Vector3D Transform( const Vector3D& Position );

	FTransform Transform( const FTransform& B );

	inline FTransform operator*( const FTransform& B )
	{
		return Transform( B );
	};

	inline FTransform operator*=( const FTransform& B )
	{
		*this = Transform( B );
		return *this;
	};

private:
	void Dirty();
	void Update();

	Matrix4D TranslationMatrix;
	Matrix4D RotationMatrix;
	Matrix4D TransformationMatrix;
	Matrix4D TransformationMatrixInverse;
	Matrix4D ScaleMatrix;

	Vector3D StoredPosition;
	Vector3D StoredOrientation;
	Vector3D StoredSize;

	bool ShouldUpdate;
};
