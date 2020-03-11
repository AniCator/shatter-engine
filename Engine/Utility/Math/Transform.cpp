// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Transform.h"

#include <Engine/Utility/Math.h>
#include <Engine/Utility/Math/Vector.h>
#include <Engine/Utility/Math/Matrix.h>

#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/matrix_transform.hpp>
#include <ThirdParty/glm/gtx/quaternion.hpp>

const static Matrix4D IdentityMatrix = Matrix4D();

FTransform::FTransform()
{
	TransformationMatrix = IdentityMatrix;
	RotationMatrix = IdentityMatrix;
	ScaleMatrix = IdentityMatrix;
	TransformationMatrixInverse = IdentityMatrix;
	StoredPosition = Vector3D( 0.0f, 0.0f, 0.0f );
	StoredOrientation = Vector3D( 0.0f, 0.0f, 0.0f );
	StoredSize = Vector3D( 1.0f, 1.0f, 1.0f );

	Dirty();
}

FTransform::FTransform( const Vector3D& Position, const Vector3D& Orientation, const Vector3D& Size )
{
	SetTransform( Position, Orientation, Size );
}

void FTransform::SetPosition( const Vector3D& Position )
{
	this->StoredPosition = Position;
	Dirty();
}

void FTransform::SetOrientation( const Vector3D& Orientation )
{
	this->StoredOrientation = Orientation;
	Dirty();
}

void FTransform::SetSize( const Vector3D& Size )
{
	this->StoredSize = Size;
	Dirty();
}

void FTransform::SetTransform( const Vector3D& Position, const Vector3D& Orientation, const Vector3D& Size )
{
	this->StoredPosition = Position;
	this->StoredOrientation = Orientation;
	this->StoredSize = Size;
	Dirty();
}

void FTransform::SetTransform( const Vector3D& Position, const Vector3D& Orientation )
{
	this->StoredPosition = Position;
	this->StoredOrientation = Orientation;
	Dirty();
}

Matrix4D& FTransform::GetRotationMatrix()
{
	Update();
	return RotationMatrix;
}

Matrix4D& FTransform::GetTransformationMatrix()
{
	Update();
	return TransformationMatrix;
}

Matrix4D& FTransform::GetTransformationMatrixInverse()
{
	Update();
	return TransformationMatrixInverse;
}

const Vector3D& FTransform::GetPosition() const
{
	return StoredPosition;
}

const Vector3D& FTransform::GetOrientation() const
{
	return StoredOrientation;
}

const Vector3D& FTransform::GetSize() const
{
	return StoredSize;
}

Vector3D FTransform::Position( const Vector3D& Position )
{
	Update();
	return TransformationMatrix.Transform( Position );
}

Vector3D FTransform::Rotate( const Vector3D& Position )
{
	Update();
	return RotationMatrix.Transform( Position );
}

glm::vec3 FTransform::RotateEuler( const glm::vec3& EulerRadians )
{
	Update();
	return glm::eulerAngles( glm::quat( Math::ToGLM( RotationMatrix ) ) * glm::quat( EulerRadians ) );
}

Vector3D FTransform::Scale( const Vector3D& Position )
{
	Update();
	return ScaleMatrix.Transform( Position );
}

glm::vec3 FTransform::Transform( const glm::vec3& Position )
{
	Update();
	return Math::ToGLM( TransformationMatrix ) * glm::vec4( Position, 1.0f );
}

Vector3D FTransform::Transform( const Vector3D& Position )
{
	Update();
	Vector4D Vector = TransformationMatrix.Transform( Vector4D( Position, 1.0f ) );
	return Vector3D( Vector.X, Vector.Y, Vector.Z );
}

FTransform FTransform::Transform( const FTransform& B )
{
	Update();

	auto NewPosition = Position( B.GetPosition() );
	Vector3D Position3D = { NewPosition[0], NewPosition[1], NewPosition[2] };

	auto OrientationRadians = glm::radians( Math::ToGLM( B.GetOrientation() ) );
	auto NewOrientation = RotateEuler( OrientationRadians );
	NewOrientation = glm::degrees( NewOrientation );
	Vector3D Orientation3D = { NewOrientation[0], NewOrientation[1], NewOrientation[2] };

	auto NewSize = Scale( B.GetSize() );
	Vector3D Size3D = { NewSize[0], NewSize[1], NewSize[2] };

	return FTransform( Position3D, Orientation3D, Size3D );
}

void FTransform::Dirty()
{
	ShouldUpdate = true;
}

void FTransform::Update()
{
	if( ShouldUpdate )
	{
		static const glm::vec3 AxisX = glm::vec3( 1.0f, 0.0f, 0.0f );
		static const glm::vec3 AxisY = glm::vec3( 0.0f, 1.0f, 0.0f );
		static const glm::vec3 AxisZ = glm::vec3( 0.0f, 0.0f, 1.0f );

		ScaleMatrix = IdentityMatrix;
		ScaleMatrix.Scale( { StoredSize[0], StoredSize[1], StoredSize[2] } );

		Vector3D Radians = StoredOrientation;
		Radians.X = glm::radians( Radians.X );
		Radians.Y = glm::radians( Radians.Y );
		Radians.Z = glm::radians( Radians.Z );

		glm::quat Pitch = glm::angleAxis( Radians.X, AxisX );
		glm::quat Yaw = glm::angleAxis( Radians.Y, AxisY );
		glm::quat Roll = glm::angleAxis( Radians.Z, AxisZ );

		glm::quat Quaternion = Yaw * Pitch * Roll;
		RotationMatrix = Math::FromGLM( glm::toMat4( Quaternion ) );

		TranslationMatrix = IdentityMatrix;
		TranslationMatrix.Translate( { StoredPosition[0], StoredPosition[1], StoredPosition[2] } );
		// TranslationMatrix = Math::ToGLM( TranslateTest );
		// TranslationMatrix = glm::translate( IdentityMatrix, { StoredPosition[0], StoredPosition[1], StoredPosition[2] } );

		TransformationMatrix = TranslationMatrix * RotationMatrix * ScaleMatrix;
		TransformationMatrixInverse = Math::FromGLM( glm::inverse( Math::ToGLM( TransformationMatrix ) ) );
	}
}
