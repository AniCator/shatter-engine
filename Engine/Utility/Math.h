// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math/Vector.h>

#include <math.h>

#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/matrix_transform.hpp>
#include <ThirdParty/glm/gtx/quaternion.hpp>

#define Vector3DToInitializerList(Vector) { Vector[0],Vector[1],Vector[2] }
#define Vector3DToGLM(Vector) glm::vec3( Vector[0],Vector[1],Vector[2] )

#define Vector2DToInitializerList(Vector) { Vector[0],Vector[1] }
#define Vector2DToGLM(Vector) glm::vec2( Vector[0],Vector[1] )

const static glm::mat4 IdentityMatrix = glm::mat4( 1.0f );
static const Vector3D WorldRight = { 1.0f, 0.0f, 0.0f };
static const Vector3D WorldForward = { 0.0f, 1.0f, 0.0f };
static const Vector3D WorldUp = { 0.0f, 0.0f, 1.0f };

struct FFrustumPlane
{

};

struct FFrustum
{
	FFrustumPlane Planes[6];
};

struct FBounds
{
	Vector3D Minimum;
	Vector3D Maximum;
};

struct FTransform
{
public:
	FTransform()
	{
		TransformationMatrix = IdentityMatrix;
		RotationMatrix = IdentityMatrix;
		ScaleMatrix = IdentityMatrix;
		StoredPosition = Vector3D( 0.0f, 0.0f, 0.0f );
		StoredOrientation = Vector3D( 0.0f, 0.0f, 0.0f );
		StoredSize = Vector3D( 1.0f, 1.0f, 1.0f );
	}

	FTransform( const Vector3D& Position, const Vector3D& Orientation, const Vector3D& Size )
	{
		SetTransform( Position, Orientation, Size );
	}

	void SetPosition( const Vector3D& Position )
	{
		this->StoredPosition = Position;
		Update();
	}

	void SetOrientation( const Vector3D& Orientation )
	{
		this->StoredOrientation = Orientation;
		Update();
	}

	void SetSize( const Vector3D& Size )
	{
		this->StoredSize = Size;
		Update();
	}

	void SetTransform( const Vector3D& Position, const Vector3D& Orientation, const Vector3D& Size )
	{
		this->StoredPosition = Position;
		this->StoredOrientation = Orientation;
		this->StoredSize = Size;
		Update();
	}

	void SetTransform( const Vector3D& Position, const Vector3D& Orientation )
	{
		this->StoredPosition = Position;
		this->StoredOrientation = Orientation;
		Update();
	}

	glm::mat4& GetRotationMatrix()
	{
		return RotationMatrix;
	}

	glm::mat4& GetTransformationMatrix()
	{
		return TransformationMatrix;
	}

	const Vector3D& GetPosition() const
	{
		return StoredPosition;
	}

	const Vector3D& GetOrientation() const
	{
		return StoredOrientation;
	}

	const Vector3D& GetSize() const
	{
		return StoredSize;
	}

	glm::vec3 Position( const glm::vec3& Position ) const
	{
		return TransformationMatrix * glm::vec4( Position, 1.0f );
	}

	glm::vec3 Rotate( const glm::vec3& Position ) const
	{
		return RotationMatrix * glm::vec4( Position, 1.0f );
	}

	glm::vec3 RotateEuler( const glm::vec3& EulerRadians ) const
	{
		return glm::eulerAngles( glm::quat( RotationMatrix ) * glm::quat( EulerRadians ) );
	}

	glm::vec3 Scale( const glm::vec3& Position ) const
	{
		return ScaleMatrix * glm::vec4( Position, 1.0f );
	}

	glm::vec3 Transform( const glm::vec3& Position ) const
	{
		return TransformationMatrix * RotationMatrix * ScaleMatrix * glm::vec4( Position, 1.0f );
	}

	FTransform Transform( const FTransform& B ) const
	{
		auto NewPosition = Position( Vector3DToGLM( B.GetPosition() ) );
		Vector3D Position3D = { NewPosition[0], NewPosition[1], NewPosition[2] };

		auto OrientationRadians = glm::radians( Vector3DToGLM( B.GetOrientation() ) );
		auto NewOrientation = RotateEuler( OrientationRadians );
		NewOrientation = glm::degrees( NewOrientation );
		Vector3D Orientation3D = { NewOrientation[0], NewOrientation[1], NewOrientation[2] };

		auto NewSize = Scale( Vector3DToGLM( B.GetSize() ) );
		Vector3D Size3D = { NewSize[0], NewSize[1], NewSize[2] };

		return FTransform( Position3D, Orientation3D, Size3D );
	}

private:
	void Update()
	{
		static const Vector3D AxisX = Vector3D( 1.0f, 0.0f, 0.0f );
		static const Vector3D AxisY = Vector3D( 0.0f, 1.0f, 0.0f );
		static const Vector3D AxisZ = Vector3D( 0.0f, 0.0f, 1.0f );

		ScaleMatrix = glm::scale( IdentityMatrix, { StoredSize[0], StoredSize[1], StoredSize[2] } );
		
		glm::quat Quaternion = glm::quat( glm::radians( Vector3DToGLM( StoredOrientation ) ) );
		RotationMatrix = glm::toMat4( Quaternion );

		TransformationMatrix = glm::translate( IdentityMatrix, { StoredPosition[0], StoredPosition[1], StoredPosition[2] } );

		TransformationMatrix = TransformationMatrix * RotationMatrix * ScaleMatrix;
	}

	glm::mat4 RotationMatrix;
	glm::mat4 TransformationMatrix;
	glm::mat4 ScaleMatrix;

	Vector3D StoredPosition;
	Vector3D StoredOrientation;
	Vector3D StoredSize;
};

namespace Math
{
	static const float Pi = glm::pi<float>();
	static const float Pi2 = Pi * 2.0f;

	inline glm::vec3 EulerToDirection( const glm::vec3& Euler )
	{
		glm::vec3 Direction;
		Direction[1] = cos( glm::radians( Euler[0] ) ) * cos( glm::radians( Euler[1] ) );
		Direction[2] = sin( glm::radians( Euler[0] ) );
		Direction[0] = cos( glm::radians( Euler[0] ) ) * sin( glm::radians( Euler[1] ) );
		Direction = glm::normalize( Direction );

		return Direction;
	}

	inline float LengthSquared( const glm::vec2& Vector )
	{
		return Vector[0] * Vector[0] + Vector[1] * Vector[1];
	}

	inline float Length( const glm::vec2& Vector )
	{
		return sqrtf( LengthSquared( Vector ) );
	}

	inline float Normalize( glm::vec2& Vector )
	{
		const float LengthBiased = 1.f / ( Length( Vector ) + FLT_EPSILON );

		Vector[0] *= LengthBiased;
		Vector[1] *= LengthBiased;

		return LengthBiased;
	}

	inline float Normalize( glm::vec2& Vector, float Length )
	{
		const float LengthBiased = 1.f / ( Length + FLT_EPSILON );

		Vector[0] *= LengthBiased;
		Vector[1] *= LengthBiased;

		return LengthBiased;
	}

	inline float LengthSquared( const glm::vec3& Vector )
	{
		return Vector[0] * Vector[0] + Vector[1] * Vector[1] + Vector[2] * Vector[2];
	}

	inline float Length( const glm::vec3& Vector )
	{
		return sqrtf( LengthSquared( Vector ) );
	}

	inline float Normalize( glm::vec3& Vector )
	{
		const float LengthBiased = 1.f / ( Length( Vector ) + FLT_EPSILON );

		Vector[0] *= LengthBiased;
		Vector[1] *= LengthBiased;
		Vector[2] *= LengthBiased;

		return LengthBiased;
	}

	inline float Normalize( glm::vec3& Vector, float Length )
	{
		const float LengthBiased = 1.f / ( Length + FLT_EPSILON );

		Vector[0] *= LengthBiased;
		Vector[1] *= LengthBiased;
		Vector[2] *= LengthBiased;

		return LengthBiased;
	}

	inline float Lerp( float A, float B, float Alpha )
	{
		return A + Alpha * ( B - A );
	}

	inline bool PointInBoundingBox( const glm::vec3& Vector, const glm::vec3& Minimum, const glm::vec3& Maximum )
	{
		if( Vector[0] > Minimum[0] && Vector[1] > Minimum[1] && Vector[2] > Minimum[2] &&
			Vector[0] < Maximum[0] && Vector[1] < Maximum[1] && Vector[2] < Maximum[2] )
		{
			return true;
		}

		return false;
	}

	inline bool BoundingBoxIntersection( const glm::vec3& MinimumA, const glm::vec3& MaximumA, const glm::vec3& MinimumB, const glm::vec3& MaximumB )
	{
		if( ( MinimumA[0] < MaximumB[0] && MaximumA[0] > MinimumB[0] ) &&
			( MinimumA[1] < MaximumB[1] && MaximumA[1] > MinimumB[1] ) &&
			( MinimumA[2] < MaximumB[2] && MaximumA[2] > MinimumB[2] ) )
		{
			return true;
		}

		return false;
	}

	inline void Seed( uint32_t Value )
	{
		std::srand( Value );
	}

	inline double Random()
	{
		return static_cast<double>( std::rand() ) / RAND_MAX;
	}
}
